#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include <glm/glm.hpp>

#include "Navigation/NavServer.hpp"

struct AgentNavigation
{
    // === Serialized Parameters ===
    
    uint8_t navmeshBuildPriority = 0;
    // can only be set once on creation :/
    uint8_t navmeshLoadRadius = VulpineNav::NavMesh::kLoadRadius;
    VulpineNav::NavServer::AgentProfile profile = VulpineNav::NavServer::AgentProfile::Humanoid;
    bool profileFromBody = true;
    vec3 goal = vec3(0);
    bool hasGoal = false;
    float arriveTolerance = 0.5f;
    
    // === Non-Serialized Parameters ===
    
    // radius of whatever stands at the goal. arrival happens at arriveTolerance + goalRadius
    float goalRadius = 0.0f;
    // top speed the locomotion system asks
    float moveSpeed = 5.0f;
    // which links paths may use
    bool canClimb = true;
    bool canDrop = true;

    struct {
        bool initialized = false;
        vec3 lastPos = vec3(0);
        int loaderRadius = VulpineNav::NavMesh::kLoadRadius; // navmeshLoadRadius when the loader was created

        // read by movement systems
        vec2 moveDirection = vec2(0); // horizontal & normalized, zero when arrived / no path
        float speedScale = 0.0f;      // 0 to 1, modified to smooth out movement
        float pathWidth = 2.0f;
        vec2 pathRoom = vec2(1.0f); // free mesh to the left (x) and right (y) of where we stand
        VulpineNav::NavServer::PathStatus status = VulpineNav::NavServer::PathStatus::Invalid;
        VulpineNav::NavServer::PartialReason partialReason = VulpineNav::NavServer::PartialReason::None;
        bool arrived = false;

        std::array<VulpineNav::NavServer::Corner, VulpineNav::NavServer::PathCorridor::kMaxCorners> corners{};
        int cornerCount = 0;

        // internal
        VulpineNav::NavServer::PathHandle pending;
        VulpineNav::NavServer::PathCorridor corridor;

        // goal the current path was requested for and where we were then
        vec3 pathGoal = vec3(0);
        vec3 requestPos = vec3(0);
        uint64_t validatedVersion = 0;
        uint64_t partialWatch = 0;

        uint16_t cooldownFrames = 0;
        uint8_t widthFrames = 0xff;

        struct BlockedLink
        {
            uint64_t poly = 0;
            uint16_t framesLeft = 0;
        };
        std::array<BlockedLink, 4> blockedLinks{};

        uint32_t reservation = 0;
        uint64_t reservedLink = 0;
        float reservedDistance = 0.0f;
        int aheadChecked = 0;

        // off mesh recovery
        struct
        {
            bool active = false;
            vec3 target = vec3(0);
            uint16_t frames = 0;
        } recovery;

        struct
        {
            float best = 0.0f;
            uint16_t stuckFrames = 0;
            uint16_t framesLeft = 0;
            bool bent = false; // avoidance turned us too much off the path last frame
        } unstick;

        struct
        {
            vec2 dir = vec2(0);
            bool held = false;
            uint16_t heldFrames = 0;
        } steering;

        // crossing a navigation link
        struct
        {
            bool active = false;
            VulpineNav::LinkKind kind = VulpineNav::LinkKind::None;
            vec3 start = vec3(0);
            vec3 end = vec3(0);
            vec3 facing = vec3(0);
            uint64_t poly = 0;
            uint16_t frames = 0;

            // owned by the locomotion system
            uint8_t phase = 0;
            uint16_t phaseFrame = 0;
            uint16_t landedFrames = 0;
            bool climbSeen = false;
        } traversal;
    } detail;
};

void UpdateAgentNavigation(AgentNavigation& nav, vec3 position);

void ResetAgentNavigation(AgentNavigation& nav);

void FinishLinkTraversal(AgentNavigation& nav, bool success);

// 0 on open ground, 1 on the narrowest paths
float PathNarrowness(const AgentNavigation& nav);

template <typename LineT, typename Drawer>
void DebugDrawAgentNavigation(Drawer& draw, const AgentNavigation& nav, vec3 position)
{
    const auto& d = nav.detail;
    if (!nav.hasGoal || d.arrived)
        return;

    const vec3 lift(0.0f, 0.15f, 0.0f);
    std::vector<LineT> lines;

    vec3 prev = position + lift;
    for (int i = 0; i < d.cornerCount; i++)
    {
        const vec3 c = d.corners[i].pos + lift;
        lines.push_back(LineT{prev, c});
        prev = c;
    }
    if (!lines.empty())
        draw.drawLines(lines, 0.0f, {}, vec3(0.2f, 0.6f, 1.0f));

    if (d.status != VulpineNav::NavServer::PathStatus::Complete)
    {
        std::vector<LineT> gap{LineT{prev, nav.goal + lift}};
        draw.drawLines(gap, 0.0f, {}, vec3(0.6f, 0.6f, 0.6f));
    }
}
