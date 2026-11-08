#include <ECS/ComponentTypeNavigation.hpp>
#include <AssetManager.hpp>
#include <AssetManagerUtils.hpp>

#include <algorithm>

using namespace VulpineNav;
using namespace VulpineNav::NavServer;
AUTOGEN_DATA_RW_FUNC(AgentProfile, height, maxClimb, maxSlopeDeg, radius)
AUTOGEN_DATA_RW_FUNC_STRUCT(AgentNavigation, (navmeshBuildPriority, navmeshLoadRadius, hasGoal, goal, profileFromBody, arriveTolerance), (profile))


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

// once arrived, being nudged this far past the arrival distance doesn't undo it
constexpr float kArriveHysteresis = 0.3f;

constexpr float kArriveSlowRadius = 2.0f;
constexpr float kArriveMinScale = 0.2f;

constexpr float kLinkSlowRadius = 1.5f;
constexpr float kLinkMinScale = 0.3f;
constexpr float kLinkTriggerRadii = 2.25f;
constexpr uint16_t kLinkTimeoutFrames = 300;

constexpr float kStaleRequestDistance = 1.0f;

constexpr uint16_t kRecoveryCheckFrames = 15;
constexpr uint16_t kRecoveryRetargetFrames = 30;
constexpr float kRecoverySpeedScale = 0.6f;
constexpr uint16_t kBlockedLinkFrames = 600;

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

PathFilter pathFilter(const AgentNavigation& nav)
{
    PathFilter f;
    f.includeFlags = kFlagWalk | (nav.canClimb ? kFlagClimb : 0) | (nav.canDrop ? kFlagDrop : 0);
    int n = 0;
    for (const auto& b : nav.detail.blockedLinks)
        if (b.framesLeft > 0)
            f.excludedLinks[n++] = b.poly;
    f.reservation = nav.detail.reservation;
    f.reservedLink = nav.detail.reservedLink;
    f.reservedDistance = nav.detail.reservedDistance;
    return f;
}

// reserves the link the agent takes next
void updateLinkReservation(AgentNavigation& nav, vec3 pos)
{
    auto& d = nav.detail;
    uint64_t link = 0;
    float distance = 0.0f;
    if (d.traversal.active)
    {
        link = d.traversal.poly;
    }
    else if (d.cornerCount > 0 && (d.corners[d.cornerCount - 1].flags & kCornerOffMesh))
    {
        link = d.corners[d.cornerCount - 1].poly;
        vec2 prev = xz(pos);
        for (int i = 0; i < d.cornerCount; i++)
        {
            distance += length(xz(d.corners[i].pos) - prev);
            prev = xz(d.corners[i].pos);
        }
    }

    if (link != d.reservedLink)
        d.aheadChecked = 0;
    ReserveLink(nav.profile, d.reservation, link, distance);
    d.reservedLink = link;
    d.reservedDistance = distance;
}

// true while the agent is off the mesh and walking back to it, moveDirection is set then
bool updateRecovery(AgentNavigation& nav, vec3 pos, bool corridorLost)
{
    auto& d = nav.detail;
    auto& r = d.recovery;
    r.frames++;

    if (d.arrived)
    {
        r.active = false;
        return false;
    }

    if (!r.active)
    {
        // only worth checking with nothing to follow, or a corridor we've been pushed away from
        if (!(d.corridor.Empty() || corridorLost) || r.frames < kRecoveryCheckFrames)
            return false;
        r.frames = 0;
        if (CanStartPath(pos, nav.profile))
            return false;

        r.active = true;
        r.frames = kRecoveryRetargetFrames; // pick a target right away
        d.corridor.Clear();
    }
    else if (CanStartPath(pos, nav.profile))
    {
        // back on the mesh
        r.active = false;
        r.frames = 0;
        ReleasePath(d.pending);
        d.pending = {};
        d.status = PathStatus::Invalid;
        d.cooldownFrames = 0;
        return false;
    }

    if (r.frames >= kRecoveryRetargetFrames)
    {
        r.frames = 0;
        if (!FindRecoveryPoint(pos, r.target, nav.profile))
            r.target = pos; // nothing within reach, stay put
    }

    // straight below 
    vec2 dir = xz(r.target) - xz(pos);
    if (length(dir) < 0.3f)
        dir = xz(nav.goal) - xz(pos);

    d.moveDirection = length(dir) > 1e-4f ? normalize(dir) : vec2(0);
    d.speedScale = kRecoverySpeedScale;
    d.cornerCount = 0;
    return true;
}

// more agents got ahead of us at our next link since we last looked
bool nextLinkGotBusier(const AgentNavigation& nav)
{
    const auto& d = nav.detail;
    if (!d.reservedLink || d.traversal.active || d.pending.IsSet())
        return false;
    return LinkAgentsAhead(nav.profile, d.reservation, d.reservedLink, d.reservedDistance) > d.aheadChecked;
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
    if (hasPath && !d.corridor.Adopt(d.pending, nav.profile, pathFilter(nav)))
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
    const vec3 onCorridor = d.corridor.Position();
    if (xzDistance(onCorridor, pos) > kOffCorridorDistance || glm::abs(onCorridor.y - pos.y) > nav.profile.height * 0.5f)
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

    // ease in when the end of the path, or a link to cross, is among the corners
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
        if (corners[i].flags & kCornerOffMesh)
        {
            scale = std::min(scale, glm::clamp(alongPath / kLinkSlowRadius, kLinkMinScale, 1.0f));
            break;
        }
    }

    d.speedScale = scale;
}

// hands the agent to the locomotion system once it's at the start of a link
void beginLinkIfReached(AgentNavigation& nav, vec3 pos)
{
    auto& d = nav.detail;
    if (d.cornerCount == 0)
        return;

    const Corner& last = d.corners[d.cornerCount - 1];
    if (!(last.flags & kCornerOffMesh))
        return;
    if (xzDistance(pos, last.pos) > nav.profile.radius * kLinkTriggerRadii)
        return;

    vec3 start, end;
    uint32_t userId = 0;
    if (!d.corridor.MoveOverLink(last.poly, start, end, userId))
        return;

    // anything in flight was searched from before the link
    ReleasePath(d.pending);
    d.pending = {};

    auto& t = d.traversal;
    t = {};
    t.active = true;
    t.kind = LinkKindFromUserId(userId);
    t.start = start;
    t.end = end;
    t.facing = LinkFacingFromUserId(userId);
    t.poly = last.poly;

    d.moveDirection = vec2(0);
    d.speedScale = 0.0f;
}

void tickBlockedLinks(AgentNavigation& nav)
{
    for (auto& b : nav.detail.blockedLinks)
        if (b.framesLeft > 0)
            b.framesLeft--;
}

} // namespace


void FinishLinkTraversal(AgentNavigation& nav, bool success)
{
    auto& d = nav.detail;
    auto& t = d.traversal;
    if (!t.active)
        return;
    t.active = false;

    ReportLinkResult(nav.profile, t.poly, success);
    if (success)
        return;

    // avoid it for a while, replacing the block closest to expiring
    auto* slot = &d.blockedLinks[0];
    for (auto& b : d.blockedLinks)
        if (b.framesLeft < slot->framesLeft)
            slot = &b;
    *slot = {t.poly, kBlockedLinkFrames};

    d.corridor.Clear();
    d.status = PathStatus::Invalid;
    d.cooldownFrames = 0;
}

void ResetAgentNavigation(AgentNavigation& nav)
{
    auto& d = nav.detail;

    ReleasePath(d.pending);
    d.pending = {};
    ReserveLink(nav.profile, d.reservation, 0, 0.0f);
    d.reservedLink = 0;
    d.aheadChecked = 0;
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
    d.traversal = {};
    d.recovery = {};
}

void UpdateAgentNavigation(AgentNavigation& nav, vec3 pos)
{
    auto& d = nav.detail;

    if (!nav.hasGoal)
    {
        if (d.status != PathStatus::Invalid || d.pending.IsSet() || !d.corridor.Empty() || d.traversal.active)
            ResetAgentNavigation(nav);
        return;
    }

    tickBlockedLinks(nav);
    if (d.cooldownFrames > 0)
        d.cooldownFrames--;

    if (d.traversal.active)
    {
        d.moveDirection = vec2(0);
        d.speedScale = 0.0f;
        d.arrived = false;
        if (++d.traversal.frames <= kLinkTimeoutFrames)
            return;
        FinishLinkTraversal(nav, false);
    }

    if (d.pending.IsSet() && distance(pos, d.requestPos) > kStaleRequestDistance)
    {
        ReleasePath(d.pending);
        d.pending = {};
        if (d.corridor.Empty())
            d.status = PathStatus::Invalid;
        else
            d.cooldownFrames = 0;
    }

    pollPending(nav);

    const bool corridorLost = !followCorridor(nav, pos);

    d.arrived = xzDistance(pos, nav.goal) <= nav.arriveTolerance + nav.goalRadius + (d.arrived ? kArriveHysteresis : 0.0f)
             && glm::abs(pos.y - nav.goal.y) <= nav.profile.height;

    if (updateRecovery(nav, pos, corridorLost))
    {
        updateLinkReservation(nav, pos);
        return;
    }

    const bool busier = nextLinkGotBusier(nav);

    // over budget check
    if ((needsRepath(nav, pos, corridorLost) || busier) && CanRequestPath())
    {
        if (busier)
            d.aheadChecked = LinkAgentsAhead(nav.profile, d.reservation, d.reservedLink, d.reservedDistance);

        // prioritize agents that aren't already moving
        const Priority priority = d.corridor.Empty() ? Priority::High : Priority::Normal;
        d.pending = RequestPath(pos, nav.goal, nav.profile, priority, pathFilter(nav));
        d.pathGoal = nav.goal;
        d.requestPos = pos;

        if (d.corridor.Empty())
            d.status = PathStatus::Pending;
    }

    steer(nav, pos);
    beginLinkIfReached(nav, pos);
    updateLinkReservation(nav, pos);
}
