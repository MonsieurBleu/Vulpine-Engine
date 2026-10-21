#include <ECS/ComponentTypeNavigation.hpp>
#include <AssetManager.hpp>
#include <AssetManagerUtils.hpp>

AUTOGEN_DATA_RW_FUNC(AgentNavigation, navmeshBuildPriority)

using namespace VulpineNav::NavServer;

namespace
{
constexpr float kWaypointReach = 0.4f;

constexpr float kRepathGoalDrift = 1.0f;

constexpr float kPartialEndDistance = 4.0f;

// repathing cooldown after a result
constexpr uint16_t kRetryCooldownFrames = 30;
constexpr uint16_t kNoPathCooldownFrames = 120;

float xzDistance(const vec3& a, const vec3& b)
{
    return length(vec2(a.x - b.x, a.z - b.z));
}

void pollPending(AgentNavigation& nav)
{
    auto& d = nav.detail;
    if (!d.pending.IsSet())
        return;

    const PathStatus st = GetStatus(d.pending);
    if (st == PathStatus::Pending)
        return;

    if (st == PathStatus::Complete || st == PathStatus::Partial)
    {
        GetPath(d.pending, d.waypoints);
        d.waypointIdx = d.waypoints.size() > 1 ? 1 : 0;
    }
    else
    {
        d.waypoints.clear();
        d.waypointIdx = 0;
    }

    d.status = (st == PathStatus::Invalid) ? PathStatus::NoPath : st;

    switch (d.status)
    {
        case PathStatus::Complete:
        case PathStatus::Partial:
            d.cooldownFrames = kRetryCooldownFrames;
            break;
        default:                   
            d.cooldownFrames = kNoPathCooldownFrames; 
            break;
    }

    ReleasePath(d.pending);
    d.pending = {};
}

bool needsRepath(const AgentNavigation& nav, vec3 pos)
{
    const auto& d = nav.detail;

    if (d.pending.IsSet() || d.arrived)
        return false;

    if (d.status == PathStatus::Invalid || distance(nav.goal, d.pathGoal) > kRepathGoalDrift)
        return true;

    if (d.cooldownFrames > 0)
        return false;

    switch (d.status)
    {
    case PathStatus::NoPath:
        return true;
    case PathStatus::Partial:
        if (VulpineNav::NavServer::GetNavmeshVersion() != d.pathVersion)
            return true;
        return d.waypoints.empty() || xzDistance(pos, d.waypoints.back()) < kPartialEndDistance;
    case PathStatus::Complete:
        // walked the whole path but not arrived
        return d.waypointIdx >= d.waypoints.size();
    default:
        return false;
    }
}

void follow(AgentNavigation& nav, vec3 pos)
{
    auto& d = nav.detail;
    d.moveDirection = vec2(0);

    if (d.arrived)
        return;

    const std::vector<vec3>& wps = d.waypoints;

    while (d.waypointIdx < wps.size())
    {
        const vec3& wp = wps[d.waypointIdx];

        if (xzDistance(pos, wp) <= kWaypointReach)
        {
            d.waypointIdx++;
            continue;
        }

        // overshot
        if (d.waypointIdx > 0)
        {
            const vec3& prev = wps[d.waypointIdx - 1];
            const vec2 incoming(wp.x - prev.x, wp.z - prev.z);
            const vec2 fromWp(pos.x - wp.x, pos.z - wp.z);
            if (dot(incoming, fromWp) > 0.0f)
            {
                d.waypointIdx++;
                continue;
            }
        }
        break;
    }

    if (d.waypointIdx >= wps.size())
        return;

    const vec3& target = wps[d.waypointIdx];
    vec2 dir(target.x - pos.x, target.z - pos.z);
    float len = length(dir);
    if (len > 1e-4f)
        d.moveDirection = dir / len;
}

} // namespace


void ResetAgentNavigation(AgentNavigation& nav)
{
    auto& d = nav.detail;

    ReleasePath(d.pending); // safe on an unset handle
    d.pending = {};
    d.waypoints.clear();
    d.waypointIdx = 0;
    d.moveDirection = glm::vec2(0);
    d.status = PathStatus::Invalid;
    d.arrived = false;
    d.cooldownFrames = 0;
    d.pathVersion = 0;
}

void UpdateAgentNavigation(AgentNavigation& nav, glm::vec3 pos)
{
    auto& d = nav.detail;

    if (!nav.hasGoal)
    {
        if (d.status != PathStatus::Invalid || d.pending.IsSet() || !d.waypoints.empty())
            ResetAgentNavigation(nav);
        return;
    }

    if (d.cooldownFrames > 0)
        d.cooldownFrames--;

    pollPending(nav);

    d.arrived = xzDistance(pos, nav.goal) <= nav.arriveTolerance
             && glm::abs(pos.y - nav.goal.y) <= nav.profile.height;

    if (needsRepath(nav, pos))
    {
        d.pending = RequestPath(pos, nav.goal, nav.profile);
        d.pathGoal = nav.goal;
        d.pathVersion = GetNavmeshVersion();

        if (d.waypoints.empty())
            d.status = PathStatus::Pending;
    }

    follow(nav, pos);
}