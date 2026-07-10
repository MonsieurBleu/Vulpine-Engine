#pragma  once

#define JPH_DEBUG_RENDERER

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Physics/PhysicsSystem.h>

#define JOLT_VULPINE_MAX_ENTITY_SENSORS 8

struct JoltBody : JPH::BodyID
{
    JoltBody(const uint id = JPH::BodyID::cInvalidBodyID) : JPH::BodyID(id){};
    JoltBody(const JPH::BodyID &id) : JPH::BodyID(id){};
    JPH::BodyID sensors[JOLT_VULPINE_MAX_ENTITY_SENSORS];
};

/*
    If an entity has this component, it has sensor or kynematic main body.
    If the flag is true, the main body is kynematic.
*/
typedef bool KynematicFlag;
