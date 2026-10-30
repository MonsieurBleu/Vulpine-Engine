#include <ECS/ComponentTypeNavigation.hpp>
#include <AssetManager.hpp>
#include <AssetManagerUtils.hpp>

#include <algorithm>

using namespace VulpineNav::NavServer;
AUTOGEN_DATA_RW_FUNC(AgentProfile, height, maxClimb, maxSlopeDeg, radius)
AUTOGEN_DATA_RW_FUNC_STRUCT(AgentNavigation, (navmeshBuildPriority, hasGoal, goal, profileFromBody, arriveTolerance), (profile))


namespace
{
constexpr float kRepathGoalDrift = 1.0f;
constexpr float kPartialEndDistance = 4.0f;

// pushed this far off the corridor -> repath
constexpr float kOffCorridorDistance = 1.0f;
// the corridor end couldn't follow the goal this closely -> repath
constexpr float kTargetFollowError = 1.0f;
constexpr int kValidityLookAhead = 16;

constexpr float kLookahead = 1.5f;
constexpr float kOptimizeRangeRadii = 30.0f; // same default as DetourCrowd

constexpr float kCornerSlowAngleMin = 0.5236f; // 30 deg
constexpr float kCornerSlowAngleMax = 2.0944f; // 120 deg
constexpr float kCornerSlowDistance = 2.5f;
// erosion chamfers corners into a few close ones, the turn is measured across those
constexpr float kTurnWindow = 1.0f;
constexpr float kCornerMinScale = 0.35f;

constexpr float kArriveSlowRadius = 2.0f;
constexpr float kArriveMinScale = 0.2f;

// repathing cooldown after a result
constexpr uint16_t kRetryCooldownFrames = 30;
constexpr uint16_t kNoPathCooldownFrames = 120;

vec2 xz(const vec3& v)
{
    return vec2(v.x, v.z);
}

float xzDistance(const vec3& a, const vec3& b)
{
    return length(xz(a) - xz(b));
}

void pollPending(AgentNavigation& nav)
{
    auto& d = nav.detail;
    if (!d.pending.IsSet())
        return;

    PathStatus st = GetStatus(d.pending);
    if (st == PathStatus::Pending)
        return;

    const bool hasPath = st == PathStatus::Complete || st == PathStatus::Partial;
    if (hasPath && !d.corridor.Adopt(d.pending, nav.profile))
        st = PathStatus::NoPath;
    else if (!hasPath)
        d.corridor.Clear();

    d.status = (st == PathStatus::Invalid) ? PathStatus::NoPath : st;
    d.partialReason = d.status == PathStatus::Partial ? GetPartialReason(d.pending) : PartialReason::None;
    d.partialWatch = GetPartialWatch(d.pending);
    d.cooldownFrames = d.status == PathStatus::NoPath ? kNoPathCooldownFrames : kRetryCooldownFrames;

    // tiles may have changed between the search and now, validate the new corridor right away
    d.validatedVersion = 0;

    ReleasePath(d.pending);
    d.pending = {};
}

// keeps the corridor on the agent and the goal, false when it can't be used anymore
bool followCorridor(AgentNavigation& nav, vec3 pos)
{
    auto& d = nav.detail;
    if (d.corridor.Empty())
        return true;

    d.corridor.MovePosition(pos);
    if (xzDistance(d.corridor.Position(), pos) > kOffCorridorDistance)
        return false;

    const uint64_t version = GetNavmeshVersion();
    if (version != d.validatedVersion)
    {
        d.validatedVersion = version;
        if (!d.corridor.IsValid(kValidityLookAhead))
            return false;
    }

    // a partial corridor ends where the mesh stops, not at the goal
    if (d.status == PathStatus::Complete)
        d.corridor.MoveTarget(nav.goal);

    return true;
}

bool needsRepath(const AgentNavigation& nav, vec3 pos, bool corridorLost)
{
    const auto& d = nav.detail;

    if (d.pending.IsSet() || d.arrived)
        return false;

    if (d.status == PathStatus::Invalid)
        return true;

    // cooldown so an agent standing off the mesh doesn't re-request every result
    if (corridorLost)
        return d.cooldownFrames == 0;

    switch (d.status)
    {
    case PathStatus::Complete:
    {
        if (d.cooldownFrames > 0)
            return false;
        const vec3 target = d.corridor.Target();
        return xzDistance(target, nav.goal) > kTargetFollowError
            || glm::abs(target.y - nav.goal.y) > nav.profile.height;
    }
    case PathStatus::Partial:
        if (distance(nav.goal, d.pathGoal) > kRepathGoalDrift)
            return true;
        if (d.cooldownFrames > 0)
            return false;
        // tiles around the goal or the path end were added or removed since the search
        if (GetWatchVersion(d.pathGoal, d.corridor.Target()) != d.partialWatch)
            return true;
        // the other reasons won't change until the goal or the mesh does, retrying would just twitch
        if (d.partialReason == PartialReason::SearchLimit)
            return d.corridor.Empty() || xzDistance(pos, d.corridor.Target()) < kPartialEndDistance;
        return false;
    case PathStatus::NoPath:
        return distance(nav.goal, d.pathGoal) > kRepathGoalDrift || d.cooldownFrames == 0;
    default:
        return false;
    }
}

void steer(AgentNavigation& nav, vec3 pos)
{
    auto& d = nav.detail;
    d.moveDirection = vec2(0);
    d.speedScale = 0.0f;
    d.cornerCount = 0;

    if (d.arrived || d.corridor.Empty())
        return;

    Corner* corners = d.corners.data();
    int n = d.corridor.FindCorners(corners, PathCorridor::kMaxCorners);
    if (n > 0)
    {
        d.corridor.OptimizeVisibility(corners[std::min(1, n - 1)].pos, nav.profile.radius * kOptimizeRangeRadii);
        n = d.corridor.FindCorners(corners, PathCorridor::kMaxCorners);
    }
    d.cornerCount = n;
    if (n == 0)
        return;

    const vec2 pos2 = xz(pos);

    // aim `kLookahead` along the path so turns start before the corner
    vec3 target = corners[0].pos;
    bool pastFirstCorner = false;
    {
        float remaining = kLookahead;
        vec2 prev = pos2;
        for (int i = 0; i < n; i++)
        {
            const vec2 c = xz(corners[i].pos);
            const float seg = length(c - prev);
            if (seg >= remaining)
            {
                const vec2 t = prev + (c - prev) * (remaining / seg);
                target = vec3(t.x, corners[i].pos.y, t.y);
                pastFirstCorner = i > 0;
                break;
            }
            remaining -= seg;
            prev = c;
            target = corners[i].pos;
            pastFirstCorner = i > 0;
            if (corners[i].flags & (kCornerOffMesh | kCornerEnd))
                break;
        }
    }

    // cutting the corner can go through the wall it turns around
    if (pastFirstCorner && !d.corridor.CanSee(target))
        target = corners[0].pos;

    const vec2 dir = xz(target) - pos2;
    const float dirLen = length(dir);
    if (dirLen < 1e-4f)
        return;
    d.moveDirection = dir / dirLen;

    float scale = 1.0f;

    const vec2 toFirst = xz(corners[0].pos) - pos2;
    const float firstDist = length(toFirst);
    if (n >= 2 && !(corners[0].flags & kCornerEnd) && firstDist > 1e-4f)
    {
        int k = 1;
        while (k < n - 1 && length(xz(corners[k].pos) - xz(corners[0].pos)) < kTurnWindow)
            k++;
        const vec2 next = xz(corners[k].pos) - xz(corners[0].pos);
        const float nextLen = length(next);
        if (nextLen > 1e-4f)
        {
            const float turn = acos(glm::clamp(dot(toFirst / firstDist, next / nextLen), -1.0f, 1.0f));
            const float sharpness = smoothstep(kCornerSlowAngleMin, kCornerSlowAngleMax, turn);
            const float proximity = 1.0f - smoothstep(0.5f, kCornerSlowDistance, firstDist);
            scale = mix(1.0f, kCornerMinScale, sharpness * proximity);
        }
    }

    // ease in when the end of the path is among the corners
    const float stopDistance = nav.arriveTolerance + nav.goalRadius;
    float alongPath = 0.0f;
    vec2 prev = pos2;
    for (int i = 0; i < n; i++)
    {
        const vec2 c = xz(corners[i].pos);
        alongPath += length(c - prev);
        prev = c;
        if (corners[i].flags & kCornerEnd)
        {
            const float arrive = glm::clamp((alongPath - stopDistance) / kArriveSlowRadius, kArriveMinScale, 1.0f);
            scale = std::min(scale, arrive);
            break;
        }
    }

    d.speedScale = scale;
}

} // namespace


void ResetAgentNavigation(AgentNavigation& nav)
{
    auto& d = nav.detail;

    ReleasePath(d.pending); // safe on an unset handle
    d.pending = {};
    d.corridor.Clear();
    d.cornerCount = 0;
    d.moveDirection = vec2(0);
    d.speedScale = 0.0f;
    d.status = PathStatus::Invalid;
    d.arrived = false;
    d.partialReason = PartialReason::None;
    d.cooldownFrames = 0;
    d.validatedVersion = 0;
    d.partialWatch = 0;
}

void UpdateAgentNavigation(AgentNavigation& nav, vec3 pos)
{
    auto& d = nav.detail;

    if (!nav.hasGoal)
    {
        if (d.status != PathStatus::Invalid || d.pending.IsSet() || !d.corridor.Empty())
            ResetAgentNavigation(nav);
        return;
    }

    if (d.cooldownFrames > 0)
        d.cooldownFrames--;

    pollPending(nav);

    const bool corridorLost = !followCorridor(nav, pos);

    d.arrived = xzDistance(pos, nav.goal) <= nav.arriveTolerance + nav.goalRadius
             && glm::abs(pos.y - nav.goal.y) <= nav.profile.height;

    // over budget the request just waits a frame, needsRepath stays true until it goes through
    if (needsRepath(nav, pos, corridorLost) && CanRequestPath())
    {
        // agents with nothing to follow are standing still, serve them first
        const Priority priority = d.corridor.Empty() ? Priority::High : Priority::Normal;
        d.pending = RequestPath(pos, nav.goal, nav.profile, priority);
        d.pathGoal = nav.goal;

        if (d.corridor.Empty())
            d.status = PathStatus::Pending;
    }

    steer(nav, pos);
}
