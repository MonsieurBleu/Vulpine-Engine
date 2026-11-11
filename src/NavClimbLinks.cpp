#include "Navigation/NavClimbLinks.hpp"

#include <recastnavigation/Recast.h>

#include <cfloat>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyLock.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/CollideShape.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/PhysicsSystem.h>

namespace VulpineNav::ClimbLinks
{
namespace
{
class SingleBroadPhaseLayer final : public JPH::BroadPhaseLayerInterface
{
public:
    JPH::uint GetNumBroadPhaseLayers() const override { return 1; }
    JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer) const override { return JPH::BroadPhaseLayer(0); }
#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer) const override { return "climb snapshot"; }
#endif
};

class CollideAllObjectVsBroadPhase final : public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
    bool ShouldCollide(JPH::ObjectLayer, JPH::BroadPhaseLayer) const override { return true; }
};

class CollideAllObjectPairs final : public JPH::ObjectLayerPairFilter
{
public:
    bool ShouldCollide(JPH::ObjectLayer, JPH::ObjectLayer) const override { return true; }
};
} // namespace

struct Snapshot::Impl
{
    SingleBroadPhaseLayer bpLayers;
    CollideAllObjectVsBroadPhase objVsBp;
    CollideAllObjectPairs objPairs;
    JPH::ObjectLayerFilter layerFilter;
    JPH::BodyFilter bodyFilter;
    JPH::PhysicsSystem system;
    JPH::BodyID body;

    Impl() { system.Init(4, 0, 4, 4, bpLayers, objVsBp, objPairs); }

    void clear()
    {
        if (body.IsInvalid())
            return;
        JPH::BodyInterface& bi = system.GetBodyInterfaceNoLock();
        bi.RemoveBody(body);
        bi.DestroyBody(body);
        body = JPH::BodyID();
    }

    ~Impl() { clear(); }
};

Snapshot::Snapshot() : m(std::make_unique<Impl>()) {}
Snapshot::~Snapshot() = default;

bool Snapshot::Build(const std::vector<float>& v)
{
    m->clear();

    JPH::TriangleList tris;
    tris.reserve(v.size() / 9);
    for (size_t i = 0; i + 9 <= v.size(); i += 9)
        tris.push_back(JPH::Triangle(JPH::Float3(v[i], v[i + 1], v[i + 2]),
                                     JPH::Float3(v[i + 3], v[i + 4], v[i + 5]),
                                     JPH::Float3(v[i + 6], v[i + 7], v[i + 8])));
    if (tris.empty())
        return false;

    JPH::MeshShapeSettings settings(std::move(tris));
    JPH::ShapeSettings::ShapeResult shape = settings.Create();
    if (shape.HasError())
        return false;

    JPH::BodyInterface& bi = m->system.GetBodyInterfaceNoLock();
    m->body = bi.CreateAndAddBody(
        JPH::BodyCreationSettings(shape.Get(), JPH::RVec3::sZero(), JPH::Quat::sIdentity(), JPH::EMotionType::Static, 0),
        JPH::EActivation::DontActivate);
    if (m->body.IsInvalid())
        return false;

    m->system.OptimizeBroadPhase();
    return true;
}

JoltVulpine::Climb::ProbeWorld Snapshot::World() const
{
    return {m->system.GetNarrowPhaseQueryNoLock(), m->system.GetBodyLockInterfaceNoLock(),
            m->layerFilter, m->bodyFilter, m->bodyFilter};
}

namespace
{
using JoltVulpine::Climb::ProbeResult;
using JoltVulpine::Climb::ProbeWorld;
using JoltVulpine::Climb::Type;

constexpr float kSampleSpacing = 0.5f;
constexpr float kMaxRunLength = 2.0f;
constexpr float kRunHeightTolerance = 0.3f;
constexpr float kEndSnapDistance = 0.6f;
constexpr float kEndSnapHeight = 0.8f;
constexpr float kStepMargin = 0.05f;

constexpr float kMinLinkSpacing = 0.6f;
constexpr float kDropMaxReach = 1.5f;
constexpr float kDropReachStep = 0.25f;
constexpr float kSlideStep = 0.25f;
constexpr float kSlideMaxReach = 6.0f;
constexpr int kSlideMaxSteps = 64;

struct Sample
{
    vec3 start;
    vec3 end;
    vec3 facing;
    LinkKind kind = LinkKind::None;
};

vec3 polyVert(const rcPolyMesh& pm, int i)
{
    const unsigned short* v = &pm.verts[i * 3];
    return vec3(pm.bmin[0] + v[0] * pm.cs, pm.bmin[1] + v[1] * pm.ch, pm.bmin[2] + v[2] * pm.cs);
}

bool insideTile(const rcPolyMesh& pm, vec3 p)
{
    return p.x >= pm.bmin[0] && p.x <= pm.bmax[0] && p.z >= pm.bmin[2] && p.z <= pm.bmax[2];
}

vec2 closestOnSegment(vec2 p, vec2 a, vec2 b)
{
    const vec2 ab = b - a;
    const float len2 = dot(ab, ab);
    const float t = len2 > 1e-8f ? clamp(dot(p - a, ab) / len2, 0.0f, 1.0f) : 0.0f;
    return a + ab * t;
}

bool snapToPolyMesh(const rcPolyMesh& pm, vec3 p, vec3& out)
{
    const int nvp = pm.nvp;
    float best = kEndSnapDistance;
    bool found = false;

    for (int i = 0; i < pm.npolys; i++)
    {
        const unsigned short* poly = &pm.polys[i * 2 * nvp];
        vec3 verts[16];
        int n = 0;
        float y = 0.0f;
        for (; n < nvp && n < 16 && poly[n] != RC_MESH_NULL_IDX; n++)
        {
            verts[n] = polyVert(pm, poly[n]);
            y += verts[n].y;
        }
        if (n < 3 || glm::abs(p.y - y / n) > kEndSnapHeight)
            continue;

        const vec2 p2(p.x, p.z);
        bool anyPositive = false, anyNegative = false;
        float dist = FLT_MAX;
        vec2 closest = p2;
        for (int j = 0, k = n - 1; j < n; k = j++)
        {
            const vec2 a(verts[k].x, verts[k].z), b(verts[j].x, verts[j].z);
            const vec2 e = b - a, w = p2 - a;
            const float side = e.x * w.y - e.y * w.x;
            anyPositive |= side > 0.0f;
            anyNegative |= side < 0.0f;
            const vec2 c = closestOnSegment(p2, a, b);
            const float d = length(p2 - c);
            if (d < dist)
            {
                dist = d;
                closest = c;
            }
        }
        if (!(anyPositive && anyNegative))
        {
            out = p;
            return true;
        }
        if (dist < best)
        {
            best = dist;
            out = vec3(closest.x, p.y, closest.y);
            found = true;
        }
    }
    return found;
}

bool placeEnd(const rcPolyMesh& pm, vec3 p, vec3& out)
{
    if (!insideTile(pm, p))
    {
        out = p;
        return true;
    }
    return snapToPolyMesh(pm, p, out);
}

bool castDown(const ProbeWorld& world, vec3 origin, float length, vec3& hit, vec3& normal)
{
    JPH::RRayCast ray;
    ray.mOrigin = JPH::RVec3(origin.x, origin.y, origin.z);
    ray.mDirection = JPH::Vec3(0, -length, 0);
    JPH::RayCastResult result;
    if (!world.query.CastRay(ray, result, JPH::BroadPhaseLayerFilter(), world.layerFilter, world.obstacleFilter))
        return false;

    const JPH::RVec3 point = ray.GetPointOnRay(result.mFraction);
    JPH::BodyLockRead lock(world.locks, result.mBodyID);
    if (!lock.Succeeded())
        return false;
    const JPH::Vec3 n = lock.GetBody().GetWorldSpaceSurfaceNormal(result.mSubShapeID2, point);
    normal = vec3(n.GetX(), n.GetY(), n.GetZ());
    hit = vec3(point.GetX(), point.GetY(), point.GetZ());
    return true;
}


bool traceSlide(const ProbeWorld& world, vec3 ledge, vec3 p, vec3 normal, float walkableNormalY, vec3& landing)
{
    for (int i = 0; i < kSlideMaxSteps; i++)
    {
        vec2 downhill(normal.x, normal.z);
        const float len = length(downhill);
        if (len < 1e-3f)
            return false;
        downhill /= len;

        const vec3 next = p + vec3(downhill.x, 0.0f, downhill.y) * kSlideStep;
        if (length(vec2(next.x - ledge.x, next.z - ledge.z)) > kSlideMaxReach)
            return false; // past the gathered geometry

        vec3 hit;
        if (!castDown(world, next + vec3(0, 0.3f, 0), kDropMaxHeight + 0.3f, hit, normal))
            return false;
        if (hit.y > p.y + 0.05f)
            return false;

        p = hit;
        if (normal.y >= walkableNormalY)
        {
            landing = hit;
            return true;
        }
    }
    return false;
}

bool columnClear(const ProbeWorld& world, vec3 feet, float top, float radius, float maxClimb)
{
    const float bottom = feet.y + maxClimb + 0.1f;
    const float height = top + 0.1f - bottom;
    if (height <= 0.0f)
        return true;

    static const JPH::RefConst<JPH::Shape> unitBox = JPH::BoxShapeSettings(JPH::Vec3::sReplicate(0.5f), 0.0f).Create().Get();
    const float width = radius * 1.6f;
    const JPH::RMat44 transform = JPH::RMat44::sTranslation(JPH::RVec3(feet.x, bottom + height * 0.5f, feet.z));

    JPH::CollideShapeSettings settings;
    settings.mBackFaceMode = JPH::EBackFaceMode::CollideWithBackFaces;

    JPH::AnyHitCollisionCollector<JPH::CollideShapeCollector> hit;
    world.query.CollideShape(unitBox, JPH::Vec3(width, height, width), transform, settings,
                             JPH::RVec3::sZero(), hit, {}, world.layerFilter, world.obstacleFilter);
    return !hit.HadHit();
}

LinkKind climbKind(Type t)
{
    switch (t)
    {
    case Type::Low: return LinkKind::ClimbLow;
    case Type::High: return LinkKind::ClimbHigh;
    case Type::Platform: return LinkKind::ClimbPlatform;
    default: return LinkKind::None;
    }
}

Sample probeSample(const rcPolyMesh& pm, const MeshTier::MeshTier& tier, const ProbeWorld& world, vec3 feet, vec3 outward)
{
    Sample s;
    s.start = feet;
    s.facing = outward;

    const ProbeResult climb = JoltVulpine::Climb::Probe(
        world, feet, outward, vec3(tier.radius * 2.0f, tier.height, tier.radius * 2.0f), true);

    if (climb.found && climb.surface.y - feet.y > tier.maxClimb + kStepMargin
        && columnClear(world, feet, climb.surface.y, tier.radius, tier.maxClimb)
        && placeEnd(pm, climb.surface, s.end))
    {
        s.kind = climbKind(climb.type);
        return s;
    }

    const float walkableNormalY = glm::cos(glm::radians(tier.maxSlopeDeg));
    for (float d = tier.radius + 0.35f; d <= kDropMaxReach; d += kDropReachStep)
    {
        vec3 hit, normal;
        if (!castDown(world, feet + outward * d + vec3(0, 0.3f, 0), kDropMaxHeight + 0.3f, hit, normal))
            break;

        if (feet.y - hit.y < 0.0f)
            break;

        LinkKind kind = LinkKind::Drop;
        vec3 landing = hit;
        if (normal.y < walkableNormalY)
        {
            if (!traceSlide(world, feet, hit, normal, walkableNormalY, landing))
                break;
            kind = LinkKind::Slide;
        }

        if (feet.y - landing.y <= tier.maxClimb + kStepMargin)
            break;

        if (placeEnd(pm, landing, s.end))
            s.kind = kind;
        break;
    }
    return s;
}

bool duplicate(const Sample& s, const OffMeshLinks& out)
{
    for (int i = 0; i < out.count(); i++)
    {
        const float* v = &out.verts[i * 6];
        if (LinkKindFromUserId(out.userIds[i]) == s.kind
            && length(vec2(v[0] - s.start.x, v[2] - s.start.z)) < kMinLinkSpacing
            && glm::abs(v[4] - s.end.y) < kRunHeightTolerance)
            return true;
    }
    return false;
}

void emit(const Sample& s, const MeshTier::MeshTier& tier, OffMeshLinks& out)
{
    if (duplicate(s, out))
        return;

    u8 area = s.kind == LinkKind::Slide ? kAreaSlide : kAreaDrop;
    u16 flags = kFlagDrop;
    if (s.kind != LinkKind::Drop && s.kind != LinkKind::Slide)
    {
        area = s.kind == LinkKind::ClimbLow ? kAreaClimbLow : kAreaClimbHigh;
        flags = kFlagClimb;
    }

    out.verts.insert(out.verts.end(), {s.start.x, s.start.y, s.start.z, s.end.x, s.end.y, s.end.z});
    out.rads.push_back(tier.radius * 2.0f);
    out.dirs.push_back(0); // one-way
    out.areas.push_back(area);
    out.flags.push_back(flags);
    out.userIds.push_back(PackLinkUserId(s.kind, glm::abs(s.end.y - s.start.y), s.facing));
}

void emitRuns(const std::vector<Sample>& samples, const MeshTier::MeshTier& tier, OffMeshLinks& out)
{
    size_t runStart = 0;
    for (size_t i = 1; i <= samples.size(); i++)
    {
        const Sample& first = samples[runStart];
        const bool continues = i < samples.size()
            && samples[i].kind == first.kind
            && glm::abs(samples[i].end.y - first.end.y) < kRunHeightTolerance
            && (i - runStart) * kSampleSpacing < kMaxRunLength;
        if (continues)
            continue;

        if (first.kind != LinkKind::None)
            emit(samples[runStart + (i - runStart) / 2], tier, out);
        runStart = i;
    }
}
} // namespace

void Generate(const rcPolyMesh& pm, const MeshTier::MeshTier& tier, const ProbeWorld& world, OffMeshLinks& out)
{
    const int nvp = pm.nvp;
    std::vector<Sample> samples;

    for (int i = 0; i < pm.npolys; i++)
    {
        const unsigned short* poly = &pm.polys[i * 2 * nvp];
        int n = 0;
        vec2 centroid(0);
        for (; n < nvp && poly[n] != RC_MESH_NULL_IDX; n++)
        {
            const vec3 v = polyVert(pm, poly[n]);
            centroid += vec2(v.x, v.z);
        }
        if (n < 3)
            continue;
        centroid /= (float)n;

        for (int j = 0; j < n; j++)
        {
            if (poly[nvp + j] != RC_MESH_NULL_IDX)
                continue;

            const vec3 a = polyVert(pm, poly[j]);
            const vec3 b = polyVert(pm, poly[(j + 1) % n]);
            const vec2 e(b.x - a.x, b.z - a.z);
            const float len = length(e);
            if (len < 1e-3f)
                continue;

            vec2 normal = vec2(e.y, -e.x) / len;
            const vec2 mid = vec2(a.x + b.x, a.z + b.z) * 0.5f;
            if (dot(normal, mid - centroid) < 0.0f)
                normal = -normal;
            const vec3 outward(normal.x, 0.0f, normal.y);

            const int count = glm::max(1, (int)glm::round(len / kSampleSpacing));
            samples.clear();
            for (int k = 0; k < count; k++)
            {
                const vec3 feet = mix(a, b, (k + 0.5f) / count);
                samples.push_back(probeSample(pm, tier, world, feet, outward));
            }
            emitRuns(samples, tier, out);
        }
    }
}

} // namespace VulpineNav::ClimbLinks
