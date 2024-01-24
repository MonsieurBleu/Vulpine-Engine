#pragma once
#include <vector>
#include <functional>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <set>

#include "Globals.hpp"

using namespace glm;

class RigidBody;

class Collider;

#define RigidBodyRef std::shared_ptr<RigidBody>
#define newRigidBody std::make_shared<RigidBody>

extern std::mutex physicsMutex;

struct Collision
{
    RigidBodyRef body1;
    RigidBodyRef body2;
    vec3 normal;
    float penetration;

    bool operator==(const Collision &other) const
    {
        return (body1 == other.body1 && body2 == other.body2) || (body1 == other.body2 && body2 == other.body1);
    }

    bool operator<(const Collision &other) const
    {
        return (body1 < other.body1) || (body1 == other.body1 && body2 < other.body2);
    }
};

struct PhysicsMaterial
{
    float restitution;
    float friction;
    float damping;
    float angularDamping;

    PhysicsMaterial(float restitution = 0.5f, float friction = 0.1f, float damping = 0.5f, float angularDamping = 0.5f) : restitution(restitution), friction(friction), damping(damping), angularDamping(angularDamping) {}
    ~PhysicsMaterial() {}
};

class PhysicsEngine
{
private:
    std::vector<RigidBodyRef> bodies;
    std::set<Collision> collisionEvents;

public:
    PhysicsEngine();
    ~PhysicsEngine();

    void addObject(RigidBodyRef body);
    void removeObject(RigidBodyRef body);

    void update(float deltaTime);
};

struct Ray
{
    vec3 origin;
    vec3 direction;
};

bool raycast(Ray ray, std::vector<RigidBodyRef> bodies, float maxLen, float &t, RigidBodyRef &body);

class RigidBody
{
private:
    vec3 position;
    vec3 velocity;
    vec3 acceleration;

    quat rotation;
    vec3 angularVelocity;
    vec3 torque;

    float mass;

    Collider *collider;

    bool enableGravity;

    PhysicsMaterial material;

    bool isStatic = false;

public:
    bool enabled = true;

    const int ID;
    static int IDCounter;

    static vec3 gravity;

    RigidBody(vec3 position = vec3(0.0f), vec3 velocity = vec3(0.0f), quat rotation = vec3(0.0f), vec3 angularVelocity = vec3(0.0f), Collider *collider = nullptr, PhysicsMaterial mat = PhysicsMaterial(), float mass = 1.0f, bool enableGravity = true);
    ~RigidBody();

    void update(float deltaTime, std::vector<RigidBodyRef> bodies, std::vector<Collision> &collisionEvents);

    vec3 getPosition() const { return position; };
    vec3 getVelocity() const { return velocity; };
    vec3 getAcceleration() const { return acceleration; };
    quat getRotation() const { return rotation; };
    vec3 getAngularVelocity() const { return angularVelocity; };
    vec3 getTorque() const { return torque; };
    PhysicsMaterial getMaterial() const { return material; };
    float getMass() const { return mass; };
    bool getIsStatic() const { return isStatic; };
    bool getGravity() const { return enableGravity; };

    Collider *getCollider() const { return collider; };

    void setPosition(vec3 _position) { position = _position; };
    void setVelocity(vec3 _velocity) { velocity = _velocity; };
    void setAcceleration(vec3 _acceleration) { acceleration = _acceleration; };
    void setRotation(quat _rotation) { rotation = _rotation; };
    void setAngularVelocity(vec3 _angularVelocity) { angularVelocity = _angularVelocity; };
    void setTorque(vec3 _torque) { torque = _torque; };
    void setMass(float _mass) { mass = _mass; };
    void setMaterial(PhysicsMaterial _material) { material = _material; };
    void setIsStatic(bool _isStatic) { isStatic = _isStatic; };
    void setGravity(bool _enableGravity) { enableGravity = _enableGravity; };

    friend class PhysicsEngine;
    friend bool raycast(Ray ray, std::vector<RigidBodyRef> bodies, float maxLen, float &t, RigidBodyRef &body);
};

enum ColliderType
{
    SPHERE, // Sphere collider
    AABB,   // Axis-aligned bounding box
    OBB,    // Oriented bounding box
    MESH    // Mesh collider
};

class Collider
{
private:
    ColliderType type;

public:
    Collider(ColliderType type);
    ~Collider();

    ColliderType getType() { return type; };

    virtual bool checkCollision(Collider *other, vec3 positionSelf, vec3 positionOther, float &penetration, vec3 &normal) const = 0;
    virtual bool raycast(Ray ray, vec3 positionSelf, float maxLen, float &t) const = 0;
};

class SphereCollider;
class AABBCollider;
class OBBCollider;
class MeshCollider;

class SphereCollider : public Collider
{
private:
    float radius;
    vec3 center;

public:
    SphereCollider(float radius, vec3 center = vec3(0.f));
    ~SphereCollider();

    float getRadius() const { return radius; };
    vec3 getCenter() const { return center; };
    void setRadius(float _radius) { radius = _radius; };
    void setCenter(vec3 _center) { center = _center; };

    bool checkCollision(Collider *other, vec3 positionSelf, vec3 positionOther, float &penetration, vec3 &normal) const override;

    bool checkCollisionSphere(SphereCollider *other, vec3 positionSelf, vec3 positionOther, float &penetration, vec3 &normal) const;
    bool checkCollisionAABB(AABBCollider *other, vec3 positionSelf, vec3 positionOther, float &penetration, vec3 &normal) const;
    bool checkCollisionOBB(OBBCollider *other, vec3 positionSelf, vec3 positionOther, float &penetration, vec3 &normal) const;
    bool checkCollisionMesh(MeshCollider *other, vec3 positionSelf, vec3 positionOther, float &penetration, vec3 &normal) const;

    bool raycast(Ray ray, vec3 positionSelf, float maxLen, float &t) const override;

    friend class AABBCollider;
};

class AABBCollider : public Collider
{
private:
    vec3 min;
    vec3 max;

public:
    AABBCollider(vec3 min, vec3 max);
    ~AABBCollider();

    vec3 getMin() const { return min; };
    vec3 getMax() const { return max; };
    void setMin(vec3 _min) { min = _min; };
    void setMax(vec3 _max) { max = _max; };

    bool checkCollision(Collider *other, vec3 positionSelf, vec3 positionOther, float &penetration, vec3 &normal) const override;

    bool checkCollisionSphere(SphereCollider *other, vec3 positionSelf, vec3 positionOther, float &penetration, vec3 &normal) const;
    bool checkCollisionAABB(AABBCollider *other, vec3 positionSelf, vec3 positionOther, float &penetration, vec3 &normal) const;
    bool checkCollisionOBB(OBBCollider *other, vec3 positionSelf, vec3 positionOther, float &penetration, vec3 &normal) const;
    bool checkCollisionMesh(MeshCollider *other, vec3 positionSelf, vec3 positionOther, float &penetration, vec3 &normal) const;

    bool raycast(Ray ray, vec3 positionSelf, float maxLen, float &t) const override;

    friend class SphereCollider;
};
