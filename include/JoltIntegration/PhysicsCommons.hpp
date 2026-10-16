#pragma once

#include <JoltIntegration/JoltIntegration.hpp>
#include <Timer.hpp>

#include <Jolt/Physics/PhysicsSettings.h>

/*
    We are using the Mask implementation of Jolt broad face functions.
    Quote from Jolt implementation : 

        Broadphase & Layers : 
        A broadphase layer is suitable for an object if its group & inGroupsToInclude is not zero 
        and its group & inGroupsToExclude is zero.
        The broadphase layers are iterated from lowest to highest value and the first one that 
        matches is taken. If none match then it takes the last layer.

        Object Pair : 
        Uses group bits and mask bits. Two layers can collide if Object1.Group & Object2.Mask is 
        non-zero and Object2.Group & Object1.Mask is non-zero.
*/
#include <Jolt/Physics/Collision/BroadPhase//ObjectVsBroadPhaseLayerFilterMask.h>

#define JOLT_MAIN_THREAD_ID THREAD_ID_JOLT_MAIN

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <MappedEnum.hpp>

class Entity;

namespace JoltVulpine
{
    extern std::mutex physicsMutex;
    extern float currentPhysicFreq;

    extern LimitTimer physicsTicks;
    extern BenchTimer physicsTimer;
    extern BenchTimer physicsWorldUpdateTimer;
    extern BenchTimer physicsSystemsTimer;

    extern TickTimer physicInterpolationTick;
    extern std::mutex physicInterpolationMutex;
    extern bool doPhysicInterpolation;

    extern std::mutex bodiesToAddMutex;
    extern JPH::Array<JPH::BodyID> bodiesToAdd;

    void TraceCallback(const char *inFMT, ...);
    bool AssertFailedCallback(const char *inExpression, const char *inMessage, const char *inFile, uint inLine);

    extern JPH::PhysicsSystem *jPhysicsSystem;

    extern bool useVelocityBasedMovement;
    extern bool enablePhysics;

    JPH::BodyInterface &GetBodyInterface();

    namespace Layers
    {
        GENERATE_ENUM_FAST_REVERSE(
            Categories,
            ENVIRONEMENT,
            HIT_ZONE,
            PICK_UP,
            EDITOR1,
            EDITOR2
        )
    }

    class EnvironementCollideFilter : public JPH::ObjectLayerFilter
    {
        virtual bool ShouldCollide(JPH::ObjectLayer inLayer) const override
        {
            return true;
            auto layer = JPH::ObjectLayerPairFilterMask::sGetObjectLayer(1<<JoltVulpine::Layers::ENVIRONEMENT, 1<<JoltVulpine::Layers::ENVIRONEMENT);
            return JoltVulpine::jPhysicsSystem->GetObjectLayerPairFilter().ShouldCollide(inLayer, layer);
        }
    };
    
    class CategoryCollideFilter : public JPH::ObjectLayerFilter
    {
        public :
        Layers::Categories category;
        CategoryCollideFilter(Layers::Categories category) : ObjectLayerFilter(), category(category){};

        virtual bool ShouldCollide(JPH::ObjectLayer inLayer) const override
        {
            auto layer = JPH::ObjectLayerPairFilterMask::sGetObjectLayer(1<<category, 1<<category);
            return JoltVulpine::jPhysicsSystem->GetObjectLayerPairFilter().ShouldCollide(inLayer, layer);
        }
    };

    class BodyExcludeFilter : public JPH::BodyFilter
    {
        public : 
            JPH::BodyID b;

            BodyExcludeFilter(JPH::BodyID exclusion) : b(exclusion){};

            virtual bool ShouldCollide(const JPH::BodyID &inBodyID) const override
            {
                return  inBodyID != b;
            }
    };

    struct RaycastResult
    {
        bool hit = false;
        Entity *entity = nullptr;
        glm::vec3 point = glm::vec3(0);
    };

    RaycastResult simplRayCast(glm::vec3 origin, glm::vec3 direction, Layers::Categories category);
};

#define GLM_TO_JOLT_TYPE_ALIGNED(name, joltName, additional) \
    class V##name; \
    class J##name; \
    class V##name : public glm::name{ \
        private : additional\
        public : \
        V##name(const glm::name &v) : glm::name(v){}; \
        V##name(const JPH::joltName &v) : glm::name(*(glm::name *)&v){}; \
        using glm::name::name; \
        operator J##name&()  {return *((J##name *)this);}; \
        J##name & operator!(){return *((J##name *)this);}; \
    }; \
    class J##name : public JPH::joltName{ \
        public : \
        J##name(const JPH::joltName &v) : JPH::joltName(v){}; \
        J##name(const glm::name &v) : JPH::joltName(*(JPH::joltName *)&v){}; \
        using JPH::joltName::joltName; \
        operator V##name&()  {return *((V##name *)this);}; \
        V##name & operator!(){return *((V##name *)this);}; \
    };

GLM_TO_JOLT_TYPE_ALIGNED(vec4, Vec4, )
GLM_TO_JOLT_TYPE_ALIGNED(vec3, Vec3, float a;)
GLM_TO_JOLT_TYPE_ALIGNED(quat, Quat, )
GLM_TO_JOLT_TYPE_ALIGNED(mat4, Mat44, )


