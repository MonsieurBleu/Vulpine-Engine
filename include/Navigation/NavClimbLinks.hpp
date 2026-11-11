#pragma once

#include "Navigation/NavMesh.hpp"
#include "JoltIntegration/ClimbProbe.hpp"

#include <memory>
#include <vector>

namespace VulpineNav::ClimbLinks
{

inline constexpr float kProbePadding = 6.5f;

struct OffMeshLinks
{
    std::vector<float> verts; // start & end xyz
    std::vector<float> rads;
    std::vector<u8> dirs;
    std::vector<u8> areas;
    std::vector<u16> flags;
    std::vector<u32> userIds;

    int count() const { return (int)rads.size(); }
    void clear() { verts.clear(); rads.clear(); dirs.clear(); areas.clear(); flags.clear(); userIds.clear(); }
};

// A private physics world holding one tile's static triangles, so the climb probe can run on the
// build thread without touching or locking the game's world. 
// !! not thread safe !!
class Snapshot
{
public:
    Snapshot();
    ~Snapshot();
    Snapshot(const Snapshot&) = delete;
    Snapshot& operator=(const Snapshot&) = delete;

    // unindexed triangles, 3 verts each. returns false if there's nothing to probe against
    bool Build(const std::vector<float>& triangleVerts);
    JoltVulpine::Climb::ProbeWorld World() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m;
};

// climb and drop links starting on the tile's wall edges
void Generate(const rcPolyMesh& pmesh, const MeshTier::MeshTier& tier,
              const JoltVulpine::Climb::ProbeWorld& world, OffMeshLinks& out);

} // namespace VulpineNav::ClimbLinks
