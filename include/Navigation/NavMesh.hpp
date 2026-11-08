#pragma once
#include "CompilingOptions.hpp"
#include "Matrix.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <unordered_map>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
using namespace glm;

// this module does the static navmesh generation by querying the physics world and using recast to build it

class dtNavMesh;
class dtNavMeshQuery;
class rcHeightfield;
class rcCompactHeightfield;
class rcContourSet;
class rcPolyMesh;
class rcPolyMeshDetail;
class rcContext;

namespace VulpineNav
{
constexpr size_t worldSize = 4096;

static constexpr float kCellSize = 0.1f; // meters per voxel, kind of acts as the resolution of the navmesh, in terms of accuracy and performance
static constexpr float kCellHeight = kCellSize / 2.0f;
static constexpr int   kTileCells = 192; // tile edge in voxels

static constexpr float kTileWorldSize = kTileCells * kCellSize; // 0.1 * 192 = 19.2m tile side length

static constexpr float kDetailMaxError = 0.2f;

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

// == polygon areas, flags and links ==
// areas set what traversal costs (see the NavServer query filter), flags who may use a polygon at all

enum NavArea : u8
{
    kAreaClimbLow  = 1,
    kAreaClimbHigh = 2, // High and Platform climbs
    kAreaDrop      = 3,
    kAreaSlide     = 4, // drops that land on a steep slope and slide down it
    kAreaGround    = 63, // RC_WALKABLE_AREA
};

enum NavFlags : u16
{
    kFlagWalk  = 1 << 0,
    kFlagClimb = 1 << 1,
    kFlagDrop  = 1 << 2,
};

enum class LinkKind : u8 { None, ClimbLow, ClimbHigh, ClimbPlatform, Drop, Slide };

inline constexpr float kDropMaxHeight = 2.5f;

// off-mesh link user ids: kind in bits 0-3, height difference in cm in bits 4-15,
// and the horizontal direction the link was probed in, as a 256 step angle, in bits 16-23
inline u32 PackLinkUserId(LinkKind kind, float height, vec3 facing)
{
    const float turns = atan2(facing.z, facing.x) / 6.2831853f;
    const u32 angle = u32((int)round(turns * 256.0f) & 0xff);
    const u32 cm = u32(clamp(height, 0.0f, 40.95f) * 100.0f + 0.5f) & 0xfff;
    return (u32(kind) & 0xf) | (cm << 4) | (angle << 16);
}
inline LinkKind LinkKindFromUserId(u32 id) { return LinkKind(id & 0xf); }
inline float LinkHeightFromUserId(u32 id) { return float((id >> 4) & 0xfff) / 100.0f; }
inline vec3 LinkFacingFromUserId(u32 id)
{
    const float a = float((id >> 16) & 0xff) / 256.0f * 6.2831853f;
    return vec3(cos(a), 0.0f, sin(a));
}

namespace NavMesh
{
static constexpr bool kGenerateClimbLinks = true;

// defaults
static constexpr int kLoadRadius = 6;
static constexpr int kUnloadRadius = 8;
static constexpr int kUnloadMargin = kUnloadRadius - kLoadRadius;




void Init();

void Shutdown();

int ResolveAgent(const MeshTier::AgentShape& shape);

bool ValidSlot(int slot);

dtNavMesh* GetNavMesh(int slot);

dtNavMeshQuery* GetNavMeshQuery(int slot);

int SlotFromTierId(MeshTier::TierId id);

inline dtNavMesh* GetNavMeshByTierId(MeshTier::TierId id)
{
    return GetNavMesh(SlotFromTierId(id));
}

inline dtNavMeshQuery* GetNavMeshQueryByTierId(MeshTier::TierId id)
{
    return GetNavMeshQuery(SlotFromTierId(id));
}

int FindAgentSlot(const MeshTier::AgentShape& shape);

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

// incremented when Tick adds or removes tiles
u64 GetVersion();

// sum of the per-tile versions in the square of tiles around pos, changes whenever one of them
// is added or removed. safe from any thread; read under QueryReadLock it matches what queries see
u64 GetRegionVersion(vec3 pos, int tileRadius = 1);

class QueryReadLock
{
public:
    QueryReadLock();
    ~QueryReadLock();
    QueryReadLock(const QueryReadLock&) = delete;
    QueryReadLock& operator=(const QueryReadLock&) = delete;
};

void CancelWriterWait();

bool IsTileWanted(vec3 pos);

// tiles within loadRadius of a loader get built, and are released past loadRadius + kUnloadMargin.
// a loader must use the same radius for its whole life
void CreateLoader(vec3 pos, u8 priority, int loadRadius = kLoadRadius);
void UpdateLoaderPosition(vec3 prevPos, vec3 newPos, u8 priority, int loadRadius = kLoadRadius);
void UnloadLoader(vec3 pos, int loadRadius = kLoadRadius);

// switches an off-mesh link off for every agent of that slot, until its tile is rebuilt. main thread
void DisableLink(int slot, u64 linkPoly);



// == navmesh geometry debug ==

struct DebugEdge
{
    vec3 start, end;
};

struct DebugLink
{
    vec3 start, end;
    u8 area;
};

struct DebugGeometry
{
    std::vector<vec3> triangles;
    std::vector<DebugEdge> walls;
    std::vector<DebugEdge> unlinked;
    std::vector<DebugLink> links;

    void clear() { triangles.clear(); walls.clear(); unlinked.clear(); links.clear(); }
};

void GetDebugGeometry(vec3 center, int tileRadius, int slot, DebugGeometry& out);

template <typename LineT, typename Drawer>
void DebugDrawGeometry(Drawer& draw, vec3 center, int tileRadius = 1, int slot = 0)
{
    static DebugGeometry geo;
    static std::vector<LineT> walls, unlinked;

    GetDebugGeometry(center, tileRadius, slot, geo);

    if (!geo.triangles.empty())
        draw.drawMesh(geo.triangles.data(), geo.triangles.size() / 3, 0.0f, {}, vec3(0.15f, 0.55f, 0.75f));

    walls.clear();
    for (const DebugEdge& e : geo.walls)
        walls.push_back(LineT{e.start, e.end});
    if (!walls.empty())
        draw.drawLines(walls, 0.0f, {}, vec3(1.0f, 0.85f, 0.1f));

    unlinked.clear();
    for (const DebugEdge& e : geo.unlinked)
        unlinked.push_back(LineT{e.start, e.end});
    if (!unlinked.empty())
        draw.drawLines(unlinked, 0.0f, {}, vec3(1.0f, 0.1f, 0.8f));

    // links as arcs: climbs low green / high orange, drops cyan, slides purple
    static std::vector<LineT> arcs[4];
    for (auto& a : arcs)
        a.clear();
    for (const DebugLink& l : geo.links)
    {
        const int bucket = l.area == kAreaClimbLow ? 0 : l.area == kAreaClimbHigh ? 1 : l.area == kAreaDrop ? 2 : 3;
        const float bump = 0.3f + 0.25f * glm::abs(l.end.y - l.start.y);
        constexpr int kSegments = 8;
        vec3 prev = l.start;
        for (int i = 1; i <= kSegments; i++)
        {
            const float t = (float)i / kSegments;
            const vec3 p = mix(l.start, l.end, t) + vec3(0, bump * 4.0f * t * (1.0f - t), 0);
            arcs[bucket].push_back(LineT{prev, p});
            prev = p;
        }
    }
    const vec3 arcColors[4] = {vec3(0.2f, 0.9f, 0.3f), vec3(1.0f, 0.5f, 0.1f), vec3(0.1f, 0.8f, 1.0f), vec3(0.7f, 0.3f, 1.0f)};
    for (int i = 0; i < 4; i++)
        if (!arcs[i].empty())
            draw.drawLines(arcs[i], 0.0f, {}, arcColors[i]);
}

// == build input debug ==

struct DebugInputGeometry
{
    std::vector<vec3> walkable;
    std::vector<vec3> steep;
    std::vector<vec3> flipped;

    void clear() { walkable.clear(); steep.clear(); flipped.clear(); }
};

void GetDebugInputGeometry(vec3 center, int slot, DebugInputGeometry& out);

template <typename Drawer>
void DebugDrawInputGeometry(Drawer& draw, vec3 center, int slot = 0)
{
    static DebugInputGeometry geo;
    GetDebugInputGeometry(center, slot, geo);

    if (!geo.walkable.empty())
        draw.drawMesh(geo.walkable.data(), geo.walkable.size() / 3, 0.0f, {}, vec3(0.2f, 0.9f, 0.3f));
    if (!geo.steep.empty())
        draw.drawMesh(geo.steep.data(), geo.steep.size() / 3, 0.0f, {}, vec3(0.45f, 0.45f, 0.45f));
    if (!geo.flipped.empty())
        draw.drawMesh(geo.flipped.data(), geo.flipped.size() / 3, 0.0f, {}, vec3(1.0f, 0.1f, 0.8f));
}

// == build pipeline debug ==

struct DebugBuildStages
{
    std::vector<vec3> rasterized;  // 0: after rasterization (slope test only)
    std::vector<vec3> filtered;    // 1: after low hanging / ledge / low height filters
    std::vector<vec3> eroded;      // 2: after erosion by the agent radius
    std::vector<vec3> regions;     // 3: after region building (small regions culled)
    std::vector<vec3> polys;       // 4: poly mesh vertices of the final tile

    float cellSize = 0.0f;
    bool buildSucceeded = false;

    void clear()
    {
        rasterized.clear(); filtered.clear(); eroded.clear(); regions.clear(); polys.clear();
        buildSucceeded = false;
    }
};


void GetDebugBuildStages(vec3 center, int slot, DebugBuildStages& out);

template <typename LineT, typename Drawer>
void DebugDrawBuildStage(Drawer& draw, vec3 center, int stage, int slot = 0, int rebuildEvery = 120)
{
    static DebugBuildStages stages;
    static std::vector<LineT> lines;
    static ivec2 lastTile(-1);
    static int lastSlot = -1;
    static int calls = 0;

    const ivec2 tile = worldToTileCoord(center);
    if (tile != lastTile || slot != lastSlot || ++calls >= rebuildEvery)
    {
        GetDebugBuildStages(center, slot, stages);
        lastTile = tile;
        lastSlot = slot;
        calls = 0;
    }

    const std::vector<vec3>* sources[5] = {
        &stages.rasterized, &stages.filtered, &stages.eroded, &stages.regions, &stages.polys};
    const vec3 colors[5] = {
        vec3(0.9f, 0.9f, 0.9f), // rasterized: white
        vec3(1.0f, 0.8f, 0.1f), // filtered: yellow
        vec3(1.0f, 0.4f, 0.1f), // eroded: orange
        vec3(0.2f, 0.9f, 0.3f), // regions: green
        vec3(0.2f, 0.6f, 1.0f), // poly verts: blue
    };
    stage = clamp(stage, 0, 4);

    const float h = (stages.cellSize > 0.0f ? stages.cellSize : 0.3f) * 0.35f;
    lines.clear();
    for (const vec3& p : *sources[stage])
    {
        lines.push_back(LineT{p + vec3(-h, 0, -h), p + vec3(h, 0, h)});
        lines.push_back(LineT{p + vec3(-h, 0, h), p + vec3(h, 0, -h)});
    }
    if (!lines.empty())
        draw.drawLines(lines, 5.0f, {}, colors[stage]);
}

void DebugDrawTileInstanced(
    float y,
    float duration = 0.0f,
    bool drawUnloaded = false,
    bool drawDiagonal = true,
    ModelState3D state = ModelState3D());

} // namespace NavMesh

} // namespace VulpineNav
