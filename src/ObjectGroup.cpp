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

    

    if(globalUpdate)
    {
        for(auto i = meshes.begin(); i != meshes.end(); i++)
        {
            (*i)->state.modelMatrix = state.modelMatrix * (*i)->state.forceUpdate().modelMatrix;
            ManageHideStatus((*i)->state.hide, state.hide);
        }

        for(auto &i : lights) i->applyModifier(state);

        for(auto i : states)
        {
            i->modelMatrix = state.modelMatrix * i->forceUpdate().modelMatrix;
            ManageHideStatus(i->hide, state.hide);
        }

    }
    
    for(auto i = children.begin(); i != children.end(); i++)
    {
        if((*i)->state.update() || globalUpdate)
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

void ObjectGroup::setAnimation(SkeletonAnimationState *animation)
{
    for(auto &i : meshes)
        if(i->getVao().animated)
            i->animation = animation;

    for(auto &i : children)
        i->setAnimation(animation);
}

void ObjectGroup::remove(ModelRef mesh)
{
    for(auto i = meshes.begin(); i < meshes.end(); i++)
        if(i->get() == mesh.get())
        {
            meshes.erase(i);
            return;
        }
}

ObjectGroupRef ObjectGroup::copy()
{
    ObjectGroupRef g = newObjectGroup();

    g->children.resize(children.size());
    g->states.resize(states.size());
    g->meshes.resize(meshes.size());
    g->lights.resize(lights.size());
    g->state = state.forceUpdate();

    int i = 0;
    for(auto &c : children) g->children[i++] = c->copy();
    i = 0;
    for(auto &c : states) g->states[i++] = ModelStateRef(new ModelState3D(*c));
    i = 0;
    for(auto &c : lights)
        switch (c->getInfos()._infos.a)
        {
            case TUBE_LIGHT : g->lights[i++] = SceneTubeLight(new TubeLight(dynamic_cast<TubeLight&>(*c))); break;
            case POINT_LIGHT : g->lights[i++] = ScenePointLight(new PointLight(dynamic_cast<PointLight&>(*c))); break;
            case DIRECTIONAL_LIGHT : g->lights[i++] = SceneDirectionalLight(new DirectionLight(dynamic_cast<DirectionLight&>(*c))); break;
            default:break;
        }
    i = 0;
    for(auto &c : meshes) g->meshes[i++] = c->copyWithSharedMesh();

    return g;
}
