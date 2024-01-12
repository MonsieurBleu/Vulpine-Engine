#pragma once
#include <glm/glm.hpp>

#include "PhysicsEngine.hpp"
#include "ObjectGroup.hpp"

using namespace glm;

class GameObject
{
private:
    ObjectGroupRef objectGroup;
    RigidBodyRef rigidBody;

public:
    GameObject();
    GameObject(ObjectGroupRef ObjectGroup, RigidBodyRef RigidBody);
    GameObject(ObjectGroupRef ObjectGroup);
    GameObject(RigidBodyRef RigidBody);
    ~GameObject();

    void update(float deltaTime);
};