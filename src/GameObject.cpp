#include "GameObject.hpp"
#include <iostream>

GameObject::GameObject()
{
}

GameObject::GameObject(ObjectGroupRef ObjectGroup, RigidBodyRef RigidBody) : objectGroup(ObjectGroup), rigidBody(RigidBody)
{
}

GameObject::GameObject(ObjectGroupRef ObjectGroup) : objectGroup(ObjectGroup)
{
}

GameObject::GameObject(RigidBodyRef RigidBody) : rigidBody(RigidBody)
{
}

GameObject::~GameObject()
{
}

void GameObject::update(float deltaTime)
{
    if (objectGroup != NULL)
    {
        if (rigidBody != NULL)
        {
            vec3 pos = rigidBody->getPosition();
            vec3 rot = eulerAngles(rigidBody->getRotation());

            // no idea if this actually faster, it prevents an update if it is unnecessary
            // but it does branch so it might be slower overall if we have a lot of moving objects
            if (objectGroup->state.position != pos)
                objectGroup->state.setPosition(pos);
            if (objectGroup->state.rotation != rot)
                objectGroup->state.setRotation(rot);
        }
    }
}