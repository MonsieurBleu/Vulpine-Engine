#include <ObjectGroup.hpp>
#include <iostream>

void ObjectGroup::update(bool forceUpdate)
{
    bool globalUpdate = state.update();
    globalUpdate |= forceUpdate;

    for(auto i = meshes.begin(); i != meshes.end(); i++)
    {
        if(globalUpdate || (*i)->state.update())
        {
            (*i)->state.modelMatrix = state.modelMatrix * (*i)->state.forceUpdate().modelMatrix;
        }
    }

    auto j = lightsDummys.begin();
    for(auto i = lights.begin(); 
        i != lights.end() && j != lightsDummys.end(); 
        i++, j++)
    {
        (*i)->_position = state.modelMatrix * (*j)->_position;
        
        if((*i)->_infos.a == POINT_LIGHT)
            (*i)->_direction.x = (*j)->_direction.x * state.scale.x;
    }

    for(auto i = children.begin(); i != children.end(); i++)
    {
        if(globalUpdate || (*i)->state.update())
            (*i)->state.modelMatrix = state.modelMatrix * (*i)->state.forceUpdate().modelMatrix;

        (*i)->update(true);
    }
}

void ObjectGroup::add(ModelRef meshe)
{
    meshes.push_back(meshe);
}

void ObjectGroup::add(SceneLight light)
{
    lights.push_back(newLight(*light));
    lightsDummys.push_back(light);
}

void ObjectGroup::add(ObjectGroupRef group)
{
    children.push_back(group);
}