#include "Physics.hpp"

#include <glm/gtx/norm.hpp>
#include <glm/gtx/matrix_operation.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <algorithm>
#include <iostream>

#include "Scene.hpp"
#include "Mesh.hpp"

extern Mesh* globalEarthMesh;
extern Scene* scenePointer;

void PhysicsEngine::tick(float delta) {
    for (RigidBody::Ref& rigidBody : rigidBodies) {
        rigidBody->tick(getEffectiveForceFields(rigidBody), delta);
    }

    // todo modifier
    constexpr bool continuous{false};

    for (const CollidingPair& pair : getPairs(continuous)) {
        const geometry::DiscreteIntersectionInfo intersectionInfo = pair.first->getCollider()->getCollisionInfo(pair.first->getPosition(), pair.second->getPosition(), pair.first->getRotation(), pair.second->getRotation(), pair.second->getCollider(), continuous);
        if (intersectionInfo.intersects()) {
            if (!continuous) {

                RigidBody::Ref body1 = pair.first;
                RigidBody::Ref body2 = pair.second;
                // Collision discrète
                float massRatio;
                if (body1->isStatic()) {
                    massRatio = 0.0f;
                }
                else if (body2->isStatic()) {
                    massRatio = 1.0f;
                }
                else {
                    massRatio = body1->getMass() / (body1->getMass() + body2->getMass());
                }
                body1->setPosition(body1->getPosition() + intersectionInfo.getPenetration() * intersectionInfo.getNormal() * massRatio);
                body2->setPosition(body2->getPosition() - intersectionInfo.getPenetration() * intersectionInfo.getNormal() * (1.0f - massRatio));
                
                const glm::vec3 collisionPoint = intersectionInfo.getIntersectionPoint() + intersectionInfo.getNormal() * intersectionInfo.getPenetration() * (massRatio - 0.5f);

                const glm::vec3 velocity1 = body1->getVelocity() + glm::cross(body1->getAngularVelocity(), collisionPoint - body1->getPosition());
                const glm::vec3 velocity2 = body2->getVelocity() + glm::cross(body2->getAngularVelocity(), collisionPoint - body2->getPosition());

                const glm::vec3 relativeVelocity = velocity2 - velocity1;

                const float normalVelocity = glm::dot(relativeVelocity, intersectionInfo.getNormal());

                const float restitution = body1->getPhysicsMaterial().restitution + body2->getPhysicsMaterial().restitution;
                
                const glm::vec3 relativeCollisionPoint1 = collisionPoint - body1->getPosition();
                const glm::vec3 relativeCollisionPoint2 = collisionPoint - body2->getPosition();

                const float invMasses = 1.0f / body1->getMass() + 1.0f / body2->getMass();

                const glm::mat3 invI1 = body1->isStatic() ? glm::mat3(0.0f) : glm::inverse(/*geometry::rotateInertiaTensor(*/body1->getMass() * body1->getCollider()->getInertiaTensor()/*, body1->getRotation())*/);
                const glm::mat3 invI2 = body2->isStatic() ? glm::mat3(0.0f) : glm::inverse(/*geometry::rotateInertiaTensor(*/body2->getMass() * body2->getCollider()->getInertiaTensor()/*, body2->getRotation())*/);
                
                const glm::vec3 theta1 = body1->canRotate() ? glm::cross(invI1 * glm::cross(relativeCollisionPoint1, intersectionInfo.getNormal()), relativeCollisionPoint1) : glm::vec3(0.0f, 0.0f, 0.0f);
                const glm::vec3 theta2 = body2->canRotate() ? glm::cross(invI2 * glm::cross(relativeCollisionPoint2, intersectionInfo.getNormal()), relativeCollisionPoint2) : glm::vec3(0.0f, 0.0f, 0.0f);

                const float jn = -(1.0f + restitution) * normalVelocity / (invMasses + glm::dot(theta1 + theta2, intersectionInfo.getNormal()));
                //const float j = -(1.0f + restitution) * normalVelocity / invMasses;

                const glm::vec3 impulse = jn * intersectionInfo.getNormal();

                body1->applyImpulse(relativeCollisionPoint1, invI1, impulse);
                body2->applyImpulse(relativeCollisionPoint2, invI2, -impulse);

                /* Friction */
                /*const float friction = body1->getPhysicsMaterial().friction * body2->getPhysicsMaterial().friction;

                const glm::vec3 tangentVelocity = relativeVelocity - glm::dot(relativeVelocity, intersectionInfo.getNormal()) * intersectionInfo.getNormal();
                const glm::vec3 tangent = glm::normalize(tangentVelocity);
                //scenePointer->getRootTransformTree()->addChild({collisionPoint, glm::identity<glm::quat>(), glm::vec3(0.4f)})->addObject(globalEarthMesh);
                //scenePointer->getRootTransformTree()->addChild({collisionPoint + tangent, glm::identity<glm::quat>(), glm::vec3(0.2f)})->addObject(globalEarthMesh);

                const glm::vec3 thetaTangent1 = body1->canRotate() ? glm::cross(invI1 * glm::cross(relativeCollisionPoint1, tangent), tangent) : glm::vec3(0.0f, 0.0f, 0.0f);
                const glm::vec3 thetaTangent2 = body2->canRotate() ? glm::cross(invI2 * glm::cross(relativeCollisionPoint1, tangent), tangent) : glm::vec3(0.0f, 0.0f, 0.0f);

                const float jr = std::abs(jn);
                const float jt1 = std::clamp(friction * glm::dot(tangentVelocity, tangent) / glm::dot(relativeCollisionPoint1, thetaTangent1), -jr, jr);
                const float jt2 = std::clamp(friction * glm::dot(tangentVelocity, tangent) / glm::dot(relativeCollisionPoint2, thetaTangent2), -jr, jr);

                body1->applyImpulse(relativeCollisionPoint1, invI1, -jt1 * tangent);
                body2->applyImpulse(relativeCollisionPoint2, invI2, jt2 * tangent);*/
                /*
                
                glm::vec3 tangentVelocity{relativeVelocity - glm::dot(relativeVelocity, normal) * normal};
                glm::vec3 tangent{glm::normalize(tangentVelocity)};

                glm::vec3 thetaTangent{glm::cross(invInertia * glm::cross(relativeCollisionPoint, tangent), tangent)};
                float jt{glm::dot(tangentVelocity, tangent) / glm::dot(relativeCollisionPoint, thetaTangent)};

                jt = std::clamp(friction * jt, -jr, jr);

                glm::vec3 tangentImpulse{jt * tangent};
                
                */

                /*const float friction = body1->getPhysicsMaterial().friction * pair.second->getPhysicsMaterial().friction;
                const glm::vec3 tangent = glm::normalize(relativeVelocity - glm::dot(relativeVelocity, intersectionInfo.normal) * intersectionInfo.normal);

                const float frictionMagnitude = friction * glm::length(impulse);

                // todo friction statique (qui bloque les objets pentus sur place)
                float staticFrictionForce = staticFrictionCoefficient * glm::length(impulse);
                if (frictionMagnitude > staticFrictionForce) {
                    frictionMagnitude = staticFrictionForce;
                }

                glm::vec3 frictionForce = -frictionMagnitude * tangent;

                body1->setVelocity(body1->getVelocity() + frictionForce / body1->getMass());
                body2->setVelocity(body2->getVelocity() - frictionForce / body2->getMass());

                if (body1->canRotate()) {
                    glm::vec3 leverArm = relativeCollisionPoint1 - glm::dot(relativeCollisionPoint1, intersectionInfo.normal) * intersectionInfo.normal;
                    glm::vec3 torque = glm::cross(leverArm, frictionForce);
                    body1->setAngularVelocity(body1->getAngularVelocity() + invI1 * torque);
                }

                if (body2->canRotate()) {
                    glm::vec3 leverArm = relativeCollisionPoint2 - glm::dot(relativeCollisionPoint2, intersectionInfo.normal) * intersectionInfo.normal;
                    glm::vec3 torque = glm::cross(leverArm, frictionForce);
                    body2->setAngularVelocity(body2->getAngularVelocity() - invI2 * torque);
                }*/
            }
        }
    }

    triggerEvents(delta);
}

void PhysicsEngine::addRigidBody(RigidBody::Ref& rigidBody) {
    rigidBodies.push_back(rigidBody);
}

void PhysicsEngine::addForceField(ForceField::Ref& forceField) {
    forceFields.push_back(forceField);
}

void PhysicsEngine::addTrigger(Trigger::Ref& trigger) {
    triggers.push_back(trigger);
}

PhysicsEngine& PhysicsEngine::setCollisionRule(const std::vector<int>& collisionLayers) {
    for (int collisionLayer1 : collisionLayers) {
        for (int collisionLayer2 : collisionLayers) {
            if (collisionLayer1 != collisionLayer2) {
                collisionRules[collisionLayer1].insert(collisionLayer2);
            }
        }
    }
    return *this;
}

PhysicsEngine& PhysicsEngine::setNoCollisionRule(const std::vector<int>& collisionLayers) {
    for (int collisionLayer1 : collisionLayers) {
        for (int collisionLayer2 : collisionLayers) {
            if (collisionLayer1 != collisionLayer2) {
                collisionRules[collisionLayer1].erase(collisionLayer2);
            }
        }
    }
    return *this;
}

bool PhysicsEngine::doLayersCollide(int collisionLayer1, int collisionLayer2) const {
    return collisionLayer1 == collisionLayer2 || (collisionRules.find(collisionLayer1) != collisionRules.end() && collisionRules.at(collisionLayer1).contains(collisionLayer2));
}

std::vector<PhysicsEngine::CollidingPair> PhysicsEngine::sweepAndPrune() const {
    std::vector<glm::vec3> minExtents(rigidBodies.size());
    std::vector<glm::vec3> maxExtents(rigidBodies.size());
    std::vector<std::size_t> xSortedPositions(rigidBodies.size());
    for (std::size_t i = 0; i < rigidBodies.size(); ++i) {
        std::tie(minExtents[i], maxExtents[i]) = rigidBodies[i]->getCollider()->getBoundingBox(rigidBodies[i]->getPosition(), rigidBodies[i]->getRotation());
        xSortedPositions[i] = i;
    }
    std::sort(xSortedPositions.begin(), xSortedPositions.end(), [&](std::size_t bodyIndex1, std::size_t bodyIndex2) {
        return minExtents[bodyIndex1].x > minExtents[bodyIndex2].x;
    });
    std::vector<PhysicsEngine::CollidingPair> potentialCollidingPairs;
    for (std::size_t i = 0; i < rigidBodies.size() - 1; ++i) {
        std::size_t j = i + 1;
        bool checkNextBoxes{true};
        do {
            if (maxExtents[xSortedPositions[i]].x >= minExtents[xSortedPositions[j]].x) {
                if ((maxExtents[i].y >= minExtents[j].y && maxExtents[j].y >= minExtents[i].y) && (maxExtents[i].z >= minExtents[j].z && maxExtents[j].z >= minExtents[i].z)) {
                    potentialCollidingPairs.push_back(std::make_pair(rigidBodies[i], rigidBodies[j]));
                }
                ++j;
            }
            else {
                checkNextBoxes = false;
            }
        } while (checkNextBoxes && j < rigidBodies.size());
    }
    return potentialCollidingPairs;
}

std::vector<PhysicsEngine::CollidingPair> PhysicsEngine::getPairs(bool continuous) const {
    std::vector<PhysicsEngine::CollidingPair> collidingPairs;
    /*for (std::size_t i = 0; i < rigidBodies.size(); ++i) {
        for (std::size_t j = i + 1; j < rigidBodies.size(); ++j) {
            if (!(rigidBodies[i]->isStatic() && rigidBodies[j]->isStatic()) && doLayersCollide(rigidBodies[i]->getLayer(), rigidBodies[j]->getLayer())) {
                collidingPairs.push_back(std::make_pair(rigidBodies[i], rigidBodies[j]));
            }
        }
    }*/
    for (PhysicsEngine::CollidingPair& collidingPair : sweepAndPrune()) {
        auto [body1, body2] = collidingPair;
        if (!(body1->isStatic() && body2->isStatic()) && doLayersCollide(body1->getLayer(), body2->getLayer())) {
            collidingPairs.push_back(collidingPair);
        }
    }
    return collidingPairs;
}

std::vector<ForceField::Ref> PhysicsEngine::getEffectiveForceFields(const RigidBody::Ref& rigidBody) const {
    std::vector<ForceField::Ref> effectiveForceFields;
    for (const ForceField::Ref& forceField : forceFields) {
        if (forceField->isGlobal() || rigidBody->getCollider()->getCollisionInfo(rigidBody->getPosition(), forceField->getPosition(), rigidBody->getRotation(), forceField->getRotation(), forceField->getCollider(), false).intersects()) {
            effectiveForceFields.push_back(forceField);
        }
    }
    return effectiveForceFields;
}

void PhysicsEngine::triggerEvents(float delta) {
    for (std::size_t i = 0; i < rigidBodies.size(); ++i) {
        for (std::size_t j = 0; j < triggers.size(); ++j) {
            RigidBody::Ref rigidBody = rigidBodies[i];
            Trigger::Ref trigger = triggers[j];
            if (trigger->getCollider()->getCollisionInfo(trigger->getPosition(), rigidBody->getPosition(), trigger->getRotation(), rigidBody->getRotation(), rigidBody->getCollider(), false).intersects()) {
                if (functionalTriggers[j].contains(i)) {
                    trigger->triggerOnTickEvent(rigidBody, delta);
                }
                else {
                    functionalTriggers[j].insert(i);
                    trigger->triggerOnEnterEvent(rigidBody, delta);
                }
            }
            else if (functionalTriggers[j].contains(i)) {
                functionalTriggers[j].erase(i);
                trigger->triggerOnExitEvent(rigidBody, delta);
            }
        }
    }
}

PhysicsMaterial::PhysicsMaterial(
    float friction,
    float restitution,
    float linearDamping,
    float angularDamping
) :
    friction(friction),
    restitution(restitution),
    linearDamping(linearDamping),
    angularDamping(angularDamping)
{
}

PhysicsMaterial::~PhysicsMaterial() {
}

/*************/
/* RigidBody */
/*************/

RigidBody::RigidBody(
    Collider* collider,
    float mass,
    PhysicsMaterial material,
    int layer,
    bool staticBody,
    const glm::vec3& position,
    const glm::vec3& velocity,
    bool fixedRotation,
    const glm::quat& rotation,
    const glm::vec3& angularVelocity
    
) :
    collider(collider),
    mass(mass),
    material(material),
    layer(layer),
    staticBody(staticBody),
    position(position),
    velocity(velocity),
    fixedRotation(fixedRotation),
    rotation(rotation),
    angularVelocity(angularVelocity)
{
}

RigidBody::~RigidBody() {
}

void RigidBody::tick(const std::vector<ForceField::Ref>& forceFields, float delta) {
    if (!isStatic()) {
        glm::vec3 additionalAcceleration = glm::vec3(0.0f);
        for (const ForceField::Ref& forceField : forceFields) {
            additionalAcceleration += forceField->getAcceleration(position);
        }
        integrateMotion(forceFields, delta);

        if (canRotate()) {
            rotation = glm::quat{delta * angularVelocity} * rotation;
        }

        if (material.linearDamping > 0) {
            velocity *= std::min(1.0f - material.linearDamping * delta, 1.0f);
        }

        if (material.angularDamping > 0) {
            angularVelocity *= std::min(1.0f - material.angularDamping * delta, 1.0f);
        }
    }
}

RigidBody& RigidBody::applyImpulse(const glm::vec3& impulsePosition, const glm::mat3& inverseInertia, const glm::vec3& impulse) {
    if (!isStatic()) {
        velocity -= impulse / mass;
        if (canRotate()) {
            angularVelocity -= inverseInertia * glm::cross(impulsePosition, impulse);
        }
    }
    return *this;
}

glm::vec3 RigidBody::getAccelerationAtPosition(const std::vector<ForceField::Ref>& forceFields) const {
    glm::vec3 acceleration = glm::vec3(0.0f);
    for (const ForceField::Ref& forceField : forceFields) {
        acceleration += forceField->getAcceleration(position);
    }
    return acceleration;
}

void RigidBody::integrateMotion(const std::vector<ForceField::Ref>& forceFields, float delta) {
    glm::vec3 acceleration{getAccelerationAtPosition(forceFields)};

    position += velocity * delta + 0.5f * acceleration * delta * delta;
    
    glm::vec3 newAcceleration{getAccelerationAtPosition(forceFields)};
        
    velocity += 0.5f * (acceleration + newAcceleration) * delta;
}

geometry::DiscreteIntersectionInfo Collider::getCollisionInfo(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const Collider* collider, bool continuous) {
    switch (collider->getColliderType()) {
        case ColliderType::OBB :
            return getCollisionInfoOBB(position, positionOther, rotation, rotationOther, static_cast<const OBBCollider*>(collider), continuous);
        case ColliderType::SPHERE :
            return getCollisionInfoSphere(position, positionOther, rotation, rotationOther, static_cast<const SphereCollider*>(collider), continuous);
        case ColliderType::CAPSULE :
            return getCollisionInfoCapsule(position, positionOther, rotation, rotationOther, static_cast<const CapsuleCollider*>(collider), continuous);
        case ColliderType::CONVEX :
            return getCollisionInfoConvex(position, positionOther, rotation, rotationOther, static_cast<const ConvexCollider*>(collider), continuous);
        default:
            return geometry::DiscreteIntersectionInfo::none();
    }
}

/**************/
/* ForceField */
/**************/

ForceField::ForceField(
    Collider* collider,
    glm::vec3 position,
    glm::quat rotation,
    const ForceFieldFunction& forceFieldFunction
) :
    collider(collider),
    position(position),
    rotation(rotation),
    forceFieldFunction(forceFieldFunction)
{}

ForceField::~ForceField() {
}

/***********/
/* Trigger */
/***********/

Trigger::Trigger (
    Collider* collider,
    glm::vec3 position,
    glm::quat rotation,
    const TriggerEvent& onEnterEvent,
    const TriggerEvent& onExitEvent,
    const TriggerEvent& onTickEvent
) :
    collider(collider),
    position(position),
    rotation(rotation),
    onEnterEvent(onEnterEvent),
    onExitEvent(onExitEvent),
    onTickEvent(onTickEvent)
{}

Trigger::~Trigger() {
}

/*******/
/* OBB */
/*******/

OBBCollider::OBBCollider(const glm::vec3& halfDimensions) :
    size(halfDimensions)
{}

glm::mat3 OBBCollider::getInertiaTensor() const {
    return geometry::boxInertiaTensor(size);
}

float OBBCollider::getVolume() const {
    return geometry::boxVolume(size);
}

geometry::DiscreteIntersectionInfo OBBCollider::getCollisionInfoOBB(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const OBBCollider* collider, bool continuous) const {
    return geometry::boxBoxIntersection(position, rotation, size, positionOther, rotationOther, collider->size);
}

geometry::DiscreteIntersectionInfo OBBCollider::getCollisionInfoSphere(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const SphereCollider* collider, bool continuous) const {
    return geometry::boxSphereIntersection(position, rotation, size, positionOther, collider->radius);
}

geometry::DiscreteIntersectionInfo OBBCollider::getCollisionInfoCapsule(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const CapsuleCollider* collider, bool continuous) const {
    return geometry::boxCapsuleIntersection(position, rotation, size, positionOther, rotationOther, collider->halfLength, collider->radius);
}

geometry::DiscreteIntersectionInfo OBBCollider::getCollisionInfoConvex(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const ConvexCollider* collider, bool continuous) const {
    return geometry::convexConvexIntersection(
        position, rotation, geometry::getBoxVertices(size),
        positionOther, rotationOther, collider->vertices
    );
}

geometry::BoundingBox OBBCollider::getBoundingBox(const glm::vec3& position, const glm::quat& rotation) const {
    return geometry::convexSetBoundingBox(geometry::transformPoints(geometry::getBoxVertices(size), position, rotation));
}

/**********/
/* Sphere */
/**********/

SphereCollider::SphereCollider(float radius) :
    radius(radius)
{}

glm::mat3 SphereCollider::getInertiaTensor() const {
    return geometry::sphereInertiaTensor(radius);
}

float SphereCollider::getVolume() const {
    return geometry::sphereVolume(radius);
}

geometry::DiscreteIntersectionInfo SphereCollider::getCollisionInfoOBB(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const OBBCollider* collider, bool continuous) const {
    return geometry::boxSphereIntersection(positionOther, rotationOther, collider->size, position, radius).getInverse();
}

geometry::DiscreteIntersectionInfo SphereCollider::getCollisionInfoSphere(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const SphereCollider* collider, bool continuous) const {
    return geometry::sphereSphereIntersection(position, radius, positionOther, collider->radius);
}

geometry::DiscreteIntersectionInfo SphereCollider::getCollisionInfoCapsule(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const CapsuleCollider* collider, bool continuous) const {
    return geometry::sphereCapsuleIntersection(position, radius, positionOther, rotationOther, collider->halfLength, collider->radius);
}

geometry::DiscreteIntersectionInfo SphereCollider::getCollisionInfoConvex(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const ConvexCollider* collider, bool continuous) const {
    return geometry::sphereConvexIntersection(position, radius, positionOther, rotationOther, collider->vertices);
}

geometry::BoundingBox SphereCollider::getBoundingBox(const glm::vec3& position, const glm::quat& rotation) const {
    const glm::vec3 radiusVector = glm::vec3(radius);
    return std::make_pair(
        position - radiusVector,
        position + radiusVector
    );
}

/***********/
/* Capsule */
/***********/

CapsuleCollider::CapsuleCollider(float halfLength, float radius) :
    halfLength(halfLength),
    radius(radius)
{}


glm::mat3 CapsuleCollider::getInertiaTensor() const {
    return geometry::capsuleInertiaTensor(radius, halfLength);
    //return geometry::boxInertiaTensor(glm::vec3(radius, halfLength, radius));
}

float CapsuleCollider::getVolume() const {
    return geometry::capsuleVolume(radius, halfLength);
}

geometry::DiscreteIntersectionInfo CapsuleCollider::getCollisionInfoOBB(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const OBBCollider* collider, bool continuous) const {
    return geometry::boxCapsuleIntersection(positionOther, rotationOther, collider->size, position, rotation, halfLength, radius).getInverse();
}

geometry::DiscreteIntersectionInfo CapsuleCollider::getCollisionInfoSphere(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const SphereCollider* collider, bool continuous) const {
    return geometry::sphereCapsuleIntersection(positionOther, collider->radius, position, rotation, halfLength, radius).getInverse();
}

geometry::DiscreteIntersectionInfo CapsuleCollider::getCollisionInfoCapsule(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const CapsuleCollider* collider, bool continuous) const {
    return geometry::capsuleCapsuleIntersection(position, rotation, halfLength, radius, positionOther, rotationOther, collider->halfLength, collider->radius);
}

geometry::DiscreteIntersectionInfo CapsuleCollider::getCollisionInfoConvex(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const ConvexCollider* collider, bool continuous) const {
    return geometry::capsuleConvexIntersection(position, rotation, halfLength, radius, positionOther, rotationOther, collider->vertices);
}

geometry::BoundingBox CapsuleCollider::getBoundingBox(const glm::vec3& position, const glm::quat& rotation) const {
    auto [capsuleCenter1, capsuleCenter2] = geometry::capsuleHemispheres(position, rotation, halfLength);
    return geometry::capsuleBoundingBox(capsuleCenter1, capsuleCenter2, radius);
}

/**********/
/* Convex */
/**********/

ConvexCollider::ConvexCollider(
    const std::vector<glm::vec3>& vertices
) :
    vertices(vertices)
{}

glm::mat3 ConvexCollider::getInertiaTensor() const {
    return geometry::convexSetInertiaTensor(vertices);
}

float ConvexCollider::getVolume() const {
    return geometry::convexSetVolume(vertices);
}

geometry::DiscreteIntersectionInfo ConvexCollider::getCollisionInfoOBB(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const OBBCollider* collider, bool continuous) const {
    return geometry::convexConvexIntersection(
        position, rotation, vertices,
        positionOther, rotationOther, geometry::getBoxVertices(collider->size)
    );
}

geometry::DiscreteIntersectionInfo ConvexCollider::getCollisionInfoSphere(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const SphereCollider* collider, bool continuous) const {
    return geometry::sphereConvexIntersection(positionOther, collider->radius, position, rotation, vertices).getInverse();
}

geometry::DiscreteIntersectionInfo ConvexCollider::getCollisionInfoCapsule(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const CapsuleCollider* collider, bool continuous) const {
    return geometry::capsuleConvexIntersection(positionOther, rotationOther, collider->halfLength, collider->radius, position, rotation, vertices).getInverse();
}

geometry::DiscreteIntersectionInfo ConvexCollider::getCollisionInfoConvex(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const ConvexCollider* collider, bool continuous) const {
    return geometry::convexConvexIntersection(
        position, rotation, vertices,
        positionOther, rotationOther, collider->vertices
    );
}

geometry::BoundingBox ConvexCollider::getBoundingBox(const glm::vec3& position, const glm::quat& rotation) const {
    return geometry::convexSetBoundingBox(geometry::transformPoints(vertices, position, rotation));
}