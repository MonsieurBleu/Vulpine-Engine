#include <ObjectGroup.hpp>
#include <iostream>

void ManageHideStatus(ModelStateHideStatus &children, ModelStateHideStatus parent)
{
    switch (children)
    {
    case ModelStateHideStatus::UNDEFINED :
        children = parent;
        break;
    
    case ModelStateHideStatus::SHOW :
        children = parent == ModelStateHideStatus::HIDE ? parent : children;
        break;

    case ModelStateHideStatus::HIDE :
        children = parent == ModelStateHideStatus::SHOW ? parent : children;
        break;

    default: break;
    }
}

void ObjectGroup::update(bool forceUpdate)
{
    bool globalUpdate = state.update();
    globalUpdate |= forceUpdate;

    for(auto i = meshes.begin(); i != meshes.end(); i++)
    {
        if(globalUpdate || (*i)->state.update())
            (*i)->state.modelMatrix = state.modelMatrix * (*i)->state.forceUpdate().modelMatrix;
        
        ManageHideStatus((*i)->state.hide, state.hide);
    }

    // auto j = lightsDummys.begin();
    // for(auto i = lights.begin(); 
    //     i != lights.end() && j != lightsDummys.end(); 
    //     i++, j++)
    // {
    //     // need to be rework
    //     // (*i)->infos._position = state.modelMatrix * (*j)->infos._position;
        
    //     // if((*i)->infos._infos.a == POINT_LIGHT)
    //     //     (*i)->infos._direction.x = (*j)->infos._direction.x * state.scale.x;

    //     (*i)->applyModifier(state);
    // }

    for(auto i = lights.begin(); i != lights.end(); i++)
        (*i)->applyModifier(state);

    for(auto i : states)
    {
        if(globalUpdate || i->update())
            i->modelMatrix = state.modelMatrix * i->forceUpdate().modelMatrix;
        
        ManageHideStatus(i->hide, state.hide);
    }

    for(auto i = children.begin(); i != children.end(); i++)
    {
        if(globalUpdate || (*i)->state.update())
            (*i)->state.modelMatrix = state.modelMatrix * (*i)->state.forceUpdate().modelMatrix;

        ManageHideStatus((*i)->state.hide, state.hide);
        (*i)->update(true);
    }
}

void ObjectGroup::add(ModelRef meshe)
{
    meshes.push_back(meshe);
}

void ObjectGroup::add(SceneLight light)
{
    // lights.push_back(newLight(*light));
    // lightsDummys.push_back(light);

    lights.push_back(light);
}

void ObjectGroup::add(ObjectGroupRef group)
{
    children.push_back(group);
}

void ObjectGroup::add(ModelStateRef state)
{
    states.push_back(state);
}
