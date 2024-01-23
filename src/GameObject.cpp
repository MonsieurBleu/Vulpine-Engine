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

vec3 GameObject::getPosition()
{
    if (rigidBody != NULL)
        return rigidBody->getPosition();
    else
        return objectGroup->state.position;
}

void GameObject::setPosition(vec3 position)
{
    if (rigidBody != NULL)
        rigidBody->setPosition(position);
    else
        objectGroup->state.setPosition(position);
}

vec3 GameObject::getRotation()
{
    if (rigidBody != NULL)
        return eulerAngles(rigidBody->getRotation());
    else
        return objectGroup->state.rotation;
}

void GameObject::setRotation(vec3 rotation)
{
    if (rigidBody != NULL)
        rigidBody->setRotation(quat(rotation));
    else
        objectGroup->state.setRotation(rotation);
}

void GameObject::translate(vec3 translation)
{
    if (rigidBody != NULL)
        rigidBody->setPosition(rigidBody->getPosition() + translation);
    else
        objectGroup->state.position += translation;
}