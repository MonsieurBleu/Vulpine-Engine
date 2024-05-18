#pragma once
#include "GeometryUtils.hpp"

#include <vector>
#include <memory>
#include <optional>
#include <algorithm>
#include <utility>
#include <unordered_map>
#include <set>
#include <functional>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct PhysicsMaterial {
    PhysicsMaterial(
        float friction = 0.0f,
        float restitution = 0.0f,
        float linearDamping = 0.0f,
        float angularDamping = 0.0f
    );

    ~PhysicsMaterial();

    float friction;
    float restitution;
    float linearDamping;
    float angularDamping;
};

class AABBCollider;
class OBBCollider;
class SphereCollider;
class CapsuleCollider;
class ConvexCollider;
class RigidBody;

class Collider {
public: 

    enum class ColliderType {
        OBB,
        SPHERE,
        CAPSULE,
        CONVEX
    };

    virtual ColliderType getColliderType() const = 0;

    virtual glm::mat3 getInertiaTensor() const = 0;

    virtual float getVolume() const = 0;

    geometry::DiscreteIntersectionInfo getCollisionInfo(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const Collider* collider, bool continuous);

    virtual geometry::BoundingBox getBoundingBox(const glm::vec3& position, const glm::quat& rotation) const = 0;

protected:

    virtual geometry::DiscreteIntersectionInfo getCollisionInfoOBB(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const OBBCollider* collider, bool continuous) const {return geometry::DiscreteIntersectionInfo::none();}
    virtual geometry::DiscreteIntersectionInfo getCollisionInfoSphere(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const SphereCollider* collider, bool continuous) const {return geometry::DiscreteIntersectionInfo::none();}
    virtual geometry::DiscreteIntersectionInfo getCollisionInfoCapsule(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const CapsuleCollider* collider, bool continuous) const {return geometry::DiscreteIntersectionInfo::none();}
    virtual geometry::DiscreteIntersectionInfo getCollisionInfoConvex(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const ConvexCollider* collider, bool continuous) const {return geometry::DiscreteIntersectionInfo::none();}

};

class OBBCollider : public Collider {
public:

    OBBCollider(const glm::vec3& halfDimensions);

    glm::vec3 size;

    ColliderType getColliderType() const override {return ColliderType::OBB;}

    glm::mat3 getInertiaTensor() const override;

    float getVolume() const override;

    geometry::DiscreteIntersectionInfo getCollisionInfoOBB(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const OBBCollider* collider, bool continuous) const override;
    geometry::DiscreteIntersectionInfo getCollisionInfoSphere(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const SphereCollider* collider, bool continuous) const override;
    geometry::DiscreteIntersectionInfo getCollisionInfoCapsule(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const CapsuleCollider* collider, bool continuous) const override;
    geometry::DiscreteIntersectionInfo getCollisionInfoConvex(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const ConvexCollider* collider, bool continuous) const override;

    geometry::BoundingBox getBoundingBox(const glm::vec3& position, const glm::quat& rotation) const override;

};

class SphereCollider : public Collider {
public:

    SphereCollider(float radius);

    float radius;

    ColliderType getColliderType() const override {return ColliderType::SPHERE;}

    glm::mat3 getInertiaTensor() const override;

    float getVolume() const override;

    geometry::DiscreteIntersectionInfo getCollisionInfoOBB(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const OBBCollider* collider, bool continuous) const override;
    geometry::DiscreteIntersectionInfo getCollisionInfoSphere(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const SphereCollider* collider, bool continuous) const override;
    geometry::DiscreteIntersectionInfo getCollisionInfoCapsule(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const CapsuleCollider* collider, bool continuous) const override;
    geometry::DiscreteIntersectionInfo getCollisionInfoConvex(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const ConvexCollider* collider, bool continuous) const override;

    geometry::BoundingBox getBoundingBox(const glm::vec3& position, const glm::quat& rotation) const override;

};

class CapsuleCollider : public Collider {
public:

    CapsuleCollider(float length, float radius);

    float halfLength;
    float radius;

    ColliderType getColliderType() const override {return ColliderType::CAPSULE;}

    glm::mat3 getInertiaTensor() const override;

    float getVolume() const override;

    geometry::DiscreteIntersectionInfo getCollisionInfoOBB(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const OBBCollider* collider, bool continuous) const override;
    geometry::DiscreteIntersectionInfo getCollisionInfoSphere(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const SphereCollider* collider, bool continuous) const override;
    geometry::DiscreteIntersectionInfo getCollisionInfoCapsule(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const CapsuleCollider* collider, bool continuous) const override;
    geometry::DiscreteIntersectionInfo getCollisionInfoConvex(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const ConvexCollider* collider, bool continuous) const override;

    geometry::BoundingBox getBoundingBox(const glm::vec3& position, const glm::quat& rotation) const override;

};

class ConvexCollider : public Collider {
public:

    ConvexCollider(const std::vector<glm::vec3>& vertices);

    std::vector<glm::vec3> vertices;

    ColliderType getColliderType() const override {return ColliderType::CONVEX;}

    glm::mat3 getInertiaTensor() const override;

    float getVolume() const override;

    geometry::DiscreteIntersectionInfo getCollisionInfoOBB(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const OBBCollider* collider, bool continuous) const override;
    geometry::DiscreteIntersectionInfo getCollisionInfoSphere(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const SphereCollider* collider, bool continuous) const override;
    geometry::DiscreteIntersectionInfo getCollisionInfoCapsule(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const CapsuleCollider* collider, bool continuous) const override;
    geometry::DiscreteIntersectionInfo getCollisionInfoConvex(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const ConvexCollider* collider, bool continuous) const override;

    geometry::BoundingBox getBoundingBox(const glm::vec3& position, const glm::quat& rotation) const override;

};

class ForceField {
public:

    using ForceFieldFunction = std::function<glm::vec3(const glm::vec3& position)>;

    using Ref = std::shared_ptr<ForceField>;

    // todo move in the cpp
    template <typename... Args>
    static ForceField::Ref global(const ForceFieldFunction& forceFieldFunction) {
        return std::make_shared<ForceField>(nullptr, glm::vec3(), glm::identity<glm::quat>(), forceFieldFunction);
    }

    template <typename... Args>
    static ForceField::Ref make(Args&&... args) {
        return std::make_shared<ForceField>(std::forward<Args>(args)...);
    }

    ForceField(
        Collider* collider,
        glm::vec3 position = {},
        glm::quat rotation = glm::identity<glm::quat>(),
        const ForceFieldFunction& forceFieldFunction = uniformForceField({0.0f, 0.0f, 0.0f})
    );

    ~ForceField();

    Collider* getCollider() const {return collider;}
    glm::vec3 getPosition() const {return position;}
    glm::quat getRotation() const {return rotation;}

    bool isGlobal() const {return collider == nullptr;}
    glm::vec3 getAcceleration(const glm::vec3& position) const {return forceFieldFunction(position);}

    void setPosition(const glm::vec3& position) {this->position = position;}
    void setRotation(const glm::quat& rotation) {this->rotation = rotation;}
    void setForceFieldFunction(const ForceFieldFunction& forceFieldFunction) {this->forceFieldFunction = forceFieldFunction;}

    static ForceFieldFunction uniformForceField(const glm::vec3& acceleration) {
        return [acceleration](const glm::vec3& position) {
            return acceleration;
        };
    }

    static ForceFieldFunction pointForceField(const glm::vec3& center, float pull, float radius) {
        return [center, pull, radius](const glm::vec3& position) {
            const float distance = glm::distance(center, position);
            return (center - position) * std::lerp(pull, 0.0f, std::min(distance / radius, 1.0f)) / distance;
        };
    }

private:

    Collider* collider;

    glm::vec3 position;
    glm::quat rotation;

    ForceFieldFunction forceFieldFunction;

};

class RigidBody {
public:

    using Ref = std::shared_ptr<RigidBody>;

    // todo move in the cpp
    template <typename... Args>
    static RigidBody::Ref make(Args&&... args) {
        return std::make_shared<RigidBody>(std::forward<Args>(args)...);
    }

    RigidBody(
        Collider* collider,
        float mass = 1.0f,
        PhysicsMaterial material = {},
        int layer = 0,
        bool staticBody = false,
        const glm::vec3& position = {},
        const glm::vec3& velocity = {},
        bool fixedRotation = false,
        const glm::quat& rotation = glm::identity<glm::quat>(),
        const glm::vec3& angularVelocity = {}
    );

    ~RigidBody();

    void tick(const std::vector<ForceField::Ref>& forceFields, float delta);

    int getLayer() const {return layer;}
    glm::vec3 getPosition() const {return position;}
    glm::vec3 getVelocity() const {return isStatic() ? glm::vec3(0.0f, 0.0f, 0.0f) : velocity;}
    glm::quat getRotation() const {return rotation;}
    glm::vec3 getAngularVelocity() const {return canRotate() ? angularVelocity : glm::vec3(0.0f, 0.0f, 0.0f);}
    float getMass() const {return isStatic() ? std::numeric_limits<float>::infinity() : mass;}
    float getDensity() const {return getMass() / collider->getVolume();}
    Collider* getCollider() const {return collider;}
    PhysicsMaterial getPhysicsMaterial() const {return material;}

    RigidBody& setLayer(int layer) {this->layer = layer; return *this;}
    RigidBody& setStatic(bool staticBody) {this->staticBody = staticBody; return *this;}
    RigidBody& setPosition(const glm::vec3 position) {this->position = position; return *this;}
    RigidBody& setVelocity(const glm::vec3 velocity) {this->velocity = velocity; return *this;}
    RigidBody& setRotation(const glm::quat& rotation) {this->rotation = rotation; return *this;}
    RigidBody& setAngularVelocity(const glm::vec3& angularVelocity) {this->angularVelocity = angularVelocity; return *this;}
    RigidBody& setMass(float mass) {this->mass = mass; return *this;}
    RigidBody& setDensity(float density) {mass = density * collider->getVolume(); return *this;}
    RigidBody& setMaterial(const PhysicsMaterial material) {this->material = material; return *this;}

    RigidBody& applyImpulse(const glm::vec3& impulsePosition, const glm::mat3& inverseInertia, const glm::vec3& impulse);

    bool isStatic() const {return staticBody;}
    bool canRotate() const {return !(fixedRotation || staticBody);}

private:

    Collider* collider;
    float mass;
    PhysicsMaterial material;
    int layer;
    bool staticBody;

    glm::vec3 position;
    glm::vec3 velocity;

    bool fixedRotation;

    glm::quat rotation;
    glm::vec3 angularVelocity;

    glm::vec3 getAccelerationAtPosition(const std::vector<ForceField::Ref>& forceFields) const;

    void integrateMotion(const std::vector<ForceField::Ref>& forceFields, float delta);

};

class Trigger {
public:

    using TriggerEvent = std::function<void(RigidBody::Ref& rigidBody, float delta)>;

    using Ref = std::shared_ptr<Trigger>;

    template <typename... Args>
    static Trigger::Ref make(Args&&... args) {
        return std::make_shared<Trigger>(std::forward<Args>(args)...);
    }

    static TriggerEvent nullEvent() {
        return [](RigidBody::Ref& rigidBody, float delta) {};
    }

    Trigger (
        Collider* collider,
        glm::vec3 position = {},
        glm::quat rotation = glm::identity<glm::quat>(),
        const TriggerEvent& onEnterEvent = nullEvent(),
        const TriggerEvent& onExitEvent = nullEvent(),
        const TriggerEvent& onTickEvent = nullEvent()
    );

    ~Trigger();

    Collider* getCollider() const {return collider;}
    glm::vec3 getPosition() const {return position;}
    glm::quat getRotation() const {return rotation;}

    void setPosition(const glm::vec3& position) {this->position = position;}
    void setRotation(const glm::quat& rotation) {this->rotation = rotation;}
    void registerOnEnterEvent(const TriggerEvent& onEnterEvent) {this->onEnterEvent = onEnterEvent;}
    void registerOnExitEvent(const TriggerEvent& onExitEvent) {this->onExitEvent = onExitEvent;}
    void registerOnTickEvent(const TriggerEvent& onTickEvent) {this->onTickEvent = onTickEvent;}

    void triggerOnEnterEvent(RigidBody::Ref& rigidBody, float delta) {onEnterEvent(rigidBody, delta);};
    void triggerOnExitEvent(RigidBody::Ref& rigidBody, float delta) {onExitEvent(rigidBody, delta);};
    void triggerOnTickEvent(RigidBody::Ref& rigidBody, float delta) {onTickEvent(rigidBody, delta);};

private:

    Collider* collider;

    glm::vec3 position;
    glm::quat rotation;

    TriggerEvent onEnterEvent;

    TriggerEvent onExitEvent;

    TriggerEvent onTickEvent;
};

class PhysicsEngine {
public:

    using CollidingPair = std::pair<RigidBody::Ref, RigidBody::Ref>;

    void tick(float delta);

    void addRigidBody(RigidBody::Ref& rigidBody);

    void addForceField(ForceField::Ref& forceField);

    void addTrigger(Trigger::Ref& trigger);

    PhysicsEngine& setCollisionRule(const std::vector<int>& collisionLayers);
    PhysicsEngine& setNoCollisionRule(const std::vector<int>& collisionLayers);
    bool doLayersCollide(int collisionLayer1, int collisionLayer2) const;

private:

    std::vector<RigidBody::Ref> rigidBodies;

    std::vector<ForceField::Ref> forceFields;

    std::vector<Trigger::Ref> triggers;

    // todo rework (indices may change over time)
    std::unordered_map<std::size_t, std::set<std::size_t>> functionalTriggers;
    
    std::unordered_map<int, std::set<int>> collisionRules;

    std::vector<CollidingPair> sweepAndPrune() const;

    std::vector<CollidingPair> getPairs(bool continuous) const;

    std::vector<ForceField::Ref> getEffectiveForceFields(const RigidBody::Ref& rigidBody) const;

    void triggerEvents(float delta);

};