#include "Navigation/NavServer.hpp"
#include "Navigation/NavMesh.hpp"
#include "Utils.hpp"

#include <algorithm>
#include <cassert>
#include <cfloat>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <random>
#include <thread>

#include <recastnavigation/DetourNavMesh.h>
#include <recastnavigation/DetourNavMeshQuery.h>
#include <recastnavigation/DetourPathCorridor.h>

namespace VulpineNav::NavServer
{
AgentProfile const AgentProfile::Humanoid = {
    .radius = 0.25f,
    .height = 2.0f,
    .maxClimb = kHumanoidStepHeight,
    .maxSlopeDeg = 47.0f
};

namespace
{
constexpr i64 kFramesPerPriority = 60;

constexpr int kWorkerQueryNodes = 8192;

constexpr int kMaxPathPolys = 512;
constexpr int kMaxStraightPoints = 256;

// when failing to build a path, will retry every `kRetryDelayFrames` frames, a maximum of `kMaxStartAttempts` times. 
// if it still fails after that, the request returns NoPath
// maybe consider changing this to time elapsed instead of frame count, could become unreliable at framerates different to 60
constexpr u64 kRetryDelayFrames = 10;
constexpr u32 kMaxStartAttempts = 30; // about 5 seconds at 60fps

constexpr u32 kMaxInFlight = 16;

constexpr u32 kDeadGeneration = 0xffffffff;

struct LinkReservation
{
    u32 id;
    int slot;
    u64 poly;
    float distance;
};
using Reservations = std::vector<LinkReservation>;

struct QueryRequest
{
    i64 key;
    u32 index;
    u32 generation;
    int meshSlot;
    vec3 start;
    vec3 goal;
    vec3 extents; // half extents for the goal radius
    u32 attempts;
    PathFilter filter;
    std::shared_ptr<const Reservations> reservations; // snapshot when requested, read on the query thread

    bool operator>(const QueryRequest& o) const { return key > o.key; }
};

struct QueryResult
{
    QueryRequest req;
    PathStatus status = PathStatus::NoPath;
    PartialReason partialReason = PartialReason::None;
    u64 partialWatch = 0;
    bool startMissing = false;
    std::vector<vec3> points;
    std::vector<dtPolyRef> polys;
    vec3 pathStart = vec3(0);
    vec3 pathEnd = vec3(0);
};

struct PathSlot
{
    u32 generation = 0;
    bool inUse = false;
    PathStatus status = PathStatus::Invalid;
    PartialReason partialReason = PartialReason::None;
    u64 partialWatch = 0;
    std::vector<vec3> points;
    std::vector<dtPolyRef> polys;
    vec3 pathStart = vec3(0);
    vec3 pathEnd = vec3(0);
};

struct DelayedRetry
{
    u64 frame;
    QueryRequest req;
};

struct DebugPath
{
    u64 frame = 0;
    PathStatus status = PathStatus::Invalid;
    vec3 start = vec3(0);
    vec3 goal = vec3(0);
    std::vector<vec3> points;
};


struct CachedProfile
{
    AgentProfile profile;
    int slot;
};

// == main thread state ==
u64 frame = 0;
u32 inFlight = 0; // slots in the Pending state
std::vector<PathSlot> slots;
std::vector<u32> freeSlots;
std::vector<QueryRequest> newRequests;
std::vector<QueryResult> finishedScratch;
std::vector<DelayedRetry> retries;
std::vector<CachedProfile> profileCache;

struct LinkFailures
{
    int slot;
    u64 poly;
    int count;
};
std::vector<LinkFailures> linkFailures;

Reservations reservations;
std::shared_ptr<const Reservations> reservationSnapshot;
bool reservationsDirty = false;
u32 nextReservationId = 1;

constexpr size_t kDebugMaxRecent = 128;
constexpr u64 kDebugLifetimeFrames = 120;
constexpr float kDebugLift = 0.1f;
constexpr float kDebugTickHeight = 0.3f;

std::vector<DebugPath> debugRecent;
size_t debugNext = 0;
u64 debugLastFrame = 0;

bool debugRecording()
{
    return debugLastFrame != 0 && frame <= debugLastFrame + 1;
}

void debugRecord(const QueryRequest& req, PathStatus status, const std::vector<vec3>& points)
{
    if (!debugRecording())
        return;

    if (debugRecent.size() < kDebugMaxRecent)
        debugRecent.emplace_back();

    DebugPath& p = debugRecent[debugNext];
    debugNext = (debugNext + 1) % kDebugMaxRecent;

    p.frame = frame;
    p.status = status;
    p.start = req.start;
    p.goal = req.goal;
    p.points = points;
}

void appendPath(std::vector<DebugSegment>& out, const std::vector<vec3>& points)
{
    const vec3 lift(0.0f, kDebugLift, 0.0f);
    const vec3 tick(0.0f, kDebugTickHeight, 0.0f);

    for (size_t i = 0; i < points.size(); i++)
    {
        const vec3 p = points[i] + lift;
        out.push_back({p, p + tick});
        if (i + 1 < points.size())
            out.push_back({p, points[i + 1] + lift});
    }
}

// == shared with the query thread (guarded by `queueMutex`) ==
std::jthread worker;
std::mutex queueMutex;
std::condition_variable_any cv;
std::priority_queue<QueryRequest, std::vector<QueryRequest>, std::greater<QueryRequest>> requestQueue;
std::vector<QueryResult> finished;
std::vector<u32> liveGeneration;

// == helpers ==

// vec3 to float[3] (or float*) & vice versa
const float* ptr(const vec3& v) { return &v.x; }
float* ptr(vec3& v) { return &v.x; }


MeshTier::AgentShape toShape(const AgentProfile& p)
{
    return {p.radius, p.height, p.maxClimb, p.maxSlopeDeg};
}

bool sameProfile(const AgentProfile& a, const AgentProfile& b)
{
    return a.radius == b.radius && a.height == b.height
        && a.maxClimb == b.maxClimb && a.maxSlopeDeg == b.maxSlopeDeg;
}

vec3 agentExtents(const AgentProfile& p)
{
    const float xz = max(1.0f, p.radius * 2.0f);
    return vec3(xz, p.height, xz);
}

int slotFor(const AgentProfile& profile, bool create)
{
    for (const CachedProfile& c : profileCache)
        if (sameProfile(c.profile, profile))
            return c.slot;

    const int slot = create
        ? NavMesh::ResolveAgent(toShape(profile))
        : NavMesh::FindAgentSlot(toShape(profile));

    if (slot >= 0)
        profileCache.push_back({profile, slot});
    return slot;
}

PathSlot* resolve(PathHandle h)
{
    if (h.index >= slots.size())
        return nullptr;
    PathSlot& s = slots[h.index];
    return (s.inUse && s.generation == h.generation) ? &s : nullptr;
}

bool isLive(u32 index, u32 generation)
{
    return index < slots.size() && slots[index].inUse && slots[index].generation == generation;
}

// static [0-1) random generator
float frand()
{
    static std::minstd_rand rng(0x5EED);
    return std::uniform_real_distribution<float>(0.0f, 1.0f)(rng);
}

class NavFilter : public dtQueryFilter
{
public:
    explicit NavFilter(const PathFilter& filter = {}, const Reservations* reservations = nullptr, int slot = -1)
        : m_excluded(filter.excludedLinks), m_reservations(reservations), m_slot(slot),
          m_self(filter.reservation), m_selfLink(filter.reservedLink), m_selfDistance(filter.reservedDistance)
    {
        setIncludeFlags(filter.includeFlags);

        m_linkPenalty[kAreaClimbLow] = 3.0f;
        m_linkPenalty[kAreaClimbHigh] = 8.0f;
        m_linkPenalty[kAreaDrop] = 2.0f;
        m_linkPenalty[kAreaSlide] = 4.0f;
    }

    float getCost(const float* pa, const float* pb,
                  const dtPolyRef prevRef, const dtMeshTile* prevTile, const dtPoly* prevPoly,
                  const dtPolyRef curRef, const dtMeshTile* curTile, const dtPoly* curPoly,
                  const dtPolyRef nextRef, const dtMeshTile* nextTile, const dtPoly* nextPoly) const override
    {
        float cost = dtQueryFilter::getCost(pa, pb, prevRef, prevTile, prevPoly, curRef, curTile, curPoly,
                                            nextRef, nextTile, nextPoly);
        if (curPoly->getType() == DT_POLYTYPE_OFFMESH_CONNECTION)
            cost += m_linkPenalty[curPoly->getArea()] + kBusyLinkCost * (float)othersOn(curRef);
        return cost;
    }

    int othersOn(dtPolyRef link) const
    {
        if (!m_reservations)
            return 0;
        int n = 0;
        for (const LinkReservation& r : *m_reservations)
            if (r.slot == m_slot && r.poly == link && r.id != m_self && (link != m_selfLink || r.distance < m_selfDistance))
                n++;
        return n;
    }

    bool passFilter(const dtPolyRef ref, const dtMeshTile* tile, const dtPoly* poly) const override
    {
        if (!dtQueryFilter::passFilter(ref, tile, poly))
            return false;
        for (uint64_t excluded : m_excluded)
            if (excluded && excluded == ref)
                return false;
        return true;
    }

private:
    float m_linkPenalty[DT_MAX_AREAS] = {};
    std::array<uint64_t, 4> m_excluded{};
    const Reservations* m_reservations;
    int m_slot;
    u32 m_self;
    u64 m_selfLink;
    float m_selfDistance;
};

bool snapToMesh(const dtNavMeshQuery* q, const dtQueryFilter& filter, const vec3& pos, const vec3& ext,
                dtPolyRef& outRef, vec3& outPt)
{
    constexpr int kMaxCandidates = 32;
    constexpr float kAboveTolerance = 0.25f;
    constexpr float kAbovePenalty = 4.0f;

    dtPolyRef polys[kMaxCandidates];
    int count = 0;
    q->queryPolygons(ptr(pos), ptr(ext), &filter, polys, &count, kMaxCandidates);

    outRef = 0;
    float best = FLT_MAX;
    for (int i = 0; i < count; i++)
    {
        vec3 closest;
        bool overPoly = false;
        if (dtStatusFailed(q->closestPointOnPoly(polys[i], ptr(pos), ptr(closest), &overPoly)))
            continue;

        const float dx = closest.x - pos.x;
        const float dz = closest.z - pos.z;
        const float dy = pos.y - closest.y; // > 0 => surface is below pos
        const float vertical = dy >= -kAboveTolerance ? dy : dy * kAbovePenalty;

        const float cost = dx * dx + dz * dz + vertical * vertical;
        if (cost < best)
        {
            best = cost;
            outRef = polys[i];
            outPt = closest;
        }
    }
    return outRef != 0;
}

// == query thread ==

// the goal isn't on the mesh: use the nearest mesh around it, or failing that,
// the first mesh found walking back towards the start
bool findProxyGoal(dtNavMeshQuery* q, const dtQueryFilter& filter, const QueryRequest& req,
                   dtPolyRef& outRef, vec3& outPt)
{
    for (float radius : {2.0f, 4.0f, 8.0f})
    {
        const vec3 ext(radius, req.extents.y, radius);
        if (snapToMesh(q, filter, req.goal, ext, outRef, outPt))
            return true;
    }

    const vec3 delta = req.goal - req.start;
    const float dist = length(vec2(delta.x, delta.z));
    if (dist < 0.001f)
        return false;

    constexpr float kStep = 2.0f;
    const vec3 ext(req.extents.x, max(req.extents.y, 32.0f), req.extents.z);
    for (float d = dist - kStep; d > 0.0f; d -= kStep)
    {
        const vec3 p = req.start + delta * (d / dist);
        if (snapToMesh(q, filter, p, ext, outRef, outPt))
            return true;
    }
    return false;
}

bool tileBuilt(const dtNavMeshQuery* q, ivec2 tile)
{
    if (tile.x < 0 || tile.y < 0 || tile.x >= (int)worldTileLength || tile.y >= (int)worldTileLength)
        return false;
    return q->getAttachedNavMesh()->getTileAt(tile.x, tile.y, 0) != nullptr;
}

// a path stopping at a tile border with nothing built past it hit unloaded mesh, not a dead end
bool endsAtMissingTile(const dtNavMeshQuery* q, const vec3& pathEnd)
{
    constexpr float kBorderDistance = 1.0f;

    const ivec2 tile = worldToTileCoord(pathEnd);
    const vec2 local = vec2(pathEnd.x - kWorldOrigin.x, pathEnd.z - kWorldOrigin.z) - vec2(tile) * kTileWorldSize;

    const ivec2 sides[4] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    const float dists[4] = {local.x, kTileWorldSize - local.x, local.y, kTileWorldSize - local.y};
    for (int i = 0; i < 4; i++)
    {
        if (dists[i] < kBorderDistance && !tileBuilt(q, tile + sides[i]))
            return true;
    }
    return false;
}

QueryResult runQuery(const QueryRequest& req, dtNavMeshQuery* q)
{
    QueryResult res;
    res.req = req;

    const NavFilter filter(req.filter, req.reservations.get(), req.meshSlot);

    dtPolyRef startRef = 0;
    vec3 startPt;
    snapToMesh(q, filter, req.start, req.extents, startRef, startPt);
    if (!startRef)
    {
        res.startMissing = true;
        return res;
    }

    bool partial = false;
    PartialReason reason = PartialReason::None;

    dtPolyRef endRef = 0;
    vec3 endPt;
    snapToMesh(q, filter, req.goal, req.extents, endRef, endPt);
    if (!endRef)
    {
        if (!findProxyGoal(q, filter, req, endRef, endPt))
            return res; // no path at all
        partial = true;
        reason = tileBuilt(q, worldToTileCoord(req.goal)) ? PartialReason::GoalOffMesh : PartialReason::Unloaded;
    }

    dtPolyRef polys[kMaxPathPolys];
    int polyCount = 0;
    const dtStatus st = q->findPath(startRef, endRef, ptr(startPt), ptr(endPt), &filter,
                                    polys, &polyCount, kMaxPathPolys);
    if (dtStatusFailed(st) || polyCount == 0)
        return res;

    bool searchLimited = dtStatusDetail(st, DT_OUT_OF_NODES) || dtStatusDetail(st, DT_BUFFER_TOO_SMALL);
    if (dtStatusDetail(st, DT_PARTIAL_RESULT) || searchLimited)
        partial = true;

    // partial corridor
    vec3 pathEnd = endPt;
    if (polys[polyCount - 1] != endRef)
    {
        q->closestPointOnPoly(polys[polyCount - 1], ptr(endPt), ptr(pathEnd), nullptr);
        partial = true;
        if (reason == PartialReason::None)
            reason = endsAtMissingTile(q, pathEnd) ? PartialReason::Unloaded : PartialReason::Disconnected;
    }

    float straight[kMaxStraightPoints * 3];
    int straightCount = 0;
    q->findStraightPath(ptr(startPt), ptr(pathEnd), polys, polyCount,
                        straight, nullptr, nullptr, &straightCount, kMaxStraightPoints, 0);
    if (straightCount == 0)
        return res;
    if (straightCount == kMaxStraightPoints)
    {
        partial = true;
        searchLimited = true;
    }

    res.points.resize(straightCount);
    for (int i = 0; i < straightCount; i++)
        res.points[i] = vec3(straight[i * 3], straight[i * 3 + 1], straight[i * 3 + 2]);

    res.polys.assign(polys, polys + polyCount);
    res.pathStart = startPt;
    res.pathEnd = pathEnd;

    res.status = partial ? PathStatus::Partial : PathStatus::Complete;
    if (partial)
    {
        // the end of a truncated search says nothing about reachability, the agent should walk it and search again
        res.partialReason = searchLimited ? PartialReason::SearchLimit
                          : reason != PartialReason::None ? reason : PartialReason::Disconnected;
        // read under the query lock, so it matches the mesh this search saw
        res.partialWatch = GetWatchVersion(req.goal, pathEnd);
    }
    return res;
}

// query object for the worker, since dtNavMeshQuery is not thread safe
dtNavMeshQuery* workerQuery(std::vector<dtNavMeshQuery*>& queries, int slot)
{
    if ((size_t)slot >= queries.size())
        queries.resize(slot + 1, nullptr);

    if (!queries[slot])
    {
        dtNavMesh* mesh = NavMesh::GetNavMesh(slot);
        dtNavMeshQuery* q = mesh ? dtAllocNavMeshQuery() : nullptr;
        if (q && dtStatusFailed(q->init(mesh, kWorkerQueryNodes)))
        {
            dtFreeNavMeshQuery(q);
            q = nullptr;
        }
        queries[slot] = q;
    }
    return queries[slot];
}

// main worker thread loop
void WorkerLoop(std::stop_token stop_token)
{
    currentThreadID = THREAD_ID_NAVMESH_QUERY;

    std::vector<dtNavMeshQuery*> queries;

    while (!stop_token.stop_requested())
    {
        QueryRequest req;
        {
            std::unique_lock lock(queueMutex);
            // wakes on stop requests too, a plain wait + notify after request_stop can miss the wakeup
            if (!cv.wait(lock, stop_token, [] { return !requestQueue.empty(); }))
                break;

            req = requestQueue.top();
            requestQueue.pop();

            // released after it was queued
            if (req.index >= liveGeneration.size() || liveGeneration[req.index] != req.generation)
                continue;
        }

        QueryResult res;
        {
            NavMesh::QueryReadLock meshLock;
            dtNavMeshQuery* q = workerQuery(queries, req.meshSlot);
            if (q)
                res = runQuery(req, q);
            else
                res.req = req; // no path
        }

        std::lock_guard lock(queueMutex);
        finished.push_back(std::move(res));
    }

    for (dtNavMeshQuery* q : queries)
        dtFreeNavMeshQuery(q);
}

} // namespace

void Init()
{
    assert(!worker.joinable());

    frame = 0;
    worker = std::jthread(&WorkerLoop);
}

void Shutdown()
{
    worker.request_stop();

    NavMesh::CancelWriterWait();

    if (worker.joinable())
        worker.join();

    while (!requestQueue.empty()) requestQueue.pop();
    inFlight = 0;
    finished.clear();
    finishedScratch.clear();
    liveGeneration.clear();
    newRequests.clear();
    retries.clear();
    slots.clear();
    freeSlots.clear();
    profileCache.clear();
    linkFailures.clear();
    reservations.clear();
    reservationSnapshot.reset();
    reservationsDirty = false;
    debugRecent.clear();
    debugNext = 0;
    debugLastFrame = 0;
}

void Tick()
{
    frame++;

    // retry failed requests
    for (size_t i = 0; i < retries.size();)
    {
        if (frame < retries[i].frame)
        {
            i++;
            continue;
        }
        if (isLive(retries[i].req.index, retries[i].req.generation))
            newRequests.push_back(retries[i].req);
        retries[i] = retries.back();
        retries.pop_back();
    }

    bool pushed = false;
    {
        std::lock_guard lock(queueMutex);

        liveGeneration.resize(slots.size());
        for (size_t i = 0; i < slots.size(); i++)
            liveGeneration[i] = slots[i].inUse ? slots[i].generation : kDeadGeneration;

        for (const QueryRequest& r : newRequests)
        {
            if (!isLive(r.index, r.generation))
                continue; // released while processing
            requestQueue.push(r);
            pushed = true;
        }

        finishedScratch.swap(finished);
    }
    if (pushed)
        cv.notify_one();
    newRequests.clear();

    for (QueryResult& res : finishedScratch)
    {
        const QueryRequest& req = res.req;
        if (!isLive(req.index, req.generation))
            continue;

        PathSlot& slot = slots[req.index];

        if (res.startMissing)
        {
            if (req.attempts + 1 < kMaxStartAttempts && NavMesh::IsTileWanted(req.start))
            {
                QueryRequest again = req;
                again.attempts++;
                retries.push_back({frame + kRetryDelayFrames, again});
                continue; // try again later
            }
            if (slot.status == PathStatus::Pending)
                inFlight--;
            slot.status = PathStatus::NoPath;
            slot.points.clear();
            slot.polys.clear();
            debugRecord(req, slot.status, slot.points);
            continue;
        }

        if (slot.status == PathStatus::Pending)
            inFlight--;
        slot.status = res.status;
        slot.partialReason = res.partialReason;
        slot.partialWatch = res.partialWatch;
        slot.points = std::move(res.points);
        slot.polys = std::move(res.polys);
        slot.pathStart = res.pathStart;
        slot.pathEnd = res.pathEnd;
        debugRecord(req, slot.status, slot.points);
    }
    finishedScratch.clear();
}

void GetDebugSegments(DebugSegments& out)
{
    out.clear();
    debugLastFrame = frame == 0 ? 1 : frame;

    auto bucket = [&](PathStatus st) -> std::vector<DebugSegment>* {
        switch (st)
        {
        case PathStatus::Complete: return &out.complete;
        case PathStatus::Partial:  return &out.partial;
        case PathStatus::NoPath:   return &out.failed;
        default:                   return nullptr;
        }
    };

    for (const PathSlot& s : slots)
    {
        if (!s.inUse)
            continue;
        if (std::vector<DebugSegment>* b = bucket(s.status); b && b != &out.failed)
            appendPath(*b, s.points);
    }

    const vec3 lift(0.0f, kDebugLift, 0.0f);
    for (const DebugPath& p : debugRecent)
    {
        if (p.frame + kDebugLifetimeFrames < frame)
            continue;

        std::vector<DebugSegment>* b = bucket(p.status);
        if (!b)
            continue;

        if (p.status == PathStatus::NoPath)
            b->push_back({p.start + lift, p.goal + lift});
        else
            appendPath(*b, p.points);
    }
}

uint64_t GetNavmeshVersion()
{
    return NavMesh::GetVersion();
}

// == async requests ==

namespace
{
std::shared_ptr<const Reservations> snapshotReservations()
{
    if (reservationsDirty || !reservationSnapshot)
    {
        reservationSnapshot = std::make_shared<const Reservations>(reservations);
        reservationsDirty = false;
    }
    return reservationSnapshot;
}
} // namespace

PathHandle RequestPath(vec3 start, vec3 goal, const AgentProfile& profile, Priority priority, const PathFilter& filter)
{
    u32 index;
    if (!freeSlots.empty())
    {
        index = freeSlots.back();
        freeSlots.pop_back();
    }
    else
    {
        index = (u32)slots.size();
        slots.emplace_back();
    }

    PathSlot& slot = slots[index];
    slot.inUse = true;
    slot.points.clear();
    slot.polys.clear();
    slot.partialReason = PartialReason::None;
    slot.partialWatch = 0;

    const PathHandle handle{index, slot.generation};

    const int meshSlot = slotFor(profile, true);
    if (meshSlot < 0)
    {
        slot.status = PathStatus::NoPath;
        return handle;
    }

    slot.status = PathStatus::Pending;
    inFlight++;
    newRequests.push_back({
        (i64)frame - (i64)priority * kFramesPerPriority,
        index, slot.generation, meshSlot,
        start, goal, agentExtents(profile), 0, filter, snapshotReservations()});

    return handle;
}

PathStatus GetPath(PathHandle path, std::vector<vec3>& outPoints)
{
    const PathSlot* s = resolve(path);
    if (!s)
        return PathStatus::Invalid;

    if (s->status == PathStatus::Complete || s->status == PathStatus::Partial)
        outPoints = s->points;
    return s->status;
}

PathStatus GetStatus(PathHandle path)
{
    const PathSlot* s = resolve(path);
    return s ? s->status : PathStatus::Invalid;
}

PartialReason GetPartialReason(PathHandle path)
{
    const PathSlot* s = resolve(path);
    return s ? s->partialReason : PartialReason::None;
}

u64 GetWatchVersion(vec3 goal, vec3 pathEnd)
{
    return NavMesh::GetRegionVersion(goal) + NavMesh::GetRegionVersion(pathEnd);
}

u64 GetPartialWatch(PathHandle path)
{
    const PathSlot* s = resolve(path);
    return s ? s->partialWatch : 0;
}

bool CanRequestPath()
{
    return inFlight < kMaxInFlight;
}

void ReserveLink(const AgentProfile& profile, uint32_t& reservation, uint64_t link, float distance)
{
    auto it = std::find_if(reservations.begin(), reservations.end(),
                           [&](const LinkReservation& r) { return r.id == reservation; });

    if (!link)
    {
        if (reservation && it != reservations.end())
        {
            *it = reservations.back();
            reservations.pop_back();
            reservationsDirty = true;
        }
        return;
    }

    const int slot = slotFor(profile, false);
    if (slot < 0)
        return;
    if (!reservation)
        reservation = nextReservationId++;

    if (it == reservations.end())
        reservations.push_back({reservation, slot, link, distance});
    else
        *it = {reservation, slot, link, distance};
    reservationsDirty = true;
}

int LinkAgentsAhead(const AgentProfile& profile, uint32_t reservation, uint64_t link, float distance)
{
    const int slot = slotFor(profile, false);
    int n = 0;
    for (const LinkReservation& r : reservations)
        if (r.slot == slot && r.poly == link && r.id != reservation && r.distance < distance)
            n++;
    return n;
}

void ReportLinkResult(const AgentProfile& profile, uint64_t linkPoly, bool success)
{
    const int slot = slotFor(profile, false);
    if (slot < 0 || !linkPoly)
        return;

    auto it = std::find_if(linkFailures.begin(), linkFailures.end(),
                           [&](const LinkFailures& l) { return l.slot == slot && l.poly == linkPoly; });

    if (success)
    {
        if (it != linkFailures.end())
            linkFailures.erase(it);
        return;
    }

    if (it == linkFailures.end())
    {
        linkFailures.push_back({slot, linkPoly, 0});
        it = linkFailures.end() - 1;
    }

    if (++it->count >= kLinkFailuresToDisable)
    {
        NavMesh::DisableLink(slot, linkPoly);
        linkFailures.erase(it);
    }
}

void ReleasePath(PathHandle path)
{
    PathSlot* s = resolve(path);
    if (!s)
        return;

    if (s->status == PathStatus::Pending)
        inFlight--;
    s->inUse = false;
    s->generation++;
    s->status = PathStatus::Invalid;
    s->points.clear();
    s->points.shrink_to_fit();
    s->polys.clear();
    s->polys.shrink_to_fit();
    freeSlots.push_back(path.index);
}

// == synchronous queries ==
// ran on the main thread so we use it's own `dtNavMeshQuery`
namespace
{
dtNavMeshQuery* mainQuery(const AgentProfile& profile)
{
    // since we are using the main thread's `dtNavMeshQuery` we can't call these from other threads. 
    // maybe see about adapting it to be thread agnostic (ie a system to automatically get the corresponding thread dtNavMeshQuery)
    if (currentThreadID != THREAD_ID_MAIN) 
    {
        ERROR_MESSAGE("Tried to do a synchronous navmesh query from outside of the main thread, this is currently not possible");
        return nullptr;
    }

    const int slot = slotFor(profile, false);
    return slot >= 0 ? NavMesh::GetNavMeshQuery(slot) : nullptr;
}
} // namespace

bool IsOnNavmesh(vec3 pos, const AgentProfile& profile)
{
    dtNavMeshQuery* q = mainQuery(profile);
    if (!q)
        return false;

    const NavFilter filter;
    const vec3 ext(max(0.1f, profile.radius), profile.height, max(0.1f, profile.radius));
    dtPolyRef ref = 0;
    vec3 nearest;
    snapToMesh(q, filter, pos, ext, ref, nearest);
    if (!ref)
        return false;

    return length(vec2(nearest.x - pos.x, nearest.z - pos.z)) < 0.05f;
}

bool CanStartPath(vec3 pos, const AgentProfile& profile)
{
    dtNavMeshQuery* q = mainQuery(profile);
    if (!q)
        return false;

    const NavFilter filter;
    dtPolyRef ref = 0;
    vec3 nearest;
    return snapToMesh(q, filter, pos, agentExtents(profile), ref, nearest)
        && glm::abs(nearest.y - pos.y) <= profile.height * 0.5f;
}

bool FindRecoveryPoint(vec3 pos, vec3& out, const AgentProfile& profile, float maxDistance)
{
    dtNavMeshQuery* q = mainQuery(profile);
    if (!q)
        return false;

    constexpr int kMaxCandidates = 64;
    constexpr float kAbovePenalty = 4.0f;
    const NavFilter filter;
    const float underFeet = profile.radius * 2.0f + 0.1f;

    for (float radius = 2.0f; ; radius = min(radius * 2.0f, maxDistance))
    {
        const vec3 ext(radius, max(radius, profile.height), radius);
        dtPolyRef polys[kMaxCandidates];
        int count = 0;
        q->queryPolygons(ptr(pos), ptr(ext), &filter, polys, &count, kMaxCandidates);

        float best = FLT_MAX;
        for (int i = 0; i < count; i++)
        {
            vec3 p;
            if (dtStatusFailed(q->closestPointOnPoly(polys[i], ptr(pos), ptr(p), nullptr)))
                continue;

            const float horizontal = length(vec2(p.x - pos.x, p.z - pos.z));
            const float below = pos.y - p.y;
            if (below > profile.maxClimb + 0.1f && horizontal < underFeet)
                continue;

            const float vertical = below >= 0.0f ? below : -below * kAbovePenalty;
            const float cost = horizontal * horizontal + vertical * vertical;
            if (cost < best)
            {
                best = cost;
                out = p;
            }
        }
        if (best < FLT_MAX)
            return true;
        if (radius >= maxDistance)
            return false;
    }
}

bool ClosestPoint(vec3 pos, vec3& out, const AgentProfile& profile, float searchRadius)
{
    dtNavMeshQuery* q = mainQuery(profile);
    if (!q)
        return false;

    const NavFilter filter;
    const vec3 ext(searchRadius, max(searchRadius, profile.height), searchRadius);
    dtPolyRef ref = 0;
    snapToMesh(q, filter, pos, ext, ref, out);
    return ref != 0;
}

bool CanWalkStraight(vec3 from, vec3 to, const AgentProfile& profile)
{
    dtNavMeshQuery* q = mainQuery(profile);
    if (!q)
        return false;

    const NavFilter filter;
    const vec3 ext = agentExtents(profile);
    dtPolyRef startRef = 0;
    vec3 startPt;
    snapToMesh(q, filter, from, ext, startRef, startPt);
    if (!startRef)
        return false;

    // 2D raycast along the mesh surface
    constexpr int kMaxVisited = 64;
    float t = 0.0f;
    float hitNormal[3];
    dtPolyRef visited[kMaxVisited];
    int visitedCount = 0;
    const dtStatus st = q->raycast(startRef, ptr(startPt), ptr(to), &filter, &t, hitNormal, visited, &visitedCount, kMaxVisited);
    if (dtStatusFailed(st) || t != FLT_MAX || visitedCount == 0)
        return false;

    if (dtStatusDetail(st, DT_BUFFER_TOO_SMALL))
        return false;

    // the raycast is 2D so we reject targets on another floor
    float h = 0.0f;
    if (dtStatusFailed(q->getPolyHeight(visited[visitedCount - 1], ptr(to), &h)))
        return false;

    return glm::abs(h - to.y) <= profile.height * 0.5f;
}

bool MoveAlongSurface(vec3 from, vec3 to, vec3& out, const AgentProfile& profile)
{
    out = from;

    dtNavMeshQuery* q = mainQuery(profile);
    if (!q)
        return false;

    const NavFilter filter;
    const vec3 ext = agentExtents(profile);

    dtPolyRef startRef = 0;
    vec3 startPt;
    snapToMesh(q, filter, from, ext, startRef, startPt);
    if (!startRef)
        return false;

    constexpr int kMaxVisited = 16;
    dtPolyRef visited[kMaxVisited];
    int visitedCount = 0;
    vec3 result;
    const dtStatus st = q->moveAlongSurface(startRef, ptr(startPt), ptr(to), &filter,
                                            ptr(result), visited, &visitedCount, kMaxVisited);
    if (dtStatusFailed(st) || visitedCount == 0)
    {
        out = startPt;
        return true;
    }

    const dtPolyRef endRef = visited[visitedCount - 1];
    float h = 0.0f;
    if (dtStatusSucceed(q->getPolyHeight(endRef, ptr(result), &h)))
        result.y = h;
    else
        q->closestPointOnPoly(endRef, ptr(result), ptr(result), nullptr);

    out = result;
    return true;
}

// == path corridor ==

namespace
{
constexpr int kCorridorMaxPolys = kMaxPathPolys;

static_assert(sizeof(dtPolyRef) == sizeof(uint64_t));
static_assert(int(kCornerEnd) == int(DT_STRAIGHTPATH_END) && int(kCornerOffMesh) == int(DT_STRAIGHTPATH_OFFMESH_CONNECTION));
}

struct PathCorridor::Impl
{
    dtPathCorridor corridor;
    NavFilter filter;
    int meshSlot = -1;
    bool hasPath = false;

    Impl() { corridor.init(kCorridorMaxPolys); }

    dtNavMeshQuery* query() const
    {
        if (!hasPath || currentThreadID != THREAD_ID_MAIN)
            return nullptr;
        return NavMesh::GetNavMeshQuery(meshSlot);
    }
};

PathCorridor::PathCorridor() = default;
PathCorridor::~PathCorridor() = default;
PathCorridor::PathCorridor(const PathCorridor&) {}
PathCorridor& PathCorridor::operator=(const PathCorridor& o)
{
    if (this != &o)
        Clear();
    return *this;
}
PathCorridor::PathCorridor(PathCorridor&&) noexcept = default;
PathCorridor& PathCorridor::operator=(PathCorridor&&) noexcept = default;

bool PathCorridor::Adopt(PathHandle path, const AgentProfile& profile, const PathFilter& filter)
{
    const PathSlot* s = resolve(path);
    if (!s || s->polys.empty())
        return false;
    if (s->status != PathStatus::Complete && s->status != PathStatus::Partial)
        return false;

    const int meshSlot = slotFor(profile, false);
    if (meshSlot < 0)
        return false;

    if (!m)
        m = std::make_unique<Impl>();

    const int count = std::min((int)s->polys.size(), kCorridorMaxPolys);
    m->corridor.reset(s->polys.front(), ptr(s->pathStart));
    m->corridor.setCorridor(ptr(s->pathEnd), s->polys.data(), count);
    m->filter = NavFilter(filter);
    m->meshSlot = meshSlot;
    m->hasPath = true;
    return true;
}

void PathCorridor::Clear()
{
    if (m)
        m->hasPath = false;
}

bool PathCorridor::Empty() const
{
    return !m || !m->hasPath;
}

bool PathCorridor::MovePosition(vec3 pos)
{
    dtNavMeshQuery* q = m ? m->query() : nullptr;
    return q && m->corridor.movePosition(ptr(pos), q, &m->filter);
}

bool PathCorridor::MoveTarget(vec3 target)
{
    dtNavMeshQuery* q = m ? m->query() : nullptr;
    if (!q || !m->corridor.moveTargetPosition(ptr(target), q, &m->filter))
        return false;

    // moveTargetPosition keeps the old height, which hides a goal that moved to another floor
    const int n = m->corridor.getPathCount();
    vec3 t = Target();
    float h = t.y;
    if (n > 0 && dtStatusSucceed(q->getPolyHeight(m->corridor.getPath()[n - 1], ptr(t), &h)))
    {
        static std::vector<dtPolyRef> scratch; // setCorridor memcpys, can't pass it its own path
        scratch.assign(m->corridor.getPath(), m->corridor.getPath() + n);
        t.y = h;
        m->corridor.setCorridor(ptr(t), scratch.data(), n);
    }
    return true;
}

void PathCorridor::OptimizeVisibility(vec3 next, float range)
{
    if (dtNavMeshQuery* q = m ? m->query() : nullptr)
        m->corridor.optimizePathVisibility(ptr(next), range, q, &m->filter);
}

bool PathCorridor::IsValid(int lookAhead)
{
    dtNavMeshQuery* q = m ? m->query() : nullptr;
    return q && m->corridor.isValid(lookAhead, q, &m->filter);
}

bool PathCorridor::CanSee(vec3 to)
{
    dtNavMeshQuery* q = m ? m->query() : nullptr;
    if (!q)
        return false;

    float t = 0.0f;
    float hitNormal[3];
    dtPolyRef visited[16];
    int visitedCount = 0;
    const dtStatus st = q->raycast(m->corridor.getFirstPoly(), m->corridor.getPos(), ptr(to), &m->filter,
                                   &t, hitNormal, visited, &visitedCount, 16);
    return dtStatusSucceed(st) && t == FLT_MAX;
}

int PathCorridor::FindCorners(Corner* out, int maxCorners)
{
    dtNavMeshQuery* q = m ? m->query() : nullptr;
    if (!q)
        return 0;

    maxCorners = std::min(maxCorners, kMaxCorners);
    float verts[kMaxCorners * 3];
    unsigned char flags[kMaxCorners];
    dtPolyRef polys[kMaxCorners];
    const int n = m->corridor.findCorners(verts, flags, polys, maxCorners, q, &m->filter);

    for (int i = 0; i < n; i++)
        out[i] = {vec3(verts[i * 3], verts[i * 3 + 1], verts[i * 3 + 2]), flags[i], polys[i]};
    return n;
}

bool PathCorridor::MoveOverLink(uint64_t linkPoly, vec3& start, vec3& end, uint32_t& userId)
{
    dtNavMeshQuery* q = m ? m->query() : nullptr;
    if (!q)
        return false;

    dtPolyRef refs[2];
    if (!m->corridor.moveOverOffmeshConnection(linkPoly, refs, ptr(start), ptr(end), q))
        return false;

    const dtOffMeshConnection* con = q->getAttachedNavMesh()->getOffMeshConnectionByRef(linkPoly);
    userId = con ? con->userId : 0;
    return true;
}

float PathCorridor::WidthAt(vec3 at, vec2 dir, float maxHalfWidth, float* left, float* right)
{
    const float unknown = 2.0f * maxHalfWidth;
    if (left)
        *left = maxHalfWidth;
    if (right)
        *right = maxHalfWidth;
    dtNavMeshQuery* q = m ? m->query() : nullptr;
    if (!q || length(dir) < 1e-4f)
        return unknown;

    const vec3 ext(0.5f, 1.0f, 0.5f);
    dtPolyRef ref = 0;
    vec3 p;
    if (dtStatusFailed(q->findNearestPoly(ptr(at), ptr(ext), &m->filter, &ref, ptr(p))) || !ref)
        return unknown;

    const vec2 side = normalize(vec2(-dir.y, dir.x)) * maxHalfWidth;
    float room[2];
    for (int i = 0; i < 2; i++)
    {
        const float s = i == 0 ? 1.0f : -1.0f;
        vec3 to(p.x + side.x * s, p.y, p.z + side.y * s);
        float t = 0.0f;
        float hitNormal[3];
        if (dtStatusFailed(q->raycast(ref, ptr(p), ptr(to), &m->filter, &t, hitNormal, nullptr, nullptr, 0)))
            return unknown;
        room[i] = (t == FLT_MAX ? 1.0f : t) * maxHalfWidth;
    }
    if (left)
        *left = room[0];
    if (right)
        *right = room[1];
    return room[0] + room[1];
}

vec3 PathCorridor::Position() const
{
    if (!m)
        return vec3(0);
    const float* p = m->corridor.getPos();
    return vec3(p[0], p[1], p[2]);
}

vec3 PathCorridor::Target() const
{
    if (!m)
        return vec3(0);
    const float* p = m->corridor.getTarget();
    return vec3(p[0], p[1], p[2]);
}

bool RandomPointAround(vec3 center, float radius, vec3& out, const AgentProfile& profile)
{
    dtNavMeshQuery* q = mainQuery(profile);
    if (!q)
        return false;

    const NavFilter filter;
    const vec3 ext = agentExtents(profile);
    dtPolyRef centerRef = 0;
    vec3 centerPt;
    snapToMesh(q, filter, center, ext, centerRef, centerPt);
    if (!centerRef)
        return false;

    dtPolyRef randomRef = 0;
    const dtStatus st = q->findRandomPointAroundCircle(centerRef, ptr(centerPt), radius, &filter,
                                                       &frand, &randomRef, ptr(out));
    return dtStatusSucceed(st) && randomRef != 0;
}

} // namespace VulpineNav::NavServer

