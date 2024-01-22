#include "PhysicsEngine.hpp"
#include <iostream>

#include <glm/gtx/string_cast.hpp>

int RigidBody::IDCounter = 0;
vec3 RigidBody::gravity = vec3(0.f, -9.81f, 0.f);

PhysicsEngine::PhysicsEngine()
{
}

PhysicsEngine::~PhysicsEngine()
{
}

void PhysicsEngine::addObject(RigidBodyRef body)
{
    bodies.push_back(std::move(body));
}

void PhysicsEngine::removeObject(RigidBodyRef body)
{
    for (size_t i = 0; i < bodies.size(); i++)
    {
        if (bodies[i]->ID == body->ID)
        {
            bodies.erase(bodies.begin() + i);
            return;
        }
    }
}

void PhysicsEngine::update(float deltaTime)
{
    if (globals.enablePhysics)
    {
        for (size_t i = 0; i < bodies.size(); i++)
        {
            std::vector<Collision> newCollisionEvents;
            bodies[i]->update(deltaTime, bodies, newCollisionEvents);

            for (auto collision : newCollisionEvents)
            {
                collisionEvents.insert(collision);
            }
        }

        // handle collision events
        for (auto it = collisionEvents.begin(); it != collisionEvents.end(); it++)
        {
            // resolve collision
            vec3 normal = it->normal;

            vec3 relativeVelocity = it->body1->velocity - it->body2->velocity;
            float normalVelocity = glm::dot(normal, relativeVelocity);

            float e = 0.5f * (it->body1->getMaterial().restitution + it->body2->getMaterial().restitution);

            float j = -(1.f + e) * normalVelocity / (1.f / it->body1->mass + 1.f / it->body2->mass);

            vec3 impulse = j * normal;

            it->body1->velocity += impulse / it->body1->mass;
            it->body2->velocity -= impulse / it->body2->mass;

            std::cout << "penetration: " << it->penetration << std::endl;
            std::cout << "diagonal: " << it->body1->collider->getBoundingBox().getDiagonal() << std::endl;

            it->body1->position += normal * (float)pow(it->penetration, 3) * (it->body1->isStatic ? 0.0f : 1.0f);
            it->body2->position -= normal * (float)pow(it->penetration, 3) * (it->body2->isStatic ? 0.0f : 1.0f);

            vec3 tangent = relativeVelocity - dot(relativeVelocity, normal) * normal;
            tangent = normalize(tangent);
            if (length(tangent) > 0.0001f)
            {
                float friction = 0.5f * (it->body1->getMaterial().friction + it->body2->getMaterial().friction);
                float frictionImpulseMagnitude = glm::min(j * friction, glm::length(relativeVelocity - normalVelocity * normal));
                vec3 frictionImpulse = -frictionImpulseMagnitude * tangent;

                it->body1->velocity += frictionImpulse / it->body1->mass;
                it->body2->velocity -= frictionImpulse / it->body2->mass;
            }
        }

        // clear collision events
        collisionEvents.clear();
    }
}

PhysicsMaterial defaultMaterial;

RigidBody::RigidBody(vec3 position, vec3 velocity, quat rotation, vec3 angularVelocity, Collider *collider, PhysicsMaterial mat, float mass, bool enableGravity) : position(position),
                                                                                                                                                                   velocity(velocity),
                                                                                                                                                                   acceleration(vec3(0.f)),
                                                                                                                                                                   rotation(rotation),
                                                                                                                                                                   angularVelocity(angularVelocity),
                                                                                                                                                                   torque(vec3(0.f)),
                                                                                                                                                                   mass(mass),
                                                                                                                                                                   collider(collider),
                                                                                                                                                                   enableGravity(enableGravity),
                                                                                                                                                                   material(mat),
                                                                                                                                                                   ID(IDCounter++)
{
    if (mass == 0.f)
    {
        isStatic = true;
        this->mass = 1.f;
    }
}

RigidBody::~RigidBody()
{
}

void RigidBody::update(float deltaTime, std::vector<RigidBodyRef> bodies, std::vector<Collision> &collisionEvents)
{
    // position and velocity update
    if (!isStatic)
    {
        if (enableGravity)
        {
            acceleration += gravity;
        }

        velocity += acceleration * deltaTime * (1.0f - material.damping);
        position += velocity * deltaTime;

        // std::cout << "velocity: " << glm::to_string(velocity) << std::endl;

        acceleration = vec3(0.f);

        // rotation and angular velocity update
        angularVelocity += torque * deltaTime * material.angularDamping;
        rotation = glm::normalize(glm::quat(0.f, angularVelocity) * rotation);

        torque = vec3(0.f);
    }

    // do collision detection
    for (size_t i = 0; i < bodies.size(); i++)
    {
        if (bodies[i]->ID == this->ID)
        {
            continue;
        }

        float penetration;
        vec3 normal;
        if (this->collider->checkCollision(bodies[i]->collider, this->position, bodies[i]->position, penetration, normal))
        {
            // normalize the penetration based on the size of the bounding boxes of the objects

            float size1 = this->collider->getBoundingBox().getDiagonal();
            float size2 = bodies[i]->collider->getBoundingBox().getDiagonal();
            float size = std::max(size1, size2);

            penetration *= size1 / size;

            Collision collision{newRigidBody(*this), bodies[i], normal, penetration};
            collision.ID = min(this->ID, bodies[i]->ID);

            // temp solution
            for (int i = 0; i < collisionEvents.size(); i++)
            {
                if (collisionEvents[i].ID == collision.ID)
                {
                    if (collisionEvents[i].penetration < collision.penetration)
                    {
                        collisionEvents[i] = collision;
                    }
                }
                else
                {
                    collisionEvents.push_back(collision);
                }
            }
            if (collisionEvents.size() == 0)
            {
                collisionEvents.push_back(collision);
            }
        }
    }
}

Collider::Collider(ColliderType type, BoundingBox boundingBox) : type(type), boundingBox(boundingBox)
{
}

Collider::~Collider()
{
}

SphereCollider::SphereCollider(float radius, vec3 center) : Collider(ColliderType::SPHERE, {vec3(-radius), vec3(radius)}),
                                                            radius(radius),
                                                            center(center)
{
}

SphereCollider::~SphereCollider()
{
}

bool SphereCollider::checkCollision(Collider *other, vec3 positionSelf, vec3 positionOther, float &penetration, vec3 &normal) const
{
    vec3 selfPosition = positionSelf + this->center;
    vec3 otherPosition = positionOther + ((SphereCollider *)other)->center;
    switch (other->getType())
    {
    case ColliderType::SPHERE:
        return checkCollisionSphere((SphereCollider *)other, selfPosition, otherPosition, penetration, normal);
        break;
    case ColliderType::AABB:
        return checkCollisionAABB((AABBCollider *)other, selfPosition, otherPosition, penetration, normal);
        break;
    // case ColliderType::OBB:
    //     return checkCollisionOBB((OBBCollider *)other, selfPosition, otherPosition, penetration, normal);
    //     break;
    // case ColliderType::MESH:
    //     return checkCollisionMesh((MeshCollider *)other, selfPosition, otherPosition, penetration, normal);
    //     break;
    default:
        return false;
        break;
    }
}

bool SphereCollider::checkCollisionSphere(SphereCollider *other, vec3 positionSelf, vec3 positionOther, float &penetration, vec3 &normal) const
{
    float distance = glm::distance(positionSelf, positionOther);
    penetration = radius + other->radius - distance;
    normal = glm::normalize(positionSelf - positionOther);
    std::cout << "distance: " << distance << std::endl;
    std::cout << "position self: " << glm::to_string(positionSelf) << std::endl;
    std::cout << "position other: " << glm::to_string(positionOther) << std::endl
              << std::endl;

    return penetration > 0.f;
}

AABBCollider::AABBCollider(vec3 min, vec3 max) : Collider(ColliderType::AABB, {min, max}),
                                                 min(min),
                                                 max(max)
{
}

AABBCollider::~AABBCollider()
{
}

bool AABBCollider::checkCollision(Collider *other, vec3 positionSelf, vec3 positionOther, float &penetration, vec3 &normal) const
{
    switch (other->getType())
    {
    case ColliderType::SPHERE:
        return checkCollisionSphere((SphereCollider *)other, positionSelf, positionOther, penetration, normal);
        break;
    case ColliderType::AABB:
        return checkCollisionAABB((AABBCollider *)other, positionSelf, positionOther, penetration, normal);
        break;
    // case ColliderType::OBB:
    //     return checkCollisionOBB((OBBCollider *)other, positionSelf, positionOther, penetration, normal);
    //     break;
    // case ColliderType::MESH:
    //     return checkCollisionMesh((MeshCollider *)other, positionSelf, positionOther, penetration, normal);
    //     break;
    default:
        return false;
        break;
    }
}

// collision detection between sphere and AABB
bool checkCollisionSphereAABB(const SphereCollider *sphere, const AABBCollider *AABB, vec3 positionSphere, vec3 positionAABB, float &penetration, vec3 &normal)
{
    vec3 closestPoint = positionSphere;

    // not ideal
    for (int i = 0; i < 3; ++i)
    {
        float v = positionSphere[i];
        if (v < AABB->getMin()[i] + positionAABB[i])
            v = AABB->getMin()[i] + positionAABB[i];
        if (v > AABB->getMax()[i] + positionAABB[i])
            v = AABB->getMax()[i] + positionAABB[i];
        closestPoint[i] = v;
    }

    vec3 sphereToClosest = closestPoint - positionSphere;
    float distance = glm::length(sphereToClosest);

    bool collision = distance <= sphere->getRadius();

    if (collision)
    {
        penetration = sphere->getRadius() - distance;
        if (distance != 0)
        {
            normal = -glm::normalize(sphereToClosest);
        }
        else
        {
            // If the sphere's center is exactly at the closest point (unlikely, but possible)
            normal = vec3(1, 0, 0); // Default normal, can be any unit vector
        }

        // std::cout << "collision" << std::endl;
        // std::cout << "penetration: " << penetration << std::endl;
        // std::cout << "normal: " << glm::to_string(normal) << std::endl
        //           << std::endl;
    }

    return collision;
}

bool AABBCollider::checkCollisionAABB(AABBCollider *other, vec3 positionSelf, vec3 positionOther, float &penetration, vec3 &normal) const
{
    // Adjust AABBs by their positions
    vec3 minA = this->min + positionSelf;
    vec3 maxA = this->max + positionSelf;
    vec3 minB = other->min + positionOther;
    vec3 maxB = other->max + positionOther;

    // Check for overlap in each axis
    bool collisionX = maxA.x >= minB.x && minA.x <= maxB.x;
    bool collisionY = maxA.y >= minB.y && minA.y <= maxB.y;
    bool collisionZ = maxA.z >= minB.z && minA.z <= maxB.z;

    bool collision = collisionX && collisionY && collisionZ;

    if (collision)
    {
        // Calculate penetration depth and normal
        float penetrationX = std::min(maxA.x - minB.x, maxB.x - minA.x);
        float penetrationY = std::min(maxA.y - minB.y, maxB.y - minA.y);
        float penetrationZ = std::min(maxA.z - minB.z, maxB.z - minA.z);

        // Choose the smallest penetration as the true penetration
        penetration = std::min({penetrationX, penetrationY, penetrationZ});

        // Determine the collision normal based on the smallest penetration
        if (penetration == penetrationX)
        {
            normal = (positionSelf.x < positionOther.x) ? vec3(-1, 0, 0) : vec3(1, 0, 0);
        }
        else if (penetration == penetrationY)
        {
            normal = (positionSelf.y < positionOther.y) ? vec3(0, -1, 0) : vec3(0, 1, 0);
        }
        else
        { // penetrationZ
            normal = (positionSelf.z < positionOther.z) ? vec3(0, 0, -1) : vec3(0, 0, 1);
        }

        // std::cout << "collision" << std::endl;
    }
    else
    {
        // std::cout << std::endl;
        // std::cout << "no AABB collision" << std::endl;
        // std::cout << "minA: " << minA.y << std::endl;
        // std::cout << "maxA: " << maxA.y << std::endl;
        // std::cout << "minB: " << minB.y << std::endl;
        // std::cout << "maxB: " << maxB.y << std::endl;
        // std::cout << "positionSelf: " << glm::to_string(positionSelf) << std::endl;
        // std::cout << "positionOther: " << glm::to_string(positionOther) << std::endl;
        // std::cout << std::endl;
    }

    return collision;
}

bool AABBCollider::checkCollisionSphere(SphereCollider *other, vec3 positionSelf, vec3 positionOther, float &penetration, vec3 &normal) const
{
    return checkCollisionSphereAABB(other, this, positionSelf, positionOther, penetration, normal);
}

bool SphereCollider::checkCollisionAABB(AABBCollider *other, vec3 positionSelf, vec3 positionOther, float &penetration, vec3 &normal) const
{
    return checkCollisionSphereAABB(this, other, positionOther, positionSelf, penetration, normal);
}

bool raycast(Ray ray, std::vector<RigidBodyRef> bodies, float maxLen, float &t, RigidBodyRef &body)
{
    t = std::numeric_limits<float>::infinity();
    body = nullptr;

    for (size_t i = 0; i < bodies.size(); i++)
    {
        float tBody;
        if (bodies[i]->collider->raycast(ray, bodies[i]->position, maxLen, tBody))
        {
            if (tBody < t)
            {
                t = tBody;
                body = bodies[i];
            }
        }
    }

    return body != nullptr;
}

bool SphereCollider::raycast(Ray ray, vec3 positionSelf, float maxLen, float &t) const
{
    vec3 v = ray.origin - (center + positionSelf);

    float a = dot(ray.direction, ray.direction);
    float b = 2 * dot(ray.direction, v);
    float c = dot(v, v) - radius * radius;

    float delta = b * b - 4 * a * c;
    if (delta < 0)
    {
        return false;
    }

    float t1 = (-b - sqrt(delta)) / (2 * a);
    float t2 = (-b + sqrt(delta)) / (2 * a);

    if (t1 < 0.0001f && t2 < 0.0001f)
    {
        return false;
    }

    if (t1 < 0.0001f)
    {
        t = t2;
    }
    else if (t2 < 0.0001f)
    {
        t = t1;
    }
    else
    {
        t = min(t1, t2);
    }

    // vec3 intersectPoint = ray.origin + ray.direction * t;

    // normal = normalize(intersectPoint - center);

    return abs(t) < maxLen;
}

bool AABBCollider::raycast(Ray ray, vec3 positionSelf, float maxLen, float &t) const
{
    float tmin = ((min.x + positionSelf.x) - ray.origin.x) / ray.direction.x;
    float tmax = ((max.x + positionSelf.x) - ray.origin.x) / ray.direction.x;

    if (tmin > tmax)
        std::swap(tmin, tmax);

    float tymin = ((min.y + positionSelf.y) - ray.origin.y) / ray.direction.y;
    float tymax = ((max.y + positionSelf.y) - ray.origin.y) / ray.direction.y;

    if (tymin > tymax)
        std::swap(tymin, tymax);

    if (tmin > tymax || tymin > tmax)
        return false;

    if (tymin > tmin)
        tmin = tymin;

    if (tymax < tmax)
        tmax = tymax;

    float tzmin = ((min.z + positionSelf.z) - ray.origin.z) / ray.direction.z;
    float tzmax = ((max.z + positionSelf.z) - ray.origin.z) / ray.direction.z;

    if (tzmin > tzmax)
        std::swap(tzmin, tzmax);

    if (tmin > tzmax || tzmin > tmax)
        return false;

    t = tmin;

    return abs(t) < maxLen;
}