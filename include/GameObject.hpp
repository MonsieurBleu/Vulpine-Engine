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

    void update();

    ObjectGroupRef &getGroup() { return objectGroup; };
    RigidBodyRef &getBody() { return rigidBody; };

    vec3 getPosition();
    void setPosition(vec3 position);

    vec3 getRotation();
    void setRotation(vec3 rotation);

    void translate(vec3 translation);
};