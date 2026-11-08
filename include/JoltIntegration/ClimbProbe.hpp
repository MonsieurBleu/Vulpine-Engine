#pragma once

#include <cstdint>

#include <glm/glm.hpp>

namespace JPH
{
class NarrowPhaseQuery;
class BodyLockInterface;
class ObjectLayerFilter;
class BodyFilter;
}

namespace JoltVulpine::Climb
{

enum class Type : uint8_t { None, Low, High, Platform };

inline constexpr float kLowMaxHeight = 1.5f;      // grounded climbs up to this height are Low
inline constexpr float kPlatformThickness = 0.4f; // tops thinner than this are Platform, thicker are High

struct ProbeWorld
{
    const JPH::NarrowPhaseQuery& query;
    const JPH::BodyLockInterface& locks;
    const JPH::ObjectLayerFilter& layerFilter;
    const JPH::BodyFilter& surfaceFilter;  // bodies that can be climbed onto
    const JPH::BodyFilter& obstacleFilter; // bodies that can block the climb
};

struct ProbeResult
{
    bool found = false;
    glm::vec3 surface = glm::vec3(0);    // where the feet end up
    glm::vec3 edge = glm::vec3(0);
    glm::vec3 edgeNormal = glm::vec3(0);
    float thickness = 0.0f;
    Type type = Type::None;
};

ProbeResult Probe(const ProbeWorld& world, glm::vec3 feet, glm::vec3 facing, glm::vec3 bodySize, bool grounded);

} // namespace JoltVulpine::Climb
