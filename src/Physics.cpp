// https://www.toptal.com/game/video-game-physics-part-iii-constrained-rigid-body-simulation

#include "Physics.hpp"
#include "Globals.hpp"
#include "Helpers.hpp"

#include <glm/gtx/norm.hpp>
#include <glm/gtx/matrix_operation.hpp>
#include <glm/gtx/string_cast.hpp>

#include <algorithm>
#include <iostream>

void PhysicsEngine::tick(float delta) {
    for (RigidBody::Ref& rigidBody : rigidBodies) {
        rigidBody->tick(getEffectiveForceFields(rigidBody), delta);
    }

    // todo modifier
    constexpr bool continuous{false};

    for (const CollidingPair& pair : getPairs(continuous)) {
        std::optional<IntersectionInfo> intersectionInfoOptional = pair.first->getCollider()->getCollisionInfo(pair.first->getPosition(), pair.second->getPosition(), pair.first->getRotation(), pair.second->getRotation(), pair.second->getCollider(), continuous);
        if (intersectionInfoOptional.has_value()) {
            const IntersectionInfo intersectionInfo = intersectionInfoOptional.value();
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
                body1->setPosition(body1->getPosition() + intersectionInfo.penetration * intersectionInfo.normal * massRatio);
                body2->setPosition(body2->getPosition() - intersectionInfo.penetration * intersectionInfo.normal * (1.0f - massRatio));

                // const glm::vec3 collisionPoint = intersectionInfo.collisionPoint + intersectionInfo.normal * (massRatio - 0.5f);

                const glm::vec3 velocity1 = body1->getVelocity();// + glm::cross(body1->getAngularVelocity(), collisionPoint - body1->getPosition());
                const glm::vec3 velocity2 = body2->getVelocity();// + glm::cross(body2->getAngularVelocity(), collisionPoint - body2->getPosition());

                const glm::vec3 relativeVelocity = velocity2 - velocity1;

                const float normalVelocity = glm::dot(relativeVelocity, intersectionInfo.normal);

                const float restitution = body1->getPhysicsMaterial().restitution + pair.second->getPhysicsMaterial().restitution;

                /*SphereHelperRef sphere(new SphereHelper({1, 1, 0}, 0.2f));
                sphere->state.setPosition(collisionPoint);
                globals.getScene()->add(sphere);*/

                //const glm::vec3 relativeCollisionPoint1 = collisionPoint - body1->getPosition();
                //const glm::vec3 relativeCollisionPoint2 = collisionPoint - body2->getPosition();

                const float invMasses = 1.0f / body1->getMass() + 1.0f / body2->getMass();
                //const glm::mat3 bodyRotation1 = glm::mat3_cast(body1->getRotation());
                //const glm::mat3 bodyRotation2 = glm::mat3_cast(body2->getRotation());

                //const glm::mat3 invI1 = body1->isStatic() ? glm::mat3(0.0f) : glm::inverse(glm::transpose(bodyRotation1) * body1->getCollider()->getInertiaTensor(body1->getMass()) * bodyRotation1);
                //const glm::mat3 invI2 = body2->isStatic() ? glm::mat3(0.0f) : glm::inverse(glm::transpose(bodyRotation2) * body2->getCollider()->getInertiaTensor(body2->getMass()) * bodyRotation2);
                
                //const glm::vec3 theta1 = body1->canRotate() ? glm::cross(invI1 * glm::cross(relativeCollisionPoint1, intersectionInfo.normal), relativeCollisionPoint1) : glm::vec3(0.0f, 0.0f, 0.0f);
                //const glm::vec3 theta2 = body2->canRotate() ? glm::cross(invI2 * glm::cross(relativeCollisionPoint2, intersectionInfo.normal), relativeCollisionPoint2) : glm::vec3(0.0f, 0.0f, 0.0f);

                //const float j = -(1.0f + restitution) * normalVelocity / (invMasses + glm::dot(theta1 + theta2, intersectionInfo.normal));
                const float j = -(1.0f + restitution) * normalVelocity / invMasses;

                const glm::vec3 impulse = j * intersectionInfo.normal;

                body1->setVelocity(body1->getVelocity() - impulse / body1->getMass());
                body2->setVelocity(body2->getVelocity() + impulse / body2->getMass());

                /*if (body1->canRotate()) {
                    body1->setAngularVelocity(body1->getAngularVelocity() - invI1 * glm::cross(relativeCollisionPoint1, impulse));
                }
                if (body2->canRotate()) {
                    body2->setAngularVelocity(body2->getAngularVelocity() + invI2 * glm::cross(relativeCollisionPoint2, impulse));
                }*/

                /* Friction */
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
        if (forceField->isGlobal() || rigidBody->getCollider()->getCollisionInfo(rigidBody->getPosition(), forceField->getPosition(), rigidBody->getRotation(), forceField->getRotation(), forceField->getCollider(), false).has_value()) {
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
            if (trigger->getCollider()->getCollisionInfo(trigger->getPosition(), rigidBody->getPosition(), trigger->getRotation(), rigidBody->getRotation(), rigidBody->getCollider(), false).has_value()) {
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
    const glm::vec3& position,
    const glm::vec3& velocity,
    const glm::vec3& acceleration,
    const glm::quat& rotation,
    const glm::vec3& angularVelocity,
    const glm::vec3& torque
) :
    collider(collider),
    mass(mass),
    material(material),
    layer(layer),
    position(position),
    velocity(velocity),
    acceleration(acceleration),
    rotation(rotation),
    angularVelocity(angularVelocity),
    torque(torque)
{
}

RigidBody::~RigidBody() {
}

void RigidBody::tick(const std::vector<ForceField::Ref>& forcesFields, float delta) {
    if (isStatic()) {
        return;
    }

    // todo force field
    glm::vec3 additionalAcceleration = glm::vec3(0.0f);
    for (const ForceField::Ref& forceField : forcesFields) {
        additionalAcceleration += forceField->getAcceleration(position);
    }
    velocity += delta * (acceleration + additionalAcceleration);
    position += delta * velocity;

    angularVelocity += torque;
    rotation = glm::quat{delta * angularVelocity} * rotation;
}

std::optional<IntersectionInfo> Collider::getCollisionInfo(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const Collider* collider, bool continuous) {
    switch (collider->getColliderType()) {
        case ColliderType::AABB :
            return getCollisionInfoAABB(position, positionOther, rotation, rotationOther, static_cast<const AABBCollider*>(collider), continuous);
        case ColliderType::OBB :
            return getCollisionInfoOBB(position, positionOther, rotation, rotationOther, static_cast<const OBBCollider*>(collider), continuous);
        case ColliderType::SPHERE :
            return getCollisionInfoSphere(position, positionOther, rotation, rotationOther, static_cast<const SphereCollider*>(collider), continuous);
        case ColliderType::CAPSULE :
            return getCollisionInfoCapsule(position, positionOther, rotation, rotationOther, static_cast<const CapsuleCollider*>(collider), continuous);
        case ColliderType::CONVEX :
            return getCollisionInfoConvex(position, positionOther, rotation, rotationOther, static_cast<const ConvexCollider*>(collider), continuous);
        default:
            return std::nullopt;
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

/********************/
/* Helper functions */
/********************/

static std::optional<IntersectionInfo> collisionInfoOBBOBB(const glm::vec3& position1, const glm::vec3& position2, const glm::quat& rotation1, const glm::quat& rotation2, const glm::vec3& size1, const glm::vec3& size2) {
    std::vector<glm::vec3> axes(6);
    glm::vec3 eulerRotation1 = glm::eulerAngles(rotation1);
    glm::mat4 rotationMatrix1 = glm::eulerAngleXYZ(eulerRotation1.x, eulerRotation1.y, eulerRotation1.z);
    axes[0] = glm::vec3(rotationMatrix1 * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    axes[1] = glm::vec3(rotationMatrix1 * glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
    axes[2] = glm::vec3(rotationMatrix1 * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));

    glm::vec3 eulerRotation2 = glm::eulerAngles(rotation2);
    glm::mat4 rotationMatrix2 = glm::eulerAngleXYZ(eulerRotation2.x, eulerRotation2.y, eulerRotation2.z);
    axes[3] = glm::vec3(rotationMatrix2 * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    axes[4] = glm::vec3(rotationMatrix2 * glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
    axes[5] = glm::vec3(rotationMatrix2 * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));

    for (int i = 0; i < 3; ++i) {
        for (int j = 3; j < 6; ++j) {
            const glm::vec3 crossAxes = glm::cross(axes[i], axes[j]);
            if (crossAxes != glm::vec3(0.0f, 0.0f, 0.0f)) {
                axes.push_back(crossAxes);
            }
        }
    }
    std::vector<glm::vec3> points1(8);
    points1[0] = position1 - axes[0] * size1.x - axes[1] * size1.y - axes[2] * size1.z;
    points1[1] = position1 - axes[0] * size1.x - axes[1] * size1.y + axes[2] * size1.z;
    points1[2] = position1 - axes[0] * size1.x + axes[1] * size1.y - axes[2] * size1.z;
    points1[3] = position1 - axes[0] * size1.x + axes[1] * size1.y + axes[2] * size1.z;
    points1[4] = position1 + axes[0] * size1.x - axes[1] * size1.y - axes[2] * size1.z;
    points1[5] = position1 + axes[0] * size1.x - axes[1] * size1.y + axes[2] * size1.z;
    points1[6] = position1 + axes[0] * size1.x + axes[1] * size1.y - axes[2] * size1.z;
    points1[7] = position1 + axes[0] * size1.x + axes[1] * size1.y + axes[2] * size1.z;

    std::vector<glm::vec3> points2(8);
    points2[0] = position2 - axes[3] * size2.x - axes[4] * size2.y - axes[5] * size2.z;
    points2[1] = position2 - axes[3] * size2.x - axes[4] * size2.y + axes[5] * size2.z;
    points2[2] = position2 - axes[3] * size2.x + axes[4] * size2.y - axes[5] * size2.z;
    points2[3] = position2 - axes[3] * size2.x + axes[4] * size2.y + axes[5] * size2.z;
    points2[4] = position2 + axes[3] * size2.x - axes[4] * size2.y - axes[5] * size2.z;
    points2[5] = position2 + axes[3] * size2.x - axes[4] * size2.y + axes[5] * size2.z;
    points2[6] = position2 + axes[3] * size2.x + axes[4] * size2.y - axes[5] * size2.z;
    points2[7] = position2 + axes[3] * size2.x + axes[4] * size2.y + axes[5] * size2.z;
    return sat(points1, points2, axes);
}

static std::optional<IntersectionInfo> collisionInfoOBBSphere(const glm::vec3& position1, const glm::vec3& position2, const glm::quat& rotation, const glm::vec3& size, float radius) {
    std::vector<glm::vec3> axes(3);
    glm::vec3 eulerRotation = glm::eulerAngles(rotation);
    glm::mat4 rotationMatrix = glm::eulerAngleXYZ(eulerRotation.x, eulerRotation.y, eulerRotation.z);
    axes[0] = glm::vec3(rotationMatrix * glm::vec4(size.x, 0.0f, 0.0f, 1.0f));
    axes[1] = glm::vec3(rotationMatrix * glm::vec4(0.0f, size.y, 0.0f, 1.0f));
    axes[2] = glm::vec3(rotationMatrix * glm::vec4(0.0f, 0.0f, size.z, 1.0f));

    const glm::vec3 relativePosition = position2 - position1;
    glm::vec3 normalizedPosition;
    for (int i = 0; i < 3; ++i) {
        normalizedPosition[i] = std::clamp(glm::dot(relativePosition, axes[i]) / glm::length2(axes[i]), -1.0f, 1.0f);
    }
    const glm::vec3 normalizedDifference = glm::abs(normalizedPosition);
    if (glm::all(glm::lessThan(normalizedDifference, glm::vec3(1.0f)))) {
        // Sphere inside
        IntersectionInfo intersectionInfo;
        if (normalizedDifference[0] > normalizedDifference[1] && normalizedDifference[0] > normalizedDifference[2]) {
            intersectionInfo.penetration = (1.0f - normalizedDifference[0]) * size.x;
            intersectionInfo.normal = axes[0] / size.x;
        }
        else if (normalizedDifference[1] > normalizedDifference[0] && normalizedDifference[1] > normalizedDifference[2]) {
            intersectionInfo.penetration = (1.0f - normalizedDifference[1]) * size.y;
            intersectionInfo.normal = axes[1] / size.y;
        }
        else {
            intersectionInfo.penetration = (1.0f - normalizedDifference[2]) * size.z;
            intersectionInfo.normal = axes[2] / size.z;
        }
        intersectionInfo.collisionPoint = position2 + 0.5f * intersectionInfo.penetration * intersectionInfo.normal;
        return std::make_optional(intersectionInfo);
    }
    else {
        // Sphere outside
        glm::vec3 closestPointPosition(0.0f);
        for (int i = 0; i < 3; ++i) {
            closestPointPosition += axes[i] * normalizedPosition[i];
        }
        return getSpheresIntersection(closestPointPosition + position1, 0.0f, position2, radius);
    }
}

static std::optional<IntersectionInfo> collisionInfoConvexConvex(const glm::vec3& position1, const glm::vec3& position2, const glm::quat& rotation1, const glm::quat& rotation2, const std::vector<glm::vec3>& vertices1, const std::vector<glm::vec3>& vertices2) {
    glm::vec3 eulerRotation1 = glm::eulerAngles(rotation1);
    glm::mat4 rotationMatrix1 = glm::eulerAngleXYZ(eulerRotation1.x, eulerRotation1.y, eulerRotation1.z);
    std::vector<glm::vec3> verticesPositions1(vertices1.size());
    for (std::size_t i = 0; i < vertices1.size(); ++i) {
        verticesPositions1[i] = position1 + glm::vec3(rotationMatrix1 * glm::vec4(vertices1[i], 1.0f));
    }

    glm::vec3 eulerRotation2 = glm::eulerAngles(rotation2);
    glm::mat4 rotationMatrix2 = glm::eulerAngleXYZ(eulerRotation2.x, eulerRotation2.y, eulerRotation2.z);
    std::vector<glm::vec3> verticesPositions2(vertices2.size());
    for (std::size_t i = 0; i < vertices2.size(); ++i) {
        verticesPositions2[i] = position2 + glm::vec3(rotationMatrix2 * glm::vec4(vertices2[i], 1.0f));
    }

    return gjk(verticesPositions1, verticesPositions2);
}

static std::optional<IntersectionInfo> collisionInfoOBBConvex(const glm::vec3& position1, const glm::vec3& position2, const glm::quat& rotation1, const glm::quat& rotation2, const glm::vec3& size, const std::vector<glm::vec3>& vertices) {
    std::vector<glm::vec3> obbVertices(8);
    obbVertices[0] = glm::vec3(-size.x, -size.y, -size.z);
    obbVertices[1] = glm::vec3(-size.x, -size.y, size.z);
    obbVertices[2] = glm::vec3(-size.x, size.y, -size.z);
    obbVertices[3] = glm::vec3(-size.x, size.y, size.z);
    obbVertices[4] = glm::vec3(size.x, -size.y, -size.z);
    obbVertices[5] = glm::vec3(size.x, -size.y, size.z);
    obbVertices[6] = glm::vec3(size.x, size.y, -size.z);
    obbVertices[7] = glm::vec3(size.x, size.y, size.z);
    
    return collisionInfoConvexConvex(position1, position2, rotation1, rotation2, obbVertices, vertices);
}

static std::optional<IntersectionInfo> collisionInfoSphereConvex(const glm::vec3& position1, const glm::vec3& position2, const glm::quat rotation, const float radius, const std::vector<glm::vec3>& vertices) {
    glm::vec3 eulerRotation = glm::eulerAngles(rotation);
    glm::mat4 rotationMatrix = glm::eulerAngleXYZ(eulerRotation.x, eulerRotation.y, eulerRotation.z);
    std::vector<glm::vec3> verticesPositions(vertices.size());
    for (std::size_t i = 0; i < vertices.size(); ++i) {
        verticesPositions[i] = position2 + glm::vec3(rotationMatrix * glm::vec4(vertices[i], 1.0f));
    }

    float minDistance = std::numeric_limits<float>::infinity();
    glm::vec3 closestPoint = glm::vec3(0.0f);
    /*for (std::size_t i = 0; i < vertices.size(); ++i) {
        for (std::size_t j = 0; j < vertices.size(); ++j) {
            for (std::size_t k = 0; k < vertices.size(); ++k) {
                if (i == j || j == k || k == i) {
                    glm::vec3 point = closestPointOnTriangle(position1, vertices[i], vertices[j], vertices[k]);
                    float distance = glm::distance2(point, position1);
                    if (distance < minDistance) {
                        minDistance = distance;
                        closestPoint = point;
                    }
                }
            }
        }
    }*/
    for (std::size_t i = 0; i < verticesPositions.size(); ++i) {
        glm::vec3 point = verticesPositions[i];
        float distance = glm::distance2(point, position1);
        if (distance < minDistance) {
            minDistance = distance;
            closestPoint = point;
        }
    }
    return getSpheresIntersection(position1, radius, closestPoint, 0.0f);
    //return std::nullopt;
}

/********/
/* AABB */
/********/

AABBCollider::AABBCollider(const glm::vec3& halfDimensions) :
    size(halfDimensions)
{}

glm::mat3 AABBCollider::getInertiaTensor(float mass) const {
    const float x2{4.0f * size.x * size.x};
    const float y2{4.0f * size.y * size.y};
    const float z2{4.0f * size.z * size.z};
    return glm::diagonal3x3(mass / 12.0f * glm::vec3(y2 + z2, x2 + z2, x2 + y2));
}

std::optional<IntersectionInfo> AABBCollider::getCollisionInfoAABB(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const AABBCollider* collider, bool continuous) const {
    if (!continuous) {
        const glm::vec3 sumSize = size + collider->size;
        const glm::vec3 difference = glm::abs(position - positionOther);
        if (glm::all(glm::lessThanEqual(difference, sumSize))) {
            IntersectionInfo intersectionInfo;
            const glm::vec3 penetration{sumSize - difference};
            if (penetration.x < penetration.y && penetration.x < penetration.z) {
                intersectionInfo.normal = glm::vec3(position.x >= positionOther.x ? 1.0f : -1.0f, 0.0f, 0.0f);
                intersectionInfo.collisionPoint = ((size.x - 0.5f * penetration) / sumSize.x) * (positionOther - position);
                intersectionInfo.penetration = penetration.x;
            }
            else if (penetration.y < penetration.x && penetration.y < penetration.z) {
                intersectionInfo.normal = glm::vec3(0.0f, position.y >= positionOther.y ? 1.0f : -1.0f, 0.0f);
                intersectionInfo.collisionPoint = ((size.y - 0.5f * penetration) / sumSize.y) * (positionOther - position);
                intersectionInfo.penetration = penetration.y;
            }
            else {
                intersectionInfo.normal = glm::vec3(0.0f, 0.0f, position.z >= positionOther.z ? 1.0f : -1.0f);
                intersectionInfo.collisionPoint = ((size.z - 0.5f * penetration) / sumSize.z) * (positionOther - position);
                intersectionInfo.penetration = penetration.z;
            }
            return std::make_optional(intersectionInfo);
        }
        else {
            return std::nullopt;
        }
    }
    else {
        // swept
        return std::nullopt;
    }
}

std::optional<IntersectionInfo> AABBCollider::getCollisionInfoOBB(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const OBBCollider* collider, bool continuous) const {
    if (!continuous) {
        return collisionInfoOBBOBB(position, positionOther, rotation, rotationOther, size, collider->size);
    }
    else {
        // swept
        return std::nullopt;
    }
}

std::optional<IntersectionInfo> AABBCollider::getCollisionInfoSphere(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const SphereCollider* collider, bool continuous) const {
    if (!continuous) {
        return collisionInfoOBBSphere(position, positionOther, rotation, size, collider->radius);
    }
    else {
        // swept
        return std::nullopt;
    }
}

std::optional<IntersectionInfo> AABBCollider::getCollisionInfoConvex(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const ConvexCollider* collider, bool continuous) const {
    if (!continuous) {
        return collisionInfoOBBConvex(position, positionOther, rotation, rotationOther, size, collider->vertices);
    }
    else {
        // swept
        return std::nullopt;
    }
}

std::pair<glm::vec3, glm::vec3> AABBCollider::getBoundingBox(const glm::vec3& position, const glm::quat& rotation) const {
    return std::make_pair(position - size, position + size);
}

/*******/
/* OBB */
/*******/

OBBCollider::OBBCollider(const glm::vec3& halfDimensions) :
    size(halfDimensions)
{}

glm::mat3 OBBCollider::getInertiaTensor(float mass) const {
    const float x2{4.0f * size.x * size.x};
    const float y2{4.0f * size.y * size.y};
    const float z2{4.0f * size.z * size.z};
    return glm::diagonal3x3(mass / 12.0f * glm::vec3(y2 + z2, x2 + z2, x2 + y2));
}

std::optional<IntersectionInfo> OBBCollider::getCollisionInfoAABB(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const AABBCollider* collider, bool continuous) const {
    std::optional<IntersectionInfo> collisionInfo = collider->getCollisionInfoOBB(positionOther, position, rotationOther, rotation, this, continuous);
    if (collisionInfo.has_value()) {
        return std::make_optional(collisionInfo.value().invert());
    }
    return std::nullopt;
}

std::optional<IntersectionInfo> OBBCollider::getCollisionInfoOBB(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const OBBCollider* collider, bool continuous) const {
    if (!continuous) {
        return collisionInfoOBBOBB(position, positionOther, rotation, rotationOther, size, collider->size);
    }
    else {
        // swept
        return std::nullopt;
    }
}

std::optional<IntersectionInfo> OBBCollider::getCollisionInfoSphere(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const SphereCollider* collider, bool continuous) const {
    if (!continuous) {
        return collisionInfoOBBSphere(position, positionOther, rotation, size, collider->radius);
    }
    else {
        // swept
        return std::nullopt;
    }
}

std::optional<IntersectionInfo> OBBCollider::getCollisionInfoConvex(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const ConvexCollider* collider, bool continuous) const {
    if (!continuous) {
        return collisionInfoOBBConvex(position, positionOther, rotation, rotationOther, size, collider->vertices);
    }
    else {
        // swept
        return std::nullopt;
    }
}

std::pair<glm::vec3, glm::vec3> OBBCollider::getBoundingBox(const glm::vec3& position, const glm::quat& rotation) const {
    return geometry::convexSetBoundingBox(geometry::translatePoints(geometry::getBoxVertices(size, rotation), position));
}

/**********/
/* Sphere */
/**********/

SphereCollider::SphereCollider(float radius) :
    radius(radius)
{}

glm::mat3 SphereCollider::getInertiaTensor(float mass) const {
    return glm::mat3(2.0f / 5.0f * mass * radius * radius);
}

std::optional<IntersectionInfo> SphereCollider::getCollisionInfoAABB(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const AABBCollider* collider, bool continuous) const {
    if (!continuous) {
        std::optional<IntersectionInfo> collisionInfo = collisionInfoOBBSphere(positionOther, position, rotationOther, collider->size, radius);
        if (collisionInfo.has_value()) {
            return std::make_optional(collisionInfo.value().invert());
        }
        return std::nullopt;
    }
    else {
        // swept
        return std::nullopt;
    }
}

std::optional<IntersectionInfo> SphereCollider::getCollisionInfoOBB(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const OBBCollider* collider, bool continuous) const {
    if (!continuous) {
        std::optional<IntersectionInfo> collisionInfo = collisionInfoOBBSphere(positionOther, position, rotationOther, collider->size, radius);
        if (collisionInfo.has_value()) {
            return std::make_optional(collisionInfo.value().invert());
        }
        return std::nullopt;
    }
    else {
        // swept
        return std::nullopt;
    }
}

std::optional<IntersectionInfo> SphereCollider::getCollisionInfoSphere(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const SphereCollider* collider, bool continuous) const {
    if (!continuous) {
        return getSpheresIntersection(position, radius, positionOther, collider->radius);
    }
    else {
        // swept
        return std::nullopt;
    }
}

std::optional<IntersectionInfo> SphereCollider::getCollisionInfoCapsule(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const CapsuleCollider* collider, bool continuous) const {
    std::optional<IntersectionInfo> collisionInfo = collider->getCollisionInfoSphere(positionOther, position, rotationOther, rotation, this, continuous);
    if (collisionInfo.has_value()) {
        return std::make_optional(collisionInfo.value().invert());
    }
    return std::nullopt;
}

std::optional<IntersectionInfo> SphereCollider::getCollisionInfoConvex(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const ConvexCollider* collider, bool continuous) const {
    if (!continuous) {
        return collisionInfoSphereConvex(position, positionOther, rotationOther, radius, collider->vertices);
    }
    else {
        // swept
        return std::nullopt;
    } 
}

std::pair<glm::vec3, glm::vec3> SphereCollider::getBoundingBox(const glm::vec3& position, const glm::quat& rotation) const {
    const glm::vec3 radiusVector = glm::vec3(radius);
    return std::make_pair(
        position - radiusVector,
        position + radiusVector
    );
}

/***********/
/* Capsule */
/***********/

CapsuleCollider::CapsuleCollider(const glm::vec3& center1, const glm::vec3& center2, float radius) :
    center1(center1),
    center2(center2),
    radius(radius)
{}

glm::mat3 CapsuleCollider::getInertiaTensor(float mass) const {
    return glm::mat3(1.0f);
    //return glm::mat3(2.0f / 5.0f * mass * radius * radius);
}

std::optional<IntersectionInfo> CapsuleCollider::getCollisionInfoSphere(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const SphereCollider* collider, bool continuous) const {
    if (!continuous) {
        const glm::vec3 lineDifference = center2 - center1;
        const glm::vec3 pointDifference = positionOther - center1;
        const float linePosition = std::clamp(glm::dot(lineDifference, pointDifference) / glm::length2(lineDifference), 0.0f, 1.0f);
        const glm::vec3 pointPosition = linePosition * lineDifference + center1;
        return getSpheresIntersection(pointPosition, radius, position, collider->radius); 
    }
    else {
        // swept
        return std::nullopt;
    }
}

// todo implement
std::pair<glm::vec3, glm::vec3> CapsuleCollider::getBoundingBox(const glm::vec3& position, const glm::quat& rotation) const {
    return std::make_pair(center1, center2);
}

/**********/
/* Convex */
/**********/

ConvexCollider::ConvexCollider(
    const std::vector<glm::vec3>& vertices
) :
    vertices(vertices)
{}

glm::mat3 ConvexCollider::getInertiaTensor(float mass) const {
    return glm::mat3(1.0f);
}

std::optional<IntersectionInfo> ConvexCollider::getCollisionInfoAABB(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const AABBCollider* collider, bool continuous) const {
    if (!continuous) {
        std::optional<IntersectionInfo> collisionInfo = collisionInfoOBBConvex(positionOther, position, rotationOther, rotation, collider->size, vertices);
        if (collisionInfo.has_value()) {
            return std::make_optional(collisionInfo.value().invert());
        }
        return std::nullopt;
    }
    else {
        // swept
        return std::nullopt;
    }
}

std::optional<IntersectionInfo> ConvexCollider::getCollisionInfoOBB(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const OBBCollider* collider, bool continuous) const {
    if (!continuous) {
        std::optional<IntersectionInfo> collisionInfo = collisionInfoOBBConvex(positionOther, position, rotationOther, rotation, collider->size, vertices);
        if (collisionInfo.has_value()) {
            return std::make_optional(collisionInfo.value().invert());
        }
        return std::nullopt;
    }
    else {
        // swept
        return std::nullopt;
    }
}

std::optional<IntersectionInfo> ConvexCollider::getCollisionInfoConvex(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const ConvexCollider* collider, bool continuous) const {
    if (!continuous) {
        return collisionInfoConvexConvex(position, positionOther, rotation, rotationOther, vertices, collider->vertices);
    }
    else {
        // swept
        return std::nullopt;
    }
}

std::optional<IntersectionInfo> ConvexCollider::getCollisionInfoSphere(const glm::vec3& position, const glm::vec3& positionOther, const glm::quat& rotation, const glm::quat& rotationOther, const SphereCollider* collider, bool continuous) const {
    if (!continuous) {
        std::optional<IntersectionInfo> collisionInfo = collisionInfoSphereConvex(positionOther, position, rotation, collider->radius, vertices);
        if (collisionInfo.has_value()) {
            return std::make_optional(collisionInfo.value().invert());
        }
        return std::nullopt;
    }
    else {
        // swept
        return std::nullopt;
    }
}

// todo rotate
std::pair<glm::vec3, glm::vec3> ConvexCollider::getBoundingBox(const glm::vec3& position, const glm::quat& rotation) const {
    return geometry::convexSetBoundingBox(geometry::translatePoints(vertices, position));
}