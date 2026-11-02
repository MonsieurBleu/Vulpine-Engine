#include "Navigation/NavServer.hpp"
#include "Navigation/NavMesh.hpp"
#include "Utils.hpp"

#include <atomic>
#include <cassert>
#include <cfloat>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <random>
#include <thread>

#include <recastnavigation/DetourNavMesh.h>
#include <recastnavigation/DetourNavMeshQuery.h>

namespace VulpineNav::NavServer
{
AgentProfile const AgentProfile::Humanoid = {
    .radius = 0.25f,
    .height = 2.0f,
    .maxClimb = 0.5f,
    .maxSlopeDeg = 45.0f
};

namespace
{
constexpr i64 kFramesPerPriority = 60;

constexpr int kWorkerQueryNodes = 8192;

constexpr int kMaxPathPolys = 512;
constexpr int kMaxStraightPoints = 256;

// when failing to build a path, will retry every `kRetryDelayFrames` frames, a maximum of `kMaxStartAttempts` times. 
// if it still fails after that, the request returns NoPath
// maybe consider changing this to time elapsed instead of frame count
constexpr u64 kRetryDelayFrames = 10;
constexpr u32 kMaxStartAttempts = 30; // about 5 seconds at 60fps

constexpr u32 kDeadGeneration = 0xffffffff;
constexpr u32 kNoIndex = 0xffffffff;

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

    bool operator>(const QueryRequest& o) const { return key > o.key; }
};

struct QueryResult
{
    QueryRequest req;
    PathStatus status = PathStatus::NoPath;
    bool startMissing = false;
    std::vector<vec3> points;
};

struct PathSlot
{
    u32 generation = 0;
    bool inUse = false;
    PathStatus status = PathStatus::Invalid;
    std::vector<vec3> points;
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
std::vector<PathSlot> slots;
std::vector<u32> freeSlots;
std::vector<QueryRequest> newRequests;
std::vector<QueryResult> finishedScratch;
std::vector<DelayedRetry> retries;
std::vector<CachedProfile> profileCache;

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
std::thread worker;
std::mutex queueMutex;
std::condition_variable cv;
bool worker_quit = false;
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

// if goal is unreachable (no navmesh): 
// walk from goal to start and use the first valid point as a proxy goal
bool findProxyGoal(dtNavMeshQuery* q, const dtQueryFilter& filter, const QueryRequest& req,
                   dtPolyRef& outRef, vec3& outPt)
{
    const vec3 delta = req.goal - req.start;
    const float dist = length(vec2(delta.x, delta.z));
    if (dist < 0.001f)
        return false;

    const vec3 ext(req.extents.x, max(req.extents.y, 32.0f), req.extents.z);
    const float step = kTileWorldSize * 0.5f;

    for (float d = dist - step; d > 0.0f; d -= step)
    {
        const vec3 p = req.start + delta * (d / dist);
        snapToMesh(q, filter, p, ext, outRef, outPt);
        if (outRef)
            return true;
    }
    return false;
}

QueryResult runQuery(const QueryRequest& req, dtNavMeshQuery* q)
{
    QueryResult res;
    res.req = req;

    // TODO: virtual filter
    const dtQueryFilter filter;

    dtPolyRef startRef = 0;
    vec3 startPt;
    snapToMesh(q, filter, req.start, req.extents, startRef, startPt);
    if (!startRef)
    {
        res.startMissing = true;
        return res;
    }

    bool partial = false;

    dtPolyRef endRef = 0;
    vec3 endPt;
    snapToMesh(q, filter, req.goal, req.extents, endRef, endPt);
    if (!endRef)
    {
        if (!findProxyGoal(q, filter, req, endRef, endPt))
            return res; // no path at all
        partial = true;
    }

    dtPolyRef polys[kMaxPathPolys];
    int polyCount = 0;
    const dtStatus st = q->findPath(startRef, endRef, ptr(startPt), ptr(endPt), &filter,
                                    polys, &polyCount, kMaxPathPolys);
    if (dtStatusFailed(st) || polyCount == 0)
        return res;

    if (dtStatusDetail(st, DT_PARTIAL_RESULT) || dtStatusDetail(st, DT_OUT_OF_NODES)
        || dtStatusDetail(st, DT_BUFFER_TOO_SMALL))
        partial = true;

    // partial corridor
    vec3 pathEnd = endPt;
    if (polys[polyCount - 1] != endRef)
    {
        q->closestPointOnPoly(polys[polyCount - 1], ptr(endPt), ptr(pathEnd), nullptr);
        partial = true;
    }

    float straight[kMaxStraightPoints * 3];
    int straightCount = 0;
    q->findStraightPath(ptr(startPt), ptr(pathEnd), polys, polyCount,
                        straight, nullptr, nullptr, &straightCount, kMaxStraightPoints, 0);
    if (straightCount == 0)
        return res;
    if (straightCount == kMaxStraightPoints)
        partial = true;

    res.points.resize(straightCount);
    for (int i = 0; i < straightCount; i++)
        res.points[i] = vec3(straight[i * 3], straight[i * 3 + 1], straight[i * 3 + 2]);

    res.status = partial ? PathStatus::Partial : PathStatus::Complete;
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
void WorkerLoop()
{
    currentThreadID = THREAD_ID_NAVMESH_QUERY;

    std::vector<dtNavMeshQuery*> queries;

    while (true)
    {
        QueryRequest req;
        {
            std::unique_lock lock(queueMutex);
            cv.wait(lock, [] { return worker_quit || !requestQueue.empty(); });
            if (worker_quit)
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

    {
        std::lock_guard lock(queueMutex);
        worker_quit = false;
    }
    frame = 0;
    worker = std::thread(&WorkerLoop);
}

void Shutdown()
{
    {
        std::lock_guard lock(queueMutex);
        worker_quit = true;
    }
    cv.notify_one();

    NavMesh::CancelWriterWait();

    if (worker.joinable())
        worker.join();

    while (!requestQueue.empty()) requestQueue.pop();
    finished.clear();
    finishedScratch.clear();
    liveGeneration.clear();
    newRequests.clear();
    retries.clear();
    slots.clear();
    freeSlots.clear();
    profileCache.clear();
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
            slot.status = PathStatus::NoPath;
            slot.points.clear();
            debugRecord(req, slot.status, slot.points);
            continue;
        }

        slot.status = res.status;
        slot.points = std::move(res.points);
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

PathHandle RequestPath(vec3 start, vec3 goal, const AgentProfile& profile, Priority priority)
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

    const PathHandle handle{index, slot.generation};

    const int meshSlot = slotFor(profile, true);
    if (meshSlot < 0)
    {
        slot.status = PathStatus::NoPath;
        return handle;
    }

    slot.status = PathStatus::Pending;
    newRequests.push_back({
        (i64)frame - (i64)priority * kFramesPerPriority,
        index, slot.generation, meshSlot,
        start, goal, agentExtents(profile), 0});

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

void ReleasePath(PathHandle path)
{
    PathSlot* s = resolve(path);
    if (!s)
        return;

    s->inUse = false;
    s->generation++;
    s->status = PathStatus::Invalid;
    s->points.clear();
    s->points.shrink_to_fit();
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

    const dtQueryFilter filter;
    const vec3 ext(max(0.1f, profile.radius), profile.height, max(0.1f, profile.radius));
    dtPolyRef ref = 0;
    vec3 nearest;
    snapToMesh(q, filter, pos, ext, ref, nearest);
    if (!ref)
        return false;

    return length(vec2(nearest.x - pos.x, nearest.z - pos.z)) < 0.05f;
}

bool ClosestPoint(vec3 pos, vec3& out, const AgentProfile& profile, float searchRadius)
{
    dtNavMeshQuery* q = mainQuery(profile);
    if (!q)
        return false;

    const dtQueryFilter filter;
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

    const dtQueryFilter filter;
    const vec3 ext = agentExtents(profile);
    dtPolyRef startRef = 0;
    vec3 startPt;
    snapToMesh(q, filter, from, ext, startRef, startPt);
    if (!startRef)
        return false;

    // 2D raycast along the mesh surface
    float t = 0.0f;
    float hitNormal[3];
    dtPolyRef visited[64];
    int visitedCount = 0;
    q->raycast(startRef, ptr(startPt), ptr(to), &filter, &t, hitNormal, visited, &visitedCount, 64);
    return t == FLT_MAX;
}

bool MoveAlongSurface(vec3 from, vec3 to, vec3& out, const AgentProfile& profile)
{
    out = from;

    dtNavMeshQuery* q = mainQuery(profile);
    if (!q)
        return false;

    const dtQueryFilter filter;
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

bool RandomPointAround(vec3 center, float radius, vec3& out, const AgentProfile& profile)
{
    dtNavMeshQuery* q = mainQuery(profile);
    if (!q)
        return false;

    const dtQueryFilter filter;
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

