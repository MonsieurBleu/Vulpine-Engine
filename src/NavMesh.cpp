#include "AssetManager.hpp"
#include "Globals.hpp"
#include "Graphics/ObjectGroup.hpp"
#include "MathsUtils.hpp"
#include "Utils.hpp"

#include <array>
#include <atomic>
#include <condition_variable>
#include <queue>
#include <thread>

#include "JoltIntegration/PhysicsCommons.hpp"

#ifndef DT_VIRTUAL_QUERYFILTER
#error "DT_VIRTUAL_QUERYFILTER must be defined."
#endif
#ifndef DT_POLYREF64
#error "DT_POLYREF64 must be defined."
#endif

#include <recastnavigation/Recast.h>
#include <recastnavigation/DetourNavMesh.h>
#include <recastnavigation/DetourNavMeshQuery.h>
#include <recastnavigation/DetourNavMeshBuilder.h>

#include "Navigation/NavMesh.hpp"

#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/Shape/HeightFieldShape.h>
#include <Jolt/Geometry/OrientedBox.h>

namespace VulpineNav
{

namespace MeshTier
{

TierRegistry::GridKey TierRegistry::gridKeyOf(const AgentShape& shape) const {
    auto stepsUp   = [](float v, float step) { return u16(ceil(v / step)); };
    auto stepsDown = [](float v, float step) { return u16(floor(v / step)); };
    return GridKey{
        stepsUp(shape.radius,        TierBucketingPolicy::radiusStep),
        stepsUp(shape.height,        TierBucketingPolicy::heightStep),
        stepsDown(shape.maxClimb,    TierBucketingPolicy::climbStep),
        stepsDown(shape.maxSlopeDeg, TierBucketingPolicy::slopeStepDeg),
    };
}

// for some frigging reason ceil is not registering as constexpr correctly in clang (even though it should)
constexpr int32_t constCeil(float num)
{
    return (static_cast<float>(static_cast<int32_t>(num)) == num)
        ? static_cast<int32_t>(num)
        : static_cast<int32_t>(num) + ((num > 0) ? 1 : 0);
}

MeshTier TierRegistry::tier(TierId id) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (id >= m_tiers.size())
    {
        ERROR_MESSAGE("TierRegistry::tier called with an unminted TierId. Returning a default tier.")
        return MeshTier{};
    }
    return m_tiers[id];
}

std::vector<MeshTier> TierRegistry::tiers() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_tiers;
}

size_t TierRegistry::tierCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_tiers.size();
}

bool TierRegistry::valid(TierId id) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return id < m_tiers.size();
}

TierId TierRegistry::resolve(const AgentShape& shape) {
    std::lock_guard<std::mutex> lock(m_mutex);

    const GridKey exact = gridKeyOf(shape);

    if (auto it = m_tierIndexer.find(exact); it != m_tierIndexer.end())
        return it->second;

    // NOTE: these will resolve to 1 by default since the slack is the size of the step
    //       unless we modify it later during testing
    constexpr i32 radiusSpan = max(1, constCeil(TierBucketingPolicy::maxRadiusSlack   / TierBucketingPolicy::radiusStep));
    constexpr i32 heightSpan = max(1, constCeil(TierBucketingPolicy::maxHeightSlack   / TierBucketingPolicy::heightStep));
    constexpr i32 climbSpan  = max(1, constCeil(TierBucketingPolicy::maxClimbSlack    / TierBucketingPolicy::climbStep));
    constexpr i32 slopeSpan  = max(1, constCeil(TierBucketingPolicy::maxSlopeSlackDeg / TierBucketingPolicy::slopeStepDeg));

    TierId best      = kInvalidTier;
    float  bestSlack = std::numeric_limits<float>::max();

    // refer to the previous note, this might look scary but it's actually just 2^4 = 16 iterations
    for (i32 dr = 0; dr <= radiusSpan; ++dr)
    for (i32 dh = 0; dh <= heightSpan; ++dh)
    for (i32 dc = 0; dc <= climbSpan;  ++dc)
    for (i32 ds = 0; ds <= slopeSpan;  ++ds) {
        GridKey k{ u16(exact.radiusSteps + dr), u16(exact.heightSteps + dh),
                   u16(exact.climbSteps  - dc), u16(exact.slopeSteps  - ds) };
        auto it = m_tierIndexer.find(k);
        if (it == m_tierIndexer.end()) continue;

        const MeshTier& t = m_tiers[it->second];
        if (!covers(t, shape)) continue;

        const float slack = (t.radius - shape.radius)      / TierBucketingPolicy::radiusStep
                           + (t.height - shape.height)      / TierBucketingPolicy::heightStep
                           + (shape.maxClimb - t.maxClimb)  / TierBucketingPolicy::climbStep
                           + (shape.maxSlopeDeg - t.maxSlopeDeg) / TierBucketingPolicy::slopeStepDeg;
        if (slack < bestSlack) { bestSlack = slack; best = it->second; }
    }


    if (best != kInvalidTier) // NOLINT
       return best;

    if (m_tiers.size() >= size_t(kInvalidTier))
    {
        ERROR_MESSAGE("TierRegistry exhausted the TierId space (65535 tiers). Something is minting tiers per spawn instead of caching the id. Reusing tier 0.")
        return TierId(0);
    }

    const TierId id = TierId(m_tiers.size());
    m_tiers.push_back(quantize(shape, id));
    m_tierIndexer.emplace(exact, id);
    return id;
}

} // namespace MeshTier

// internals & stuff
namespace NavMesh::detail
{
static constexpr i64 kFramesPerPriority = 60;
static constexpr int kMaxIdx = worldTileLength - 1;
static constexpr u32 kMaxTiers = 64;


enum TileState : u8 { Unloaded, Pending, Loaded };

struct TileGrid
{
    std::array<u16, worldTileLength * worldTileLength> nearCount;
    std::array<u16, worldTileLength * worldTileLength> farCount;
    std::array<std::atomic<u8>, worldTileLength * worldTileLength> loaded;
    std::array<TileState, worldTileLength * worldTileLength> state;
};

struct NavMeshData
{
    dtNavMesh* navMesh;
    dtNavMeshQuery* navMeshQuery;
    MeshTier::TierId key;
};

struct LoadRequest
{
    i64 key;
    size_t idx;
    u64 slotMask;
    bool operator>(const LoadRequest& o) const { return key > o.key; }
};

struct BuiltTile
{
    u8* data;
    int size;
    u8 slot;
};

struct TileGeometry
{
    std::vector<float> verts;  // xyz, unindexed: 3 verts per triangle
    std::vector<int>   tris;
    float minY = 0.0f;
    float maxY = 0.0f;

    void clear() { verts.clear(); tris.clear(); minY = FLT_MAX; maxY = -FLT_MAX; }
    int vertCount() const { return (int)verts.size() / 3; }
    int triCount() const { return (int)tris.size() / 3; }
};

struct RecastScratch
{
    rcHeightfield*        hf    = nullptr;
    rcCompactHeightfield* chf   = nullptr;
    rcContourSet*         cset  = nullptr;
    rcPolyMesh*           pmesh = nullptr;
    rcPolyMeshDetail*     dmesh = nullptr;

    ~RecastScratch();
};


struct BuildResult
{
    LoadRequest req;
    bool skipped; // tile was unwanted when the worker picked it up (maybe it was unloaded before it could build it)
    u64 builtMask; // tiers the worker attempted
    std::vector<BuiltTile> tiles;
};

struct SpanPair
{
    ivec2 left;
    ivec2 right;
};

// main thread state
TileGrid grid;
std::array<NavMeshData, kMaxTiers> navMeshes{};
std::atomic<u32> navMeshCount = 0;

std::vector<ivec2> pendingRemovals;
u64 meshVersion = 0;
MeshTier::TierRegistry tierRegistry;
u64 frame = 0;
std::vector<LoadRequest> newRequests;
std::vector<BuildResult> resultsScratch;

// worker state
TileGeometry workerGeo;
thread_local std::vector<u8> workerAreas;

NavMeshData createMeshData(const MeshTier::MeshTier& tier);

// helpers
u64 allSlots(size_t count) 
{ 
    return count >= 64 ? ~0ull : (1ull << count) - 1; 
}

void tileWorldBounds(const ivec2& coord, float padding, vec3& bmin, vec3& bmax)
{
    bmin.x = kWorldOrigin.x + coord.x * kTileWorldSize - padding;
    bmin.y = kWorldMinY;
    bmin.z = kWorldOrigin.z + coord.y * kTileWorldSize - padding;

    bmax.x = kWorldOrigin.x + (coord.x + 1) * kTileWorldSize + padding;
    bmax.y = kWorldMaxY;
    bmax.z = kWorldOrigin.z + (coord.y + 1) * kTileWorldSize + padding;
}

// because of how recast builds tiled navmeshes, we need a small padding around each cell that it can then merge with other tiles
// so we iterate over every tier to find the largest one to use
int borderCells(const MeshTier::MeshTier& tier)
{
    return (int)ceilf(tier.radius / kCellSize) + 3;
}

float gatherPadding(u64 mask, const std::vector<MeshTier::MeshTier>& tiers)
{
    int cells = 0;
    for (u32 s = 0; s < tiers.size(); s++)
    {
        if (mask & (1ull << s))
            cells = max(cells, borderCells(tiers[s]));
    }
    return cells * kCellSize;
}


i64 requestKey(u8 priority)
{ 
    return (i64)frame - (i64)priority * kFramesPerPriority; 
}

ivec2 rowRange(int yMin, int yMax) 
{ 
    return {max(yMin, 0), min(yMax, kMaxIdx)}; 
}

ivec2 clampSpan(const ivec2& s)
{ 
    return {max(s.x, 0), min(s.y, kMaxIdx)}; 
}

ivec2 rowSpan(const ivec2& cell, int radius, int y);

SpanPair subtractSpan(const ivec2& a, const ivec2& b)
{
    return {{a.x, min(a.y, b.x - 1)}, {max(a.x, b.y + 1), a.y}};
}

// base tile update methods
void addNear(int y, ivec2 span, ivec2 center, i64 baseKey);
void removeFar(int y, ivec2 span);
void updateRow(int y, ivec2 prevTile, ivec2 newTile, ivec2 prevUnload, ivec2 newUnload, i64 baseKey);

void removeNear(int y, ivec2 span)
{
    span = clampSpan(span);
    u16* nearRow = &grid.nearCount[(size_t)y * worldTileLength];

    for (int x = span.x; x <= span.y; ++x)
        --nearRow[x];
}

void addFar(int y, ivec2 span)
{
    span = clampSpan(span);
    u16* farRow = &grid.farCount[(size_t)y * worldTileLength];

    for (int x = span.x; x <= span.y; ++x)
        ++farRow[x];
}

void requestTile(size_t i, i64 key)
{
    if (grid.state[i] != Unloaded) // tile already loaded or pending 
        return;

    grid.state[i] = Pending;
    newRequests.push_back({key, i, 0});
}

void releaseTile(int x, int y);

void addMeshTile(NavMeshData& mesh, int x, int y, u8* data, int size);

void removeMeshTile(NavMeshData& mesh, int x, int y);

void addAll(BuildResult& r);

void freeAll(BuildResult& r);

void integrate(BuildResult& r);
BuildResult buildRequest(const LoadRequest& req, u64 mask, const std::vector<MeshTier::MeshTier>& tiers);
void workerLoop(std::stop_token stop_token);
int addTier(const MeshTier::MeshTier& tier);

void gatherTileGeometry(ivec2 coord, float padding, TileGeometry& geo);

bool buildTierTile(ivec2 coord, const MeshTier::MeshTier& tier, const TileGeometry& geo, u8*& data, int& size,
                   DebugBuildStages* dbg = nullptr);

// shared with the nav server query thread
std::mutex navMutex;
std::atomic<bool> writerWaiting = false;

// shared with worker
std::jthread worker;
std::mutex requestMutex;
std::condition_variable cv;
std::priority_queue<LoadRequest, std::vector<LoadRequest>, std::greater<LoadRequest>> buildQueue;
std::vector<BuildResult> results;
std::vector<MeshTier::MeshTier> buildTiers;

rcContext workerCtx {false};

RecastScratch::~RecastScratch()
{
    rcFreeHeightField(hf);
    rcFreeCompactHeightfield(chf);
    rcFreeContourSet(cset);
    rcFreePolyMesh(pmesh);
    rcFreePolyMeshDetail(dmesh);
}

void removeMeshTile(NavMeshData& mesh, int x, int y)
{
    dtTileRef ref = mesh.navMesh->getTileRefAt(x, y, 0);
    if (ref)
        mesh.navMesh->removeTile(ref, nullptr, nullptr);
}

void freeAll(BuildResult& r)
{
    for (BuiltTile& t : r.tiles)
        dtFree(t.data);
    r.tiles.clear();
}

rcConfig makeTileConfig(const MeshTier::MeshTier& tier, ivec2 coord, const TileGeometry& geo);

void appendShapeTriangles(const JPH::TransformedShape& shape, const JPH::AABox& box, TileGeometry& geo);

} // namespace NavMesh::detail

using namespace NavMesh::detail;

bool NavMesh::ValidSlot(int slot)
{
    return slot >= 0 && (u32)slot < detail::navMeshCount.load(std::memory_order_acquire);
}
dtNavMesh* NavMesh::GetNavMesh(int slot)
{
    return ValidSlot(slot) ? detail::navMeshes[slot].navMesh : nullptr;
}
dtNavMeshQuery* NavMesh::GetNavMeshQuery(int slot)
{
    return ValidSlot(slot) ? detail::navMeshes[slot].navMeshQuery : nullptr;
}
int NavMesh::SlotFromTierId(MeshTier::TierId id)
{
    const u32 count = detail::navMeshCount.load(std::memory_order_acquire);

    for (u32 s = 0; s < count; s++)
        if (detail::navMeshes[s].key == id)
            return (int)s;
    return -1;
}

int NavMesh::FindAgentSlot(const MeshTier::AgentShape& shape)
{
    const u32 count = detail::navMeshCount.load(std::memory_order_acquire);

    for (u32 s = 0; s < count; s++)
    {
        if (MeshTier::covers(detail::tierRegistry.tier(detail::navMeshes[s].key), shape))
            return (int)s;
    }
    return -1;
}

ivec2 NavMesh::detail::rowSpan(const ivec2& cell, int radius, int y)
{
    int dy = y - cell.y;

    if (dy < -radius || dy > radius)
        return {0, -1};

    i64 rem = (i64)radius * radius - (i64)dy * dy;
    i64 w = (i64)sqrt((double)rem);

    while (w * w > rem) w--;
    while ((w + 1) * (w + 1) <= rem) w++;

    return {cell.x - (int)w, cell.x + (int)w};
}

void NavMesh::detail::addNear(int y, ivec2 span, ivec2 center, i64 baseKey)
{
    span = clampSpan(span);
    const int dy = y - center.y;
    const i64 rowKey = baseKey + (i64)dy * dy;

    u16* nearRow = &grid.nearCount[(size_t)y * worldTileLength];
    std::atomic<u8>* loadedRow = &grid.loaded[(size_t)y * worldTileLength];

    for (int x = span.x; x <= span.y; ++x)
    {
        ++nearRow[x];
        if (!loadedRow[x])
        {
            loadedRow[x] = 1;
            const int dx = x - center.x;
            requestTile(idxFromTileCoord(x, y), rowKey + (i64)dx * dx);
        }
    }
}

void NavMesh::detail::removeFar(int y, ivec2 span)
{
    span = clampSpan(span);
    u16* farRow = &grid.farCount[(size_t)y * worldTileLength];
    std::atomic<u8>* loadedRow = &grid.loaded[(size_t)y * worldTileLength];

    for (int x = span.x; x <= span.y; ++x)
    {
        if (--farRow[x] == 0 && loadedRow[x])
        {
            loadedRow[x] = 0;
            releaseTile(x, y);
        }
    }
}

void NavMesh::detail::updateRow(int y, ivec2 prevTile, ivec2 newTile, ivec2 prevUnload, ivec2 newUnload, i64 baseKey)
{
    const ivec2 prevLoad = rowSpan(prevTile, kLoadRadius, y);
    const ivec2 newLoad  = rowSpan(newTile, kLoadRadius, y);

    const SpanPair leftLoad    = subtractSpan(prevLoad, newLoad);
    const SpanPair enteredFar  = subtractSpan(newUnload, prevUnload);
    const SpanPair enteredLoad = subtractSpan(newLoad, prevLoad);
    const SpanPair leftFar     = subtractSpan(prevUnload, newUnload);

    removeNear(y, leftLoad.left);
    removeNear(y, leftLoad.right);

    addFar(y, enteredFar.left);
    addFar(y, enteredFar.right);

    addNear(y, enteredLoad.left, newTile, baseKey);
    addNear(y, enteredLoad.right, newTile, baseKey);

    removeFar(y, leftFar.left);
    removeFar(y, leftFar.right);
}

void NavMesh::detail::releaseTile(int x, int y)
{
    size_t i = idxFromTileCoord(x, y);
    if (grid.state[i] != Loaded) // tile already unloaded
        return;

    pendingRemovals.push_back({x, y});

    grid.state[i] = Unloaded;
}

void NavMesh::detail::addMeshTile(NavMeshData& mesh, int x, int y, u8* data, int size)
{
    if (mesh.navMesh->getTileAt(x, y, 0) ||
        dtStatusFailed(mesh.navMesh->addTile(data, size, DT_TILE_FREE_DATA, 0, nullptr)))
    {
        dtFree(data);
    }
}

void NavMesh::detail::addAll(BuildResult& r)
{
    const ivec2 c = coordFromIdx(r.req.idx);
    for (BuiltTile& t : r.tiles)
        addMeshTile(navMeshes[t.slot], c.x, c.y, t.data, t.size);
    r.tiles.clear();
}

void NavMesh::detail::integrate(BuildResult& r)
{
    const u32 i = r.req.idx;
    const bool wanted = grid.loaded[i];

    // backfill, we added a new tier and so we must populate the tiles already loaded
    if (r.req.slotMask != 0)
    {
        if (wanted && grid.state[i] == Loaded)
            addAll(r);
        else
            freeAll(r);
        return;
    }

    // resolve pending tile

    if (!wanted)
    {
        freeAll(r);
        grid.state[i] = Unloaded;
        return;
    }

    if (r.skipped) 
    {
        newRequests.push_back(r.req);
        return;
    }

    addAll(r);
    grid.state[i] = Loaded;

    // tiers added while this tile was being built
    const u64 missing = allSlots(navMeshCount.load(std::memory_order_relaxed)) & ~r.builtMask;
    if (missing)
        newRequests.push_back({r.req.key, i, missing});
}


BuildResult NavMesh::detail::buildRequest(const LoadRequest& req, u64 mask, const std::vector<MeshTier::MeshTier>& tiers)
{
    BuildResult r{req, true, 0, {}};

    if (!grid.loaded[req.idx].load(std::memory_order_relaxed) || !JoltVulpine::jPhysicsSystem)
        return r;

    r.skipped = false;
    r.builtMask = mask;

    ivec2 coord = coordFromIdx(req.idx);
    gatherTileGeometry(coord, gatherPadding(mask, tiers), workerGeo);

    if (workerGeo.triCount() == 0)
        return r;

    for (u32 s = 0; s < tiers.size(); s++)
    {
        if (!(mask & (1ull << s)))
            continue;

        BuiltTile t{nullptr, 0, (u8)s};
        if (!buildTierTile(coord, tiers[s], workerGeo, t.data, t.size))
        {
            WARNING_MESSAGE("navmesh tile build failed: ", coord.x, ", ", coord.y, " tier ", tiers[s].id);
        }
        else if (t.data)
        {
            r.tiles.push_back(t);
        }
    }
    return r;
}

void NavMesh::detail::workerLoop(std::stop_token stop_token)
{
    currentThreadID = THREAD_ID_NAVMESH_BUILD;

    std::vector<MeshTier::MeshTier> tiers; // snapshot of the tiers

    while (!stop_token.stop_requested())
    {
        LoadRequest req;
        u64 mask;

        // block for mutex lock scope
        {
            std::unique_lock<std::mutex> lock(requestMutex);
            while (!stop_token.stop_requested() && buildQueue.empty())
                cv.wait(lock);
            if (stop_token.stop_requested())
                return;

            req = buildQueue.top();
            buildQueue.pop();

            const u64 existing = allSlots(buildTiers.size());
            mask = req.slotMask ? (req.slotMask & existing) : existing;
            tiers.assign(buildTiers.begin(), buildTiers.end());
        }

        BuildResult r = buildRequest(req, mask, tiers);

        std::lock_guard<std::mutex> lock(requestMutex);
        results.push_back(std::move(r));
    }
}

int NavMesh::detail::addTier(const MeshTier::MeshTier& tier)
{
    const u32 count = navMeshCount.load(std::memory_order_relaxed);
    assert(count < kMaxTiers);

    const u8 slot = (u8)count;
    navMeshes[slot] = createMeshData(tier);
    navMeshCount.store(count + 1, std::memory_order_release);

    {
        std::lock_guard<std::mutex> lock(requestMutex);
        buildTiers.push_back(tier);
    }

    // tiles already loaded don't have this tier yet
    const i64 key = (i64)frame;
    for (u32 i = 0; i < grid.state.size(); i++)
    {
        if (grid.state[i] == Loaded)
            newRequests.push_back({key, i, 1ull << slot});
    }
    return slot;
}

void NavMesh::Shutdown()
{
    worker.get_stop_source().request_stop();
    cv.notify_one();
    if (worker.joinable())
        worker.join();

    while (!buildQueue.empty()) buildQueue.pop();
    for (BuildResult& r : results) freeAll(r);
    results.clear();
    for (BuildResult& r : resultsScratch) freeAll(r);
    resultsScratch.clear();
    pendingRemovals.clear();
}

int NavMesh::ResolveAgent(const MeshTier::AgentShape& shape)
{
    const MeshTier::TierId id = tierRegistry.resolve(shape);

    const u32 count = navMeshCount.load(std::memory_order_relaxed);
    for (u32 s = 0; s < count; s++)
    {
        if (navMeshes[s].key == id)
            return (int)s;
    }
    return addTier(tierRegistry.tier(id));
}

void NavMesh::Tick()
{
    frame++;

    {
        std::lock_guard<std::mutex> lock(requestMutex);
        for (LoadRequest& r : newRequests)
            buildQueue.push(r);
        resultsScratch.insert(resultsScratch.end(),
            std::make_move_iterator(results.begin()),
            std::make_move_iterator(results.end()));
        results.clear();
    }
    if (!newRequests.empty())
        cv.notify_one();
    newRequests.clear();

    if (pendingRemovals.empty() && resultsScratch.empty())
        return;

    if (!navMutex.try_lock())
    {
        writerWaiting.store(true, std::memory_order_release);
        return;
    }

    const u32 count = navMeshCount.load(std::memory_order_relaxed);
    for (const ivec2& c : pendingRemovals)
    {
        if (grid.loaded[idxFromTileCoord(c)]) // wanted again, keep the old tile
            continue;
        for (u32 s = 0; s < count; s++)
            removeMeshTile(navMeshes[s], c.x, c.y);
    }
    pendingRemovals.clear();

    for (BuildResult& r : resultsScratch)
        integrate(r);
    resultsScratch.clear();

    navMutex.unlock();

    meshVersion++;

    writerWaiting.store(false, std::memory_order_release);
    writerWaiting.notify_all();
}

NavMesh::QueryReadLock::QueryReadLock()
{
    writerWaiting.wait(true, std::memory_order_acquire);
    navMutex.lock();
}

NavMesh::QueryReadLock::~QueryReadLock()
{
    navMutex.unlock();
}

u64 NavMesh::GetVersion()
{
    return meshVersion;
}

void NavMesh::CancelWriterWait()
{
    writerWaiting.store(false, std::memory_order_release);
    writerWaiting.notify_all();
}

bool NavMesh::IsTileWanted(vec3 pos)
{
    const ivec2 c = worldToTileCoord(pos);
    if (c.x < 0 || c.y < 0 || c.x >= (int)worldTileLength || c.y >= (int)worldTileLength)
        return false;
    return grid.loaded[idxFromTileCoord(c)].load(std::memory_order_relaxed) != 0;
}

void NavMesh::CreateLoader(vec3 pos, u8 priority)
{
    const ivec2 tile = worldToTileCoord(pos);
    const i64 baseKey = requestKey(priority);
    const ivec2 rows = rowRange(tile.y - kUnloadRadius, tile.y + kUnloadRadius);

    if (tile.x < 0 || tile.y < 0 || tile.x > kMaxIdx || tile.y > kMaxIdx)
        WARNING_MESSAGE("NavMesh loader outside the navmesh grid: tile ", tile.x, ", ", tile.y);

    for (int y = rows.x; y <= rows.y; y++)
    {
        addFar(y, rowSpan(tile, kUnloadRadius, y));
        addNear(y, rowSpan(tile, kLoadRadius, y), tile, baseKey);
    }
}

void NavMesh::UpdateLoaderPosition(vec3 prevPos, vec3 newPos, u8 priority)
{
    const ivec2 prevTile = worldToTileCoord(prevPos);
    const ivec2 newTile = worldToTileCoord(newPos);

    if (prevTile == newTile)
        return;

    const i64 baseKey = requestKey(priority);
    const ivec2 rows = rowRange(min(prevTile.y, newTile.y) - kUnloadRadius,
                                max(prevTile.y, newTile.y) + kUnloadRadius);

    for (int y = rows.x; y <= rows.y; y++)
    {
        const ivec2 prevUnload = rowSpan(prevTile, kUnloadRadius, y);
        const ivec2 newUnload = rowSpan(newTile, kUnloadRadius, y);

        if (prevUnload.y < prevUnload.x && newUnload.y < newUnload.x)
        {
            y = max(prevTile.y, newTile.y) - kUnloadRadius - 1;
            continue;
        }

        updateRow(y, prevTile, newTile, prevUnload, newUnload, baseKey);
    }
}

void NavMesh::UnloadLoader(vec3 pos)
{
    const ivec2 tile = worldToTileCoord(pos);
    const ivec2 rows = rowRange(tile.y - kUnloadRadius, tile.y + kUnloadRadius);

    for (int y = rows.x; y <= rows.y; y++)
    {
        removeNear(y, rowSpan(tile, kLoadRadius, y));
        removeFar(y, rowSpan(tile, kUnloadRadius, y));
    }
}

void NavMesh::detail::appendShapeTriangles(const JPH::TransformedShape& shape, const JPH::AABox& box, TileGeometry& geo)
{
    // compound shapes don't iterate triangles directly, so walk their leaves.
    JPH::AllHitCollisionCollector<JPH::TransformedShapeCollector> leaves;
    shape.CollectTransformedShapes(box, leaves, JPH::ShapeFilter());

    constexpr int kBatch = 256;  // must be >= JPH::Shape::cGetTrianglesMinTrianglesRequested
    JPH::Float3 v[kBatch * 3];

    for (const JPH::TransformedShape& leaf : leaves.mHits)
    {
        JPH::Shape::GetTrianglesContext ctx;
        leaf.GetTrianglesStart(ctx, box, JPH::RVec3::sZero());

        while (int n = leaf.GetTrianglesNext(ctx, kBatch, v))
        {
            const int base = geo.vertCount();

            for (int i = 0; i < n * 3; i++)
            {
                geo.verts.push_back(v[i].x);
                geo.verts.push_back(v[i].y);
                geo.verts.push_back(v[i].z);
                geo.minY = min(geo.minY, v[i].y);
                geo.maxY = max(geo.maxY, v[i].y);
            }

            for (int i = 0; i < n * 3; i++)
                geo.tris.push_back(base + i);
        }
    }
}

void NavMesh::detail::gatherTileGeometry(ivec2 coord, float padding, TileGeometry& geo)
{
    geo.clear();

    Vvec3 bmin, bmax;
    tileWorldBounds(coord, padding, bmin, bmax);
    const JPH::AABox box = JPH::AABox(bmin, bmax);

    // acquire the physics lock for gathering the geometry (even if we only gather static bodies)
    std::lock_guard<std::mutex> physicsLock(JoltVulpine::physicsMutex);

    JPH::PhysicsSystem& physics = *JoltVulpine::jPhysicsSystem;

    JPH::AllHitCollisionCollector<JPH::CollideShapeBodyCollector> bodies;
    physics.GetBroadPhaseQuery().CollideAABox(box, bodies, JPH::BroadPhaseLayerFilter(), JPH::ObjectLayerFilter());

    const JPH::BodyLockInterface& locks = physics.GetBodyLockInterface();

    for (const JPH::BodyID& id : bodies.mHits)
    {
        JPH::TransformedShape shape;
        {
            JPH::BodyLockRead lock(locks, id);
            if (!lock.Succeeded())
                continue;

            const JPH::Body& body = lock.GetBody();
            if (!body.IsStatic() || body.IsSensor())
                continue;

            shape = body.GetTransformedShape();
        }
        appendShapeTriangles(shape, box, geo);
    }
}

rcConfig NavMesh::detail::makeTileConfig(const MeshTier::MeshTier& tier, ivec2 coord, const TileGeometry& geo)
{
    rcConfig cfg{};
    cfg.cs = kCellSize;
    cfg.ch = kCellHeight;
    cfg.walkableSlopeAngle = tier.maxSlopeDeg;
    // -epsilon so float noise doesn't cost a whole voxel
    cfg.walkableHeight = (int)ceilf(tier.height / cfg.ch - 1e-3f);
    cfg.walkableClimb = (int)floorf(tier.maxClimb / cfg.ch + 1e-3f);
    cfg.walkableRadius = (int)ceilf(tier.radius / cfg.cs);
    cfg.maxEdgeLen = (int)(12.0f / cfg.cs);
    cfg.maxSimplificationError = 1.3f;
    cfg.minRegionArea = rcSqr(4);
    cfg.mergeRegionArea = rcSqr(20);
    cfg.maxVertsPerPoly = DT_VERTS_PER_POLYGON;
    cfg.detailSampleDist = cfg.cs * 6;
    cfg.detailSampleMaxError = kDetailMaxError;

    cfg.tileSize = kTileCells;
    cfg.borderSize = borderCells(tier);
    cfg.width = cfg.tileSize + cfg.borderSize * 2;
    cfg.height = cfg.width;

    tileWorldBounds(coord, cfg.borderSize * cfg.cs, *(vec3*)cfg.bmin, *(vec3*)cfg.bmax);
    cfg.bmin[1] = geo.minY;
    cfg.bmax[1] = geo.maxY;
    return cfg;
}

namespace
{
// debug recording helpers, tile interior only
void recordHeightfield(const rcHeightfield& hf, int border, std::vector<vec3>& out)
{
    for (int y = border; y < hf.height - border; y++)
    for (int x = border; x < hf.width - border; x++)
        for (const rcSpan* s = hf.spans[x + y * hf.width]; s; s = s->next)
            if (s->area != RC_NULL_AREA)
                out.push_back(vec3(
                    hf.bmin[0] + (x + 0.5f) * hf.cs,
                    hf.bmin[1] + s->smax * hf.ch + 0.03f,
                    hf.bmin[2] + (y + 0.5f) * hf.cs));
}

void recordCompact(const rcCompactHeightfield& chf, int border, bool regionsOnly, std::vector<vec3>& out)
{
    for (int y = border; y < chf.height - border; y++)
    for (int x = border; x < chf.width - border; x++)
    {
        const rcCompactCell& c = chf.cells[x + y * chf.width];
        for (unsigned int i = c.index, end = c.index + (unsigned int)c.count; i < end; i++)
        {
            if (chf.areas[i] == RC_NULL_AREA)
                continue;
            const unsigned short reg = chf.spans[i].reg;
            if (regionsOnly && (reg == 0 || (reg & RC_BORDER_REG)))
                continue;
            out.push_back(vec3(
                chf.bmin[0] + (x + 0.5f) * chf.cs,
                chf.bmin[1] + chf.spans[i].y * chf.ch + 0.03f,
                chf.bmin[2] + (y + 0.5f) * chf.cs));
        }
    }
}
} // namespace

bool NavMesh::detail::buildTierTile(ivec2 coord, const MeshTier::MeshTier& tier, const TileGeometry& geo, u8*& data, int& size,
                                    DebugBuildStages* dbg)
{
    data = nullptr;
    size = 0;

    const rcConfig cfg = makeTileConfig(tier, coord, geo);
    rcContext* ctx = &workerCtx;
    RecastScratch rc;

    const float* verts = geo.verts.data();
    const int* tris = geo.tris.data();
    const int nv = geo.vertCount();
    const int nt = geo.triCount();

    rc.hf = rcAllocHeightfield();
    if (!rc.hf || !rcCreateHeightfield(ctx, *rc.hf, cfg.width, cfg.height, cfg.bmin, cfg.bmax, cfg.cs, cfg.ch))
        return false;

    workerAreas.assign(nt, 0);
    rcMarkWalkableTriangles(ctx, cfg.walkableSlopeAngle, verts, nv, tris, nt, workerAreas.data());

    if (!rcRasterizeTriangles(ctx, verts, nv, tris, workerAreas.data(), nt, *rc.hf, cfg.walkableClimb))
        return false;

    if (dbg) recordHeightfield(*rc.hf, cfg.borderSize, dbg->rasterized);

    rcFilterLowHangingWalkableObstacles(ctx, cfg.walkableClimb, *rc.hf);
    rcFilterLedgeSpans(ctx, cfg.walkableHeight, cfg.walkableClimb, *rc.hf);
    rcFilterWalkableLowHeightSpans(ctx, cfg.walkableHeight, *rc.hf);

    if (dbg) recordHeightfield(*rc.hf, cfg.borderSize, dbg->filtered);

    rc.chf = rcAllocCompactHeightfield();
    if (!rc.chf || !rcBuildCompactHeightfield(ctx, cfg.walkableHeight, cfg.walkableClimb, *rc.hf, *rc.chf))
        return false;

    rcFreeHeightField(rc.hf);
    rc.hf = nullptr;

    if (!rcErodeWalkableArea(ctx, cfg.walkableRadius, *rc.chf))
        return false;

    if (dbg) recordCompact(*rc.chf, cfg.borderSize, false, dbg->eroded);

    if (!rcBuildLayerRegions(ctx, *rc.chf, cfg.borderSize, cfg.minRegionArea))
        return false;

    if (dbg) recordCompact(*rc.chf, cfg.borderSize, true, dbg->regions);

    rc.cset = rcAllocContourSet();
    if (!rc.cset || !rcBuildContours(ctx, *rc.chf, cfg.maxSimplificationError, cfg.maxEdgeLen, *rc.cset))
        return false;

    if (rc.cset->nconts == 0)
        return true;  // nothing walkable

    rc.pmesh = rcAllocPolyMesh();
    if (!rc.pmesh || !rcBuildPolyMesh(ctx, *rc.cset, cfg.maxVertsPerPoly, *rc.pmesh))
        return false;

    rc.dmesh = rcAllocPolyMeshDetail();
    if (!rc.dmesh || !rcBuildPolyMeshDetail(ctx, *rc.pmesh, *rc.chf, cfg.detailSampleDist, cfg.detailSampleMaxError, *rc.dmesh))
        return false;

    if (rc.pmesh->nverts == 0 || rc.pmesh->npolys == 0)
        return true;  // nothing walkable

    if (dbg)
        for (int i = 0; i < rc.pmesh->nverts; i++)
        {
            const unsigned short* v = &rc.pmesh->verts[i * 3];
            dbg->polys.push_back(vec3(
                rc.pmesh->bmin[0] + v[0] * rc.pmesh->cs,
                rc.pmesh->bmin[1] + v[1] * rc.pmesh->ch + 0.03f,
                rc.pmesh->bmin[2] + v[2] * rc.pmesh->cs));
        }

    if (rc.pmesh->nverts >= 0xffff)
        return false;

    for (int i = 0; i < rc.pmesh->npolys; ++i)
        rc.pmesh->flags[i] = (rc.pmesh->areas[i] == RC_WALKABLE_AREA) ? 1 : 0;

    dtNavMeshCreateParams params{};
    params.verts = rc.pmesh->verts;
    params.vertCount = rc.pmesh->nverts;
    params.polys = rc.pmesh->polys;
    params.polyAreas = rc.pmesh->areas;
    params.polyFlags = rc.pmesh->flags;
    params.polyCount = rc.pmesh->npolys;
    params.nvp = rc.pmesh->nvp;
    params.detailMeshes = rc.dmesh->meshes;
    params.detailVerts = rc.dmesh->verts;
    params.detailVertsCount = rc.dmesh->nverts;
    params.detailTris = rc.dmesh->tris;
    params.detailTriCount = rc.dmesh->ntris;
    params.walkableHeight = tier.height;
    params.walkableRadius = tier.radius;
    params.walkableClimb = tier.maxClimb;
    params.tileX = coord.x;
    params.tileY = coord.y;
    params.tileLayer = 0;
    rcVcopy(params.bmin, rc.pmesh->bmin);
    rcVcopy(params.bmax, rc.pmesh->bmax);
    params.cs = cfg.cs;
    params.ch = cfg.ch;
    params.buildBvTree = true;

    if (!dtCreateNavMeshData(&params, &data, &size))
    {
        data = nullptr;
        size = 0;
        return false;
    }
    return true;
}

NavMesh::detail::NavMeshData NavMesh::detail::createMeshData(const MeshTier::MeshTier& tier)
{
    dtNavMeshParams params{};
    params.orig[0] = kWorldOrigin.x;
    params.orig[1] = kWorldOrigin.y;
    params.orig[2] = kWorldOrigin.z;
    params.tileWidth = kTileWorldSize;
    params.tileHeight = kTileWorldSize;
    params.maxTiles = worldTileLength * worldTileLength;
    params.maxPolys = 1 << DT_POLY_BITS;

    NavMeshData mesh{dtAllocNavMesh(), dtAllocNavMeshQuery(), tier.id};

    if (!mesh.navMesh || dtStatusFailed(mesh.navMesh->init(&params)))
    {
        ERROR_MESSAGE("could not init navmesh for tier ", tier.id);
    }
    else if (!mesh.navMeshQuery || dtStatusFailed(mesh.navMeshQuery->init(mesh.navMesh, 2048)))
    {
        ERROR_MESSAGE("could not init navmesh query for tier ", tier.id);
    }

    return mesh;
}

void NavMesh::Init()
{
    assert(!worker.joinable());

    worker = std::jthread(&workerLoop);
}

void VulpineNav::NavMesh::GetDebugGeometry(vec3 center, int tileRadius, int slot, DebugGeometry& out)
{
    out.clear();

    const dtNavMesh* mesh = GetNavMesh(slot);
    if (!mesh)
        return;

    const vec3 lift(0.0f, 0.05f, 0.0f);
    auto v3 = [&](const float* p) { return vec3(p[0], p[1], p[2]) + lift; };

    const ivec2 c = worldToTileCoord(center);
    constexpr int kMaxLayers = 8;

    for (int ty = c.y - tileRadius; ty <= c.y + tileRadius; ty++)
    for (int tx = c.x - tileRadius; tx <= c.x + tileRadius; tx++)
    {
        if (tx < 0 || ty < 0 || tx >= (int)worldTileLength || ty >= (int)worldTileLength)
            continue;

        const dtMeshTile* tiles[kMaxLayers];
        const int tileCount = mesh->getTilesAt(tx, ty, tiles, kMaxLayers);

        for (int t = 0; t < tileCount; t++)
        {
            const dtMeshTile* tile = tiles[t];
            if (!tile->header)
                continue;

            for (int i = 0; i < tile->header->polyCount; i++)
            {
                const dtPoly* p = &tile->polys[i];
                if (p->getType() == DT_POLYTYPE_OFFMESH_CONNECTION)
                    continue;

                const dtPolyDetail* pd = &tile->detailMeshes[i];
                for (int j = 0; j < pd->triCount; j++)
                {
                    const unsigned char* tri = &tile->detailTris[(pd->triBase + j) * 4];
                    for (int k = 0; k < 3; k++)
                    {
                        const float* v = tri[k] < p->vertCount
                            ? &tile->verts[p->verts[tri[k]] * 3]
                            : &tile->detailVerts[(pd->vertBase + tri[k] - p->vertCount) * 3];
                        out.triangles.push_back(v3(v));
                    }
                }

                const int nv = p->vertCount;
                for (int j = 0; j < nv; j++)
                {
                    const unsigned short nei = p->neis[j];
                    const bool tileBorder = (nei & DT_EXT_LINK) != 0;

                    if (nei != 0 && !tileBorder)
                        continue;

                    const DebugEdge edge{
                        v3(&tile->verts[p->verts[j] * 3]),
                        v3(&tile->verts[p->verts[(j + 1) % nv] * 3])};

                    if (!tileBorder)
                    {
                        out.walls.push_back(edge);
                        continue;
                    }

                    bool linked = false;
                    for (unsigned int k = p->firstLink; k != DT_NULL_LINK; k = tile->links[k].next)
                    {
                        if (tile->links[k].edge == j)
                        {
                            linked = true;
                            break;
                        }
                    }
                    if (!linked)
                        out.unlinked.push_back(edge);
                }
            }
        }
    }
}

void VulpineNav::NavMesh::GetDebugInputGeometry(vec3 center, int slot, DebugInputGeometry& out)
{
    out.clear();

    if (!ValidSlot(slot))
        return;

    const MeshTier::MeshTier tier = tierRegistry.tier(navMeshes[slot].key);
    const float walkableThr = cosf(radians(tier.maxSlopeDeg));

    static TileGeometry geo;
    gatherTileGeometry(worldToTileCoord(center), 0.0f, geo);

    const vec3 lift(0.0f, 0.02f, 0.0f);

    for (int t = 0; t < geo.triCount(); t++)
    {
        const vec3 v0 = *(const vec3*)&geo.verts[geo.tris[t * 3 + 0] * 3];
        const vec3 v1 = *(const vec3*)&geo.verts[geo.tris[t * 3 + 1] * 3];
        const vec3 v2 = *(const vec3*)&geo.verts[geo.tris[t * 3 + 2] * 3];

        const vec3 n = cross(v1 - v0, v2 - v0);
        const float len = length(n);
        if (len < 1e-8f)
            continue;

        const float ny = n.y / len;

        std::vector<vec3>& bucket =
              ny > walkableThr  ? out.walkable
            : ny < -walkableThr ? out.flipped
            :                     out.steep;

        bucket.push_back(v0 + lift);
        bucket.push_back(v1 + lift);
        bucket.push_back(v2 + lift);
    }
}

void VulpineNav::NavMesh::GetDebugBuildStages(vec3 center, int slot, DebugBuildStages& out)
{
    out.clear();
    out.cellSize = kCellSize;

    if (!ValidSlot(slot))
        return;

    const MeshTier::MeshTier tier = tierRegistry.tier(navMeshes[slot].key);
    const ivec2 coord = worldToTileCoord(center);

    static TileGeometry geo;
    gatherTileGeometry(coord, borderCells(tier) * kCellSize, geo);
    if (geo.triCount() == 0)
        return;

    u8* data = nullptr;
    int size = 0;
    out.buildSucceeded = buildTierTile(coord, tier, geo, data, size, &out);
    if (data)
        dtFree(data);
}

void VulpineNav::NavMesh::DebugDrawTileInstanced(float y,
                        float duration,
                        bool drawUnloaded,
                        bool drawDiagonal,
                        ModelState3D state)
{
    constexpr vec3 kUnloadedColor   = "#595959"_rgb;
    constexpr size_t kCountMax = 50;

    static bool initialized = false;
    static std::shared_ptr<ObjectGroup> model = Loader<ObjectGroup>::get("Cube Helper").copy();

    if (!initialized)
    {
        globals.getScene()->add(model);
        initialized = true;

        auto m = model->getInstances()[0].originalModel;

        for (int ty = 0; ty < (int)worldTileLength; ++ty)
            for (int tx = 0; tx < (int)worldTileLength; ++tx)
                    m->createInstance();
    }

    auto m = model->getInstances()[0].originalModel;

    for (int ty = 0; ty < (int)worldTileLength; ++ty)
    {
        for (int tx = 0; tx < (int)worldTileLength; ++tx)
        {
            const size_t idx = idxFromTileCoord(tx, ty);
            const TileState st = grid.state[idx];

            vec3 color;
            if (st == TileState::Unloaded)
            {
                color = kUnloadedColor;
            }
            else 
            {
                color = hsv2rgb({0.6 - 0.5 * ((float)grid.nearCount[idx] / kCountMax), 0.8, 1.0});
            }

            auto& inst = m->getInstances()[idx];
            inst.setPosition(VulpineNav::tileToWorldPos({tx, ty}) + vec3(0, 128, 0)).scaleScalar(VulpineNav::kTileWorldSize);
            inst.userData = uvec4(uvec3(color*256.f), 0.f);
            inst.update();
        }
    }
}

} // namespace VulpineNav
