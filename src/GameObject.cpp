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

            // also there seems to be a problem with the object group state, setting it doesn't seem to update the meshes
            // so insted we directly update the meshes inside the object group
            // also this isn't ideal since we assume all the meshes have the same position within the object group which is obviously a problem
            for (ModelRef mesh : objectGroup->getMeshes())
            {
                if (mesh->state.rotation != rot)
                    mesh->state.setRotation(rot);

                if (mesh->state.position != pos)
                    mesh->state.setPosition(pos);
            }

            // std::cout << "pos: " << pos.x << ", " << pos.y << ", " << pos.z << std::endl;
        }
    }
}