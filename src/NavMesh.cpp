#include "AssetManager.hpp"
#include "Globals.hpp"
#include "Graphics/ObjectGroup.hpp"
#include "MathsUtils.hpp"
#include "Navigation/NavMesh.hpp"
#include "ECS/ComponentTypeNavigation.hpp"

#include "JoltIntegration/PhysicsCommons.hpp"

#ifndef DT_VIRTUAL_QUERYFILTER
#error "DT_VIRTUAL_QUERYFILTER must be defined."
#endif
#ifndef DT_POLYREF64
#error "DT_POLYREF64 must be defined."
#endif

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

namespace NavMesh::NavMeshInternals
{
std::thread worker = std::thread();
std::mutex requestMutex = std::mutex();
std::condition_variable cv = std::condition_variable();
bool worker_quit = false;
std::priority_queue<NavMesh::LoadRequest, std::vector<NavMesh::LoadRequest>, std::greater<NavMesh::LoadRequest>> buildQueue = std::priority_queue<NavMesh::LoadRequest, std::vector<NavMesh::LoadRequest>, std::greater<NavMesh::LoadRequest>>();
std::vector<NavMesh::BuildResult> results = std::vector<NavMesh::BuildResult>();
std::vector<MeshTier::MeshTier> buildTiers = std::vector<MeshTier::MeshTier>();
}

ivec2 NavMesh::NavMeshInternals::rowSpan(const ivec2& cell, int radius, int y)
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

void NavMesh::NavMeshInternals::addNear(int y, ivec2 span, ivec2 center, i64 baseKey)
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

void NavMesh::NavMeshInternals::removeFar(int y, ivec2 span)
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

void NavMesh::NavMeshInternals::updateRow(int y, ivec2 prevTile, ivec2 newTile, ivec2 prevUnload, ivec2 newUnload, i64 baseKey)
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

void NavMesh::NavMeshInternals::releaseTile(int x, int y)
{
    size_t i = idxFromTileCoord(x, y);
    if (grid.state[i] != Loaded) // tile already unloaded
        return;

    for (NavMeshData& mesh : navMeshes)
        removeMeshTile(mesh, x, y);

    grid.state[i] = Unloaded;
}

void NavMesh::NavMeshInternals::addMeshTile(NavMeshData& mesh, int x, int y, u8* data, int size)
{
    if (mesh.navMesh->getTileAt(x, y, 0) ||
        dtStatusFailed(mesh.navMesh->addTile(data, size, DT_TILE_FREE_DATA, 0, nullptr)))
    {
        dtFree(data);
    }
}

void NavMesh::NavMeshInternals::addAll(BuildResult& r)
{
    const ivec2 c = coordFromIdx(r.req.idx);
    for (BuiltTile& t : r.tiles)
        addMeshTile(navMeshes[t.slot], c.x, c.y, t.data, t.size);
    r.tiles.clear();
}

void NavMesh::NavMeshInternals::integrate(BuildResult& r)
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
    const u64 missing = allSlots(navMeshes.size()) & ~r.builtMask;
    if (missing)
        newRequests.push_back({r.req.key, i, missing});
}


NavMesh::BuildResult NavMesh::NavMeshInternals::buildRequest(const LoadRequest& req, u64 mask, const std::vector<MeshTier::MeshTier>& tiers)
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

void NavMesh::NavMeshInternals::workerLoop()
{
    currentThreadID = NAVMESH_BUILD_THREAD_ID;

    std::vector<MeshTier::MeshTier> tiers; // snapshot of the tiers

    while (true)
    {
        LoadRequest req;
        u64 mask;

        // block for mutex lock scope
        {
            std::unique_lock<std::mutex> lock(requestMutex);
            while (!worker_quit && buildQueue.empty())
                cv.wait(lock);
            if (worker_quit)
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

int NavMesh::NavMeshInternals::addTier(const MeshTier::MeshTier& tier)
{
    assert(navMeshes.size() < kMaxTiers);

    const u8 slot = (u8)navMeshes.size();
    navMeshes.push_back(createMeshData(tier));
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

void NavMesh::shutdown()
{
    {
        std::lock_guard<std::mutex> lock(requestMutex);
        worker_quit = true;
    }
    cv.notify_one();
    if (worker.joinable())
        worker.join();

    while (!buildQueue.empty()) buildQueue.pop();
    for (BuildResult& r : results) freeAll(r);
    results.clear();
    for (BuildResult& r : resultsScratch) freeAll(r);
    resultsScratch.clear();
}

int NavMesh::ResolveAgent(const MeshTier::AgentShape& shape)
{
    const MeshTier::TierId id = tierRegistry.resolve(shape);

    for (size_t s = 0; s < navMeshes.size(); s++)
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
        resultsScratch.swap(results);
    }
    if (!newRequests.empty())
        cv.notify_one();
    newRequests.clear();

    for (BuildResult& r : resultsScratch)
        integrate(r);
    resultsScratch.clear();

    {
        std::lock_guard<std::mutex> lock(requestMutex);
        ManageNavGarbage();
    }
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

void NavMesh::NavMeshInternals::appendShapeTriangles(const JPH::TransformedShape& shape, const JPH::AABox& box, TileGeometry& geo)
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

void NavMesh::NavMeshInternals::gatherTileGeometry(ivec2 coord, float padding, TileGeometry& geo)
{
    geo.clear();

    Vvec3 bmin, bmax;
    tileWorldBounds(coord, padding, bmin, bmax);
    const JPH::AABox box = JPH::AABox(bmin, bmax);

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

rcConfig NavMesh::NavMeshInternals::makeTileConfig(const MeshTier::MeshTier& tier, ivec2 coord, const TileGeometry& geo)
{
    rcConfig cfg{};
    cfg.cs = kCellSize;
    cfg.ch = kCellHeight;
    cfg.walkableSlopeAngle = tier.maxSlopeDeg;
    cfg.walkableHeight = (int)ceilf(tier.height / cfg.ch);
    cfg.walkableClimb = (int)floorf(tier.maxClimb / cfg.ch);
    cfg.walkableRadius = (int)ceilf(tier.radius / cfg.cs);
    cfg.maxEdgeLen = (int)(12.0f / cfg.cs);
    cfg.maxSimplificationError = 1.3f;
    cfg.minRegionArea = rcSqr(8);
    cfg.mergeRegionArea = rcSqr(20);
    cfg.maxVertsPerPoly = DT_VERTS_PER_POLYGON;
    cfg.detailSampleDist = cfg.cs * 6;
    cfg.detailSampleMaxError = cfg.ch;

    cfg.tileSize = kTileCells;
    cfg.borderSize = borderCells(tier);
    cfg.width = cfg.tileSize + cfg.borderSize * 2;
    cfg.height = cfg.width;

    tileWorldBounds(coord, cfg.borderSize * cfg.cs, *(vec3*)cfg.bmin, *(vec3*)cfg.bmax);
    cfg.bmin[1] = geo.minY;
    cfg.bmax[1] = geo.maxY;
    return cfg;
}

bool NavMesh::NavMeshInternals::buildTierTile(ivec2 coord, const MeshTier::MeshTier& tier, const TileGeometry& geo, u8*& data, int& size)
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

    rcFilterLowHangingWalkableObstacles(ctx, cfg.walkableClimb, *rc.hf);
    rcFilterLedgeSpans(ctx, cfg.walkableHeight, cfg.walkableClimb, *rc.hf);
    rcFilterWalkableLowHeightSpans(ctx, cfg.walkableHeight, *rc.hf);

    rc.chf = rcAllocCompactHeightfield();
    if (!rc.chf || !rcBuildCompactHeightfield(ctx, cfg.walkableHeight, cfg.walkableClimb, *rc.hf, *rc.chf))
        return false;

    rcFreeHeightField(rc.hf);
    rc.hf = nullptr;

    if (!rcErodeWalkableArea(ctx, cfg.walkableRadius, *rc.chf))
        return false;

    if (!rcBuildLayerRegions(ctx, *rc.chf, cfg.borderSize, cfg.minRegionArea))
        return false;

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

NavMesh::NavMeshData NavMesh::NavMeshInternals::createMeshData(const MeshTier::MeshTier& tier)
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

void NavMesh::init()
{
    {
        std::lock_guard<std::mutex> lock(requestMutex);
        worker_quit = false;
    }
    worker = std::thread(&NavMesh::workerLoop);
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
