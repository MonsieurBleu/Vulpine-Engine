#include "JoltIntegration/ClimbProbe.hpp"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Body/BodyLock.h>
#include <Jolt/Physics/Body/BodyLockInterface.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/CollideShape.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/NarrowPhaseQuery.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>

#include <cmath>

namespace JoltVulpine::Climb
{
namespace
{
using glm::vec2;
using glm::vec3;

JPH::Vec3 toJ(vec3 v) { return JPH::Vec3(v.x, v.y, v.z); }
vec3 toG(JPH::Vec3Arg v) { return vec3(v.GetX(), v.GetY(), v.GetZ()); }

JPH::RMat44 facingTransform(vec3 pos, vec3 dir2D)
{
    const float angle = -std::atan2(dir2D.z, dir2D.x);
    return JPH::RMat44::sRotationTranslation(JPH::Quat::sRotation(JPH::Vec3::sAxisY(), angle), toJ(pos));
}

bool surfaceNormal(const JPH::BodyLockInterface& locks, const JPH::BodyID& id, const JPH::SubShapeID& sub,
                   JPH::RVec3Arg point, vec3& out)
{
    JPH::BodyLockRead lock(locks, id);
    if (!lock.Succeeded())
        return false;
    out = toG(lock.GetBody().GetWorldSpaceSurfaceNormal(sub, point));
    return true;
}

class HighestSurface : public JPH::CastShapeCollector
{
public:
    HighestSurface(const JPH::BodyLockInterface& locks, vec3 dir2D, vec3 feet)
        : mLocks(locks), mDir2D(dir2D), best(feet) {}

    void AddHit(const ResultType& hit) override
    {
        const vec3 point = toG(hit.mContactPointOn2) + mDir2D * 0.25f;
        vec3 normal;
        if (!surfaceNormal(mLocks, hit.mBodyID2, hit.mSubShapeID2, toJ(point), normal))
            return;

        if (normal.y > 0.75f && point.y > best.y)
            best = point;
    }

    const JPH::BodyLockInterface& mLocks;
    vec3 mDir2D;
    vec3 best;
};

bool castRay(const ProbeWorld& world, vec3 origin, vec3 dirAndLength, vec3& outPoint, vec3& outNormal)
{
    JPH::RRayCast ray;
    ray.mOrigin = toJ(origin);
    ray.mDirection = toJ(dirAndLength);
    JPH::RayCastResult result;

    if (!world.query.CastRay(ray, result, JPH::BroadPhaseLayerFilter(), world.layerFilter, world.obstacleFilter)
        || result.mBodyID.IsInvalid())
        return false;

    const JPH::RVec3 point = ray.GetPointOnRay(result.mFraction);
    outPoint = toG(point);
    return surfaceNormal(world.locks, result.mBodyID, result.mSubShapeID2, point, outNormal);
}
} // namespace

ProbeResult Probe(const ProbeWorld& world, vec3 feet, vec3 facing, vec3 bodySize, bool grounded)
{
    ProbeResult res;

    const vec3 dir2D = glm::normalize(facing * vec3(1, 0, 1));

    // == highest walkable top in a 2m x 0.5m box swept down from 3m above, reaching ~1.25m ahead ==
    static const JPH::RefConst<JPH::Shape> sweepBox = JPH::RotatedTranslatedShapeSettings(
        JPH::Vec3(-1.0f, -0.25f, 0.0f),
        JPH::Quat::sIdentity(),
        JPH::BoxShapeSettings(JPH::Vec3(1.0f, 0.25f, 0.25f)).Create().Get()
    ).Create().Get();

    const vec3 sweep(0, -3, 0);
    const vec3 sweepStart = feet + dir2D * 1.25f - sweep;

    JPH::RShapeCast cast(sweepBox, JPH::Vec3::sReplicate(1.0f), facingTransform(sweepStart, dir2D), toJ(sweep));

    JPH::ShapeCastSettings castSettings;
    castSettings.SetBackFaceMode(JPH::EBackFaceMode::IgnoreBackFaces);
    castSettings.mUseShrunkenShapeAndConvexRadius = true;

    HighestSurface surface(world.locks, dir2D, feet);
    world.query.CastShape(cast, castSettings, JPH::RVec3::sZero(), surface, {}, world.layerFilter, world.surfaceFilter);

    if (surface.best.y <= feet.y)
        return res;

    // == room for the body between us and the top ==
    const float r = bodySize.x * 0.125f;
    const float h = bodySize.y;
    const float d = glm::length(vec2(feet.x - surface.best.x, feet.z - surface.best.z));

    static const JPH::RefConst<JPH::Shape> unitBox = JPH::BoxShapeSettings(JPH::Vec3::sReplicate(0.5f), 0.0f).Create().Get();
    const vec3 clearanceCenter = surface.best - dir2D * (r + d / 2.0f) + vec3(0, h / 2.0f + 0.25f, 0);

    // back faces count: against triangle meshes, a thin box crossing a wall can sit behind both of its faces
    JPH::CollideShapeSettings clearanceSettings;
    clearanceSettings.mBackFaceMode = JPH::EBackFaceMode::CollideWithBackFaces;

    JPH::AnyHitCollisionCollector<JPH::CollideShapeCollector> blocked;
    world.query.CollideShape(unitBox, JPH::Vec3(d + 4.0f * r, h, r), facingTransform(clearanceCenter, dir2D),
                             clearanceSettings, JPH::RVec3::sZero(), blocked, {}, world.layerFilter, world.obstacleFilter);
    if (blocked.HadHit())
        return res;

    res.found = true;
    res.surface = surface.best;

    // == edge position, normal and top thickness, from rays at the top's height ==
    vec3 edge = res.surface;
    vec3 edgeNormal(0);
    float cnt = 0.0f;
    float thickness = 0.0f;

    constexpr int kEdgeSteps = 8;
    constexpr float kEdgeMaxOffset = 0.5f;
    const float steepNormalY = std::cos(glm::radians(35.0f));

    for (int i = 0; i < kEdgeSteps; i++)
    {
        const float a = kEdgeMaxOffset * (1.0f - (float)i / (float)(kEdgeSteps - 1));
        vec3 position, normal;
        if (!castRay(world, res.surface - dir2D - vec3(0, a, 0), dir2D, position, normal))
            continue;

        thickness = glm::max(thickness, res.surface.y - position.y);

        if (normal.y < steepNormalY)
        {
            edge = position;
            cnt++;
            edgeNormal += normal;
        }

        if (i == 0 && cnt == 0.0f)
        {
            edge = position;
            edgeNormal = normal;
            cnt++;
        }
    }

    res.edge = edge;
    res.thickness = thickness;

    const vec3 awayFromEdge = glm::normalize(vec3(1, 0, 1) * (feet - edge));
    const float edgeHeight = glm::abs(res.surface.y - feet.y);
    if (grounded && edgeHeight <= kLowMaxHeight)
    {
        res.type = Type::Low;
        res.edgeNormal = awayFromEdge;
    }
    else
    {
        res.type = thickness >= kPlatformThickness ? Type::High : Type::Platform;
        res.edgeNormal = cnt > 0.0f ? glm::normalize(edgeNormal / cnt) : awayFromEdge;
    }
    return res;
}

} // namespace JoltVulpine::Climb
