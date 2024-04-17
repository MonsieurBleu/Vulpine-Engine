#pragma once
#include <glm/glm.hpp>

#include "Physics.hpp"
#include "ObjectGroup.hpp"

using namespace glm;

class GameObject
{
private:
    ObjectGroupRef objectGroup;
    RigidBody::Ref rigidBody;

public:
    GameObject();
    GameObject(ObjectGroupRef ObjectGroup, RigidBody::Ref RigidBody);
    GameObject(ObjectGroupRef ObjectGroup);
    GameObject(RigidBody::Ref RigidBody);
    ~GameObject();

    void update();

    ObjectGroupRef &getGroup() { return objectGroup; };
    RigidBody::Ref &getBody() { return rigidBody; };

    vec3 getPosition();
    void setPosition(vec3 position);

    vec3 getRotation();
    void setRotation(vec3 rotation);

    void translate(vec3 translation);
};