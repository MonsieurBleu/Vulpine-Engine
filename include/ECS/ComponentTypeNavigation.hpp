#pragma once

#include <cstdint>
#include <vector>

#include <glm/glm.hpp>

#include "Navigation/NavServer.hpp"

struct AgentNavigation
{
    bool initialized = false;
    vec3 lastPos = vec3(0);
    uint8_t navmeshBuildPriority = 0;

    // set by user
    VulpineNav::NavServer::AgentProfile profile = VulpineNav::NavServer::AgentProfile::Humanoid;
    bool profileFromBody = true;
    vec3 goal = vec3(0);
    bool hasGoal = false;
    float arriveTolerance = 0.5f;

    struct {
        // read by movement systems
        vec2 moveDirection = vec2(0); // horizontal & normalized, zero when arrived / no path
        VulpineNav::NavServer::PathStatus status = VulpineNav::NavServer::PathStatus::Invalid;
        bool arrived = false;

        // internal
        VulpineNav::NavServer::PathHandle pending;
        std::vector<vec3> waypoints;
        uint32_t waypointIdx = 0;

        // goal the current path was computed for
        vec3 pathGoal = vec3(0);
        uint64_t pathVersion = 0;

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

    vec3 prev = position;
    for (size_t i = d.waypointIdx; i < d.waypoints.size(); i++)
    {
        const vec3 wp = d.waypoints[i] + lift;
        lines.push_back(LineT{prev, wp});
        prev = wp;
    }
    if (!lines.empty())
        draw.drawLines(lines, 0.0f, {}, vec3(0.2f, 0.6f, 1.0f));

    if (d.status != VulpineNav::NavServer::PathStatus::Complete)
    {
        std::vector<LineT> gap{LineT{prev, nav.goal + lift}};
        draw.drawLines(gap, 0.0f, {}, vec3(0.6f, 0.6f, 0.6f));
    }
}

void ManageNavGarbage();