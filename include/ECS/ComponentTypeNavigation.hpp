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

    struct {
        bool initialized = false;
        vec3 lastPos = vec3(0);

        // read by movement systems
        vec2 moveDirection = vec2(0); // horizontal & normalized, zero when arrived / no path
        float speedScale = 0.0f;      // 0 to 1, modified to smooth out movement
        VulpineNav::NavServer::PathStatus status = VulpineNav::NavServer::PathStatus::Invalid;
        bool arrived = false;

        std::array<VulpineNav::NavServer::Corner, VulpineNav::NavServer::PathCorridor::kMaxCorners> corners{};
        int cornerCount = 0;

        // internal
        VulpineNav::NavServer::PathHandle pending;
        VulpineNav::NavServer::PathCorridor corridor;

        // goal the current path was requested for
        vec3 pathGoal = vec3(0);
        uint64_t pathVersion = 0;
        uint64_t validatedVersion = 0;

        uint16_t cooldownFrames = 0;
    } detail;
};

void UpdateAgentNavigation(AgentNavigation& nav, vec3 position);

void ResetAgentNavigation(AgentNavigation& nav);

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
