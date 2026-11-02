#pragma once

#include "Navigation/NavMesh.hpp"
#include <cstdint>
#include <vector>

#include <glm/glm.hpp>
using namespace glm;


namespace VulpineNav::NavServer
{

struct AgentProfile
{
    float radius = Humanoid.radius;
    float height = Humanoid.height;
    float maxClimb = Humanoid.maxClimb;
    float maxSlopeDeg = Humanoid.maxSlopeDeg;

    // generic profiles
    static const AgentProfile Humanoid;

    operator MeshTier::AgentShape()
    {
        return { radius, height, maxClimb, maxSlopeDeg };
    }

    bool operator==(const AgentProfile& other) const noexcept
    {
        return radius == other.radius
            && height == other.height
            && maxClimb == other.maxClimb
            && maxSlopeDeg == other.maxSlopeDeg;
    }

    bool operator!=(const AgentProfile& other) const noexcept { return !(*this == other); }
};

// consider adding "smart pointer"-like system for tracking leaked paths
struct PathHandle
{
    uint32_t index = 0xffffffff;
    uint32_t generation = 0;

    bool IsSet() const { return index != 0xffffffff; }
};

enum class PathStatus : uint8_t
{
    Pending,   // queued/processing/tile not yet processed
    Complete,  // reached goal
    Partial,   // best path towards goal, out of nodes/unreachable/tile not loaded
    NoPath,    // can't even find a best path, start/end far from navmesh
    Invalid,   // handle expired or released
};

enum class Priority : uint8_t { Low = 0, Normal = 1, High = 2 };

void Init();
void Shutdown();

void Tick();

u64 GetNavmeshVersion();

// == Async path requests ==

// returns a handle that can be used to query the request status synchronously
// the query is treated internally on a separate thread
PathHandle RequestPath(
    vec3 start, vec3 goal,
    const AgentProfile& profile = {},
    Priority priority = Priority::Normal);

// fills outPoints when the path is either `Complete` or `Partial`.
PathStatus GetPath(PathHandle path, std::vector<vec3>& outPoints);

PathStatus GetStatus(PathHandle path);

// releases or cancels a path request
// must be called when done with the query, internal result is freed & handle is invalidated
// safe to call on an unset or already released handle
void ReleasePath(PathHandle path);

// == synchronous & fast queries ==
bool IsOnNavmesh(vec3 pos, const AgentProfile& profile = {});
bool ClosestPoint(vec3 pos, vec3& out, const AgentProfile& profile = {}, float searchRadius = 2.0f);
bool CanWalkStraight(vec3 from, vec3 to, const AgentProfile& profile = {});
bool MoveAlongSurface(vec3 from, vec3 to, vec3& out, const AgentProfile& profile = {});
bool RandomPointAround(vec3 center, float radius, vec3& out, const AgentProfile& profile = {});

struct DebugSegment
{
    vec3 start, end;
};

struct DebugSegments
{
    std::vector<DebugSegment> complete;
    std::vector<DebugSegment> partial;
    std::vector<DebugSegment> failed;

    void clear() { complete.clear(); partial.clear(); failed.clear(); }
};

void GetDebugSegments(DebugSegments& out);

template <typename LineT, typename Drawer>
void DebugDraw(Drawer& draw)
{
    static DebugSegments segments;
    static std::vector<LineT> lines[3];

    GetDebugSegments(segments);

    const std::vector<DebugSegment>* sources[3] = {&segments.complete, &segments.partial, &segments.failed};
    const vec3 colors[3] = {
        vec3(0.20f, 0.90f, 0.30f),
        vec3(1.00f, 0.60f, 0.10f),
        vec3(0.90f, 0.15f, 0.15f),
    };

    for (int i = 0; i < 3; i++)
    {
        if (sources[i]->empty())
            continue;

        lines[i].clear();
        lines[i].reserve(sources[i]->size());
        for (const DebugSegment& s : *sources[i])
            lines[i].push_back(LineT{s.start, s.end});

        draw.drawLines(lines[i], 0.0f, {}, colors[i]);
    }
}

} // namespace VulpineNav::NavServer
