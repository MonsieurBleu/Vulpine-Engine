#pragma once
#include "CompilingOptions.hpp"
#include "Matrix.hpp"
#include "Utils.hpp"

#include <array>
#include <atomic>
#include <cassert>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>
#include <vector>

#include "JoltIntegration/JoltIntegration.hpp"

#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/Shape/HeightFieldShape.h>
#include <Jolt/Geometry/OrientedBox.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
using namespace glm;

// does the static navmesh generation by querying the physics world and using recast to build it

#define NAVMESH_BUILD_THREAD_ID THREAD_ID_NAVMESH_BUILD

#include <recastnavigation/Recast.h>
#include <recastnavigation/DetourNavMesh.h>
#include <recastnavigation/DetourNavMeshQuery.h>
#include <recastnavigation/DetourNavMeshBuilder.h>


namespace VulpineNav
{
constexpr size_t worldSize = 4096;

static constexpr float kCellSize = 0.3f; // meters per voxel, kind of acts as the resolution of the navmesh, in terms of accuracy and performance
static constexpr float kCellHeight = 0.2f;
static constexpr int   kTileCells = 64; // tile edge in voxels

static constexpr float kTileWorldSize = kTileCells * kCellSize; // 0.3 * 64 = 19.2m tile side length

static constexpr float kWorldMinY = 0;
static constexpr float kWorldMaxY = 512; // TODO: confirm this

static inline const vec3 kWorldOrigin = vec3(-(float)worldSize * 0.5f, 0.0f, -(float)worldSize * 0.5f);

constexpr size_t worldTileLength = worldSize / kTileWorldSize;

#ifndef DT_POLYREF64
#error "NavMesh expects Detour built with DT_POLYREF64"
#endif



inline size_t idxFromTileCoord(int x, int y)
{
    return x + y * worldTileLength;
}

inline size_t idxFromTileCoord(ivec2 tileCoord)
{
    return idxFromTileCoord(tileCoord.x, tileCoord.y);
}

inline ivec2 coordFromIdx(size_t idx)
{
    return {(int)idx % worldTileLength, (int)(idx / worldTileLength)};
}

inline vec3 tileToWorldPos(const ivec2& tilePos)
{
    return vec3(
        (tilePos.x + 0.5f) * kTileWorldSize + kWorldOrigin.x,
        kWorldOrigin.y,
        (tilePos.y + 0.5f) * kTileWorldSize + kWorldOrigin.z
    );
}

// writes the position to tileX & tileY
inline ivec2 worldToTileCoord(const vec3& worldPos)
{
    return ivec2(
        floor((worldPos.x - kWorldOrigin.x) / kTileWorldSize),
        floor((worldPos.z - kWorldOrigin.z) / kTileWorldSize));
}

inline size_t idxFromWorldPos(const vec3& worldPos)
{
    return idxFromTileCoord(worldToTileCoord(worldPos));
}

// the "tier" of the mesh, determines stuff like radius, height, etc 
// to determine how to build the mesh
namespace MeshTier
{

using TierId = u16;
inline constexpr TierId kInvalidTier = TierId(-1);

struct MeshTier {
    TierId id          = 0;
    float   radius      = 0.6f;
    float   height      = 2.0f;
    float   maxClimb    = 0.4f;
    float   maxSlopeDeg = 45.0f;

    bool operator==(const MeshTier& o) const {
        return radius == o.radius && height == o.height
            && maxClimb == o.maxClimb && maxSlopeDeg == o.maxSlopeDeg;
    }
};

// raw shape of the agent, this will be automatically bucketed in it's navmesh tier
struct AgentShape {
    float radius      = 0.6f;
    float height      = 2.0f;
    float maxClimb    = 0.4f;
    float maxSlopeDeg = 45.0f;
};

// rules for bucketing and minting the tiers
namespace TierBucketingPolicy {
    // Snap-to-grid step for newly minted tiers
    inline constexpr float radiusStep   = 0.15f;
    inline constexpr float heightStep   = 0.5f;
    inline constexpr float climbStep    = 0.1f;
    inline constexpr float slopeStepDeg = 5.0f;

    // max slack an existing tier may carry before it's rejected in favor of
    // minting a new one 
    inline constexpr float maxRadiusSlack   = 0.15f;
    inline constexpr float maxHeightSlack   = 0.5f;
    inline constexpr float maxClimbSlack    = 0.1f;
    inline constexpr float maxSlopeSlackDeg = 5.0f;
};

inline bool covers(const MeshTier& tier, const AgentShape& shape) {
    return tier.radius      >= shape.radius
        && tier.radius      <= shape.radius      + TierBucketingPolicy::maxRadiusSlack
        && tier.height      >= shape.height
        && tier.height      <= shape.height      + TierBucketingPolicy::maxHeightSlack
        && tier.maxClimb    <= shape.maxClimb
        && tier.maxClimb    >= shape.maxClimb    - TierBucketingPolicy::maxClimbSlack
        && tier.maxSlopeDeg <= shape.maxSlopeDeg
        && tier.maxSlopeDeg >= shape.maxSlopeDeg - TierBucketingPolicy::maxSlopeSlackDeg;
}

inline MeshTier quantize(const AgentShape& shape, TierId id) {
    auto up   = [](float v, float step) { return ceil(v / step) * step; };
    auto down = [](float v, float step) { return floor(v / step) * step; };
    MeshTier t;
    t.id          = id;
    t.radius      = up(shape.radius, TierBucketingPolicy::radiusStep);
    t.height      = up(shape.height, TierBucketingPolicy::heightStep);
    t.maxClimb    = down(shape.maxClimb, TierBucketingPolicy::climbStep);
    t.maxSlopeDeg = down(shape.maxSlopeDeg, TierBucketingPolicy::slopeStepDeg);
    return t;
}

class TierRegistry {
public:
    TierId resolve(const AgentShape& shape);

    MeshTier tier(TierId id) const;
    std::vector<MeshTier> tiers() const;
    size_t tierCount() const;

    bool valid(TierId id) const;
private:
    struct GridKey {
        u16 radiusSteps, heightSteps, climbSteps, slopeSteps;
        bool operator==(const GridKey& o) const {
            return radiusSteps == o.radiusSteps && heightSteps == o.heightSteps
                && climbSteps == o.climbSteps && slopeSteps == o.slopeSteps;
        }
    };

    struct GridKeyHash {
        size_t operator()(const GridKey& k) const {
            uint64_t packed = 
                  (u64(k.radiusSteps) << 48)
                | (u64(k.heightSteps) << 32)
                | (u64(k.climbSteps)  << 16)
                |  u64(k.slopeSteps);

            // splitmix64 for avalanche in unordered_map
            packed ^= packed >> 33;
            packed *= 0xff51afd7ed558ccdull;
            packed ^= packed >> 33;
            return size_t(packed);
        }
    };

    GridKey gridKeyOf(const AgentShape& shape) const;

    mutable std::mutex m_mutex;
    std::vector<MeshTier> m_tiers;
    std::unordered_map<GridKey, TierId, GridKeyHash> m_tierIndexer;
};
}




namespace NavMesh
{
static constexpr int kLoadRadius = 6;
static constexpr int kUnloadRadius = 8;
namespace NavMeshInternals
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

    ~RecastScratch()
    {
        rcFreeHeightField(hf);
        rcFreeCompactHeightfield(chf);
        rcFreeContourSet(cset);
        rcFreePolyMesh(pmesh);
        rcFreePolyMeshDetail(dmesh);
    }
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
inline TileGrid grid;
inline std::vector<NavMeshData> navMeshes; // index == slot
inline MeshTier::TierRegistry tierRegistry;
inline u64 frame = 0;
inline std::vector<LoadRequest> newRequests;
inline std::vector<BuildResult> resultsScratch;

// shared with worker
extern std::thread worker;
extern std::mutex requestMutex;
extern std::condition_variable cv;
extern bool worker_quit;
extern std::priority_queue<LoadRequest, std::vector<LoadRequest>, std::greater<LoadRequest>> buildQueue;
extern std::vector<BuildResult> results;
extern std::vector<MeshTier::MeshTier> buildTiers;

// worker state
inline TileGeometry workerGeo;
inline std::vector<u8> workerAreas;
inline rcContext workerCtx{false};

NavMeshData createMeshData(const MeshTier::MeshTier& tier);

// helpers
static inline u64 allSlots(size_t count) 
{ 
    return count >= 64 ? ~0ull : (1ull << count) - 1; 
}

static inline void tileWorldBounds(const ivec2& coord, float padding, vec3& bmin, vec3& bmax)
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
static inline int borderCells(const MeshTier::MeshTier& tier)
{
    return (int)ceilf(tier.radius / kCellSize) + 3;
}

static inline float gatherPadding(u64 mask, const std::vector<MeshTier::MeshTier>& tiers)
{
    int cells = 0;
    for (u32 s = 0; s < tiers.size(); s++)
    {
        if (mask & (1ull << s))
            cells = max(cells, borderCells(tiers[s]));
    }
    return cells * kCellSize;
}


inline i64 requestKey(u8 priority)
{ 
    return (i64)frame - (i64)priority * kFramesPerPriority; 
}

static inline ivec2 rowRange(int yMin, int yMax) 
{ 
    return {max(yMin, 0), min(yMax, kMaxIdx)}; 
}

static inline ivec2 clampSpan(const ivec2& s)
{ 
    return {max(s.x, 0), min(s.y, kMaxIdx)}; 
}

static inline ivec2 rowSpan(const ivec2& cell, int radius, int y);

static inline SpanPair subtractSpan(const ivec2& a, const ivec2& b)
{
    return {{a.x, min(a.y, b.x - 1)}, {max(a.x, b.y + 1), a.y}};
}

// base tile update methods
void addNear(int y, ivec2 span, ivec2 center, i64 baseKey);
void removeFar(int y, ivec2 span);
void updateRow(int y, ivec2 prevTile, ivec2 newTile, ivec2 prevUnload, ivec2 newUnload, i64 baseKey);

inline void removeNear(int y, ivec2 span)
{
    span = clampSpan(span);
    u16* nearRow = &grid.nearCount[(size_t)y * worldTileLength];

    for (int x = span.x; x <= span.y; ++x)
        --nearRow[x];
}

inline void addFar(int y, ivec2 span)
{
    span = clampSpan(span);
    u16* farRow = &grid.farCount[(size_t)y * worldTileLength];

    for (int x = span.x; x <= span.y; ++x)
        ++farRow[x];
}

inline void requestTile(size_t i, i64 key)
{
    if (grid.state[i] != Unloaded) // tile already loaded or pending 
        return;

    grid.state[i] = Pending;
    newRequests.push_back({key, i, 0});
}

void releaseTile(int x, int y);

void addMeshTile(NavMeshData& mesh, int x, int y, u8* data, int size);

static inline void removeMeshTile(NavMeshData& mesh, int x, int y)
{
    dtTileRef ref = mesh.navMesh->getTileRefAt(x, y, 0);
    if (ref)
        mesh.navMesh->removeTile(ref, nullptr, nullptr);
}

void addAll(BuildResult& r);

static inline void freeAll(BuildResult& r)
{
    for (BuiltTile& t : r.tiles)
        dtFree(t.data);
    r.tiles.clear();
}

void integrate(BuildResult& r);
BuildResult buildRequest(const LoadRequest& req, u64 mask, const std::vector<MeshTier::MeshTier>& tiers);
void workerLoop();
int addTier(const MeshTier::MeshTier& tier);

void appendShapeTriangles(const JPH::TransformedShape& shape, const JPH::AABox& box, TileGeometry& geo);

void gatherTileGeometry(ivec2 coord, float padding, TileGeometry& geo);

rcConfig makeTileConfig(const MeshTier::MeshTier& tier, ivec2 coord, const TileGeometry& geo);

bool buildTierTile(ivec2 coord, const MeshTier::MeshTier& tier, const TileGeometry& geo, u8*& data, int& size);


} // namespace NavMeshInternals

using namespace NavMeshInternals;

void init();

void shutdown();

int ResolveAgent(const MeshTier::AgentShape& shape);

inline bool ValidSlot(int slot)
{
    return slot >= 0 && (size_t)slot < navMeshes.size();
}

inline dtNavMesh* GetNavMesh(int slot)
{
    return ValidSlot(slot) ? navMeshes[slot].navMesh : nullptr;
}

inline dtNavMeshQuery* GetNavMeshQuery(int slot)
{
    return ValidSlot(slot) ? navMeshes[slot].navMeshQuery : nullptr;
}

inline int SlotFromTierId(MeshTier::TierId id)
{
    for (size_t s = 0; s < navMeshes.size(); s++)
        if (navMeshes[s].key == id)
            return (int)s;
    return -1;
}

inline dtNavMesh* GetNavMeshByTierId(MeshTier::TierId id)
{
    return GetNavMesh(SlotFromTierId(id));
}

inline dtNavMeshQuery* GetNavMeshQueryByTierId(MeshTier::TierId id)
{
    return GetNavMeshQuery(SlotFromTierId(id));
}

inline int FindAgentSlot(const MeshTier::AgentShape& shape)
{
    for (size_t s = 0; s < navMeshes.size(); s++)
    {
        if (MeshTier::covers(tierRegistry.tier(navMeshes[s].key), shape))
            return (int)s;
    }
    return -1;
}

inline dtNavMesh* GetNavMesh(const MeshTier::AgentShape& shape)
{
    return GetNavMesh(FindAgentSlot(shape));
}

inline dtNavMeshQuery* GetNavMeshQuery(const MeshTier::AgentShape& shape)
{
    return GetNavMeshQuery(FindAgentSlot(shape));
}

inline dtNavMesh* GetOrCreateNavMesh(const MeshTier::AgentShape& shape)
{
    return GetNavMesh(ResolveAgent(shape));
}

inline dtNavMeshQuery* GetOrCreateNavMeshQuery(const MeshTier::AgentShape& shape)
{
    return GetNavMeshQuery(ResolveAgent(shape));
}

void Tick();
void CreateLoader(vec3 pos, u8 priority);
void UpdateLoaderPosition(vec3 prevPos, vec3 newPos, u8 priority);
void UnloadLoader(vec3 pos);

template <typename TDraw>
void DebugDrawTiles(TDraw& draw,
                    float yMin = kWorldMinY,
                    float yMax = kWorldMaxY,
                    bool drawUnloaded = false)
{
    constexpr vec3 kPendingColor  = vec3(1.0f, 0.85f, 0.1f);
    constexpr vec3 kLoadedColor   = vec3(0.1f, 0.9f, 0.2f);
    constexpr vec3 kUnloadedColor = vec3(0.35f, 0.35f, 0.35f);

    for (int y = 0; y < (int)worldTileLength; ++y)
    {
        for (int x = 0; x < (int)worldTileLength; ++x)
        {
            const TileState st = grid.state[idxFromTileCoord(x, y)];

            vec3 color;
            switch (st)
            {
            case Pending:  color = kPendingColor;  break;
            case Loaded:   color = kLoadedColor;   break;
            case Unloaded:
                if (!drawUnloaded) continue;
                color = kUnloadedColor;
                break;
            default: continue;
            }

            vec3 bmin, bmax;
            tileWorldBounds({x, y}, 0.0f, bmin, bmax);
            bmin.y = yMin;
            bmax.y = yMax;

            draw.drawBox(bmin, bmax, 0.0f, ModelState3D(), color);
        }
    }
}


// also much more optimised for large number of tiles since they're all bundled into a single mesh
template <typename TDraw>
void DebugDrawTileQuads(TDraw& draw,
                        float y,
                        float duration = 0.0f,
                        bool drawUnloaded = false,
                        bool drawDiagonal = true,
                        ModelState3D state = ModelState3D())
{
    constexpr vec3 kPendingColor    = "#ffd91a"_rgb;
    constexpr vec3 kLoadedNearColor = "#1ae633"_rgb;   // loaded, inside an agent's load radius
    constexpr vec3 kLoadedFarColor  = "#1a99f2"_rgb;  // loaded, only kept alive by the unload radius
    constexpr vec3 kUnloadedColor   = "#595959"_rgb;

    const size_t vertsPerQuad = drawDiagonal ? 6 : 12;

    static std::vector<vec3> pending, loadedNear, loadedFar, unloaded;

    pending.clear();
    loadedNear.clear();
    loadedFar.clear();
    unloaded.clear();

    auto pushQuad = [&](std::vector<vec3>& out, const vec3& bmin, const vec3& bmax)
    {
        const vec3 a(bmin.x, y, bmin.z);
        const vec3 b(bmax.x, y, bmin.z);
        const vec3 c(bmax.x, y, bmax.z);
        const vec3 d(bmin.x, y, bmax.z);

        if (drawDiagonal)
        {
            out.push_back(a); out.push_back(b); out.push_back(c);
            out.push_back(a); out.push_back(c); out.push_back(d);
        }
        else
        {
            out.push_back(a); out.push_back(b); out.push_back(b);
            out.push_back(b); out.push_back(c); out.push_back(c);
            out.push_back(c); out.push_back(d); out.push_back(d);
            out.push_back(d); out.push_back(a); out.push_back(a);
        }
    };

    for (int ty = 0; ty < (int)worldTileLength; ++ty)
    {
        for (int tx = 0; tx < (int)worldTileLength; ++tx)
        {
            const size_t idx = idxFromTileCoord(tx, ty);
            const TileState st = grid.state[idx];

            std::vector<vec3>* bucket = nullptr;
            switch (st)
            {
            case Pending:  bucket = &pending;  break;
            case Loaded:
                bucket = grid.nearCount[idx] > 0 ? &loadedNear : &loadedFar;
                break;
            case Unloaded: bucket = drawUnloaded ? &unloaded : nullptr; break;
            default: break;
            }
            if (!bucket)
                continue;

            vec3 bmin, bmax;
            tileWorldBounds({tx, ty}, 0.0f, bmin, bmax);

            if (bucket->capacity() == 0)
                bucket->reserve(vertsPerQuad * 64);

            pushQuad(*bucket, bmin, bmax);
        }
    }

    auto flush = [&](std::vector<vec3>& verts, const vec3& color)
    {
        if (verts.empty())
            return;
        // every 3 verts = 1 triangle
        draw.drawMesh(verts.data(), verts.size() / 3, duration, state, color);
    };

    flush(pending,    kPendingColor);
    flush(loadedNear, kLoadedNearColor);
    flush(loadedFar,  kLoadedFarColor);
    flush(unloaded,   kUnloadedColor);
}


void DebugDrawTileInstanced(
    float y,
    float duration = 0.0f,
    bool drawUnloaded = false,
    bool drawDiagonal = true,
    ModelState3D state = ModelState3D());

} // namespace NavMesh

} // namespace VulpineNav
