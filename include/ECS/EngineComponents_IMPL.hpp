
#pragma once

#ifndef VULPINE_COMPONENT_IMPL
#include <ECS/EngineComponents.hpp>
#endif

#include <Graphics/Scene.hpp>

template<> void Component<WidgetModel>::ComponentElem::init()
{
    data.scene->add(data);
};

template<> void Component<WidgetModel>::ComponentElem::clean()
{
    if(data.get())
        data.scene->remove(data);
    else
        WARNING_MESSAGE("Trying to clean null component from entity " << entity->ids[ENTITY_LIST] << " named " << entity->comp<EntityInfos>().name)
};

