
#pragma once

#ifndef VULPINE_COMPONENT_IMPL
#include <ECS/ModularEntityGroupping.hpp>
#endif

void ComponentModularity::addChild(Entity &parent, EntityRef child)
{
    if(!parent.hasComp<EntityGroupInfo>())
        parent.set<EntityGroupInfo>(EntityGroupInfo());

    Reparent(parent, child, parent);
};

void ComponentModularity::removeChild(Entity &parent, EntityRef child)
{
    std::vector<EntityRef> children;
    auto &old = parent.comp<EntityGroupInfo>().children;
    
    for(auto i : old)
        if(i != child)
            children.push_back(i);
    
    old = children;
}

void ComponentModularity::synchronizeChildren(EntityRef parent)
{
    for(auto &i : SynchFuncs)
        if(parent->state[i.ComponentID])
            i.element(*parent, parent);

    for(auto &c : parent->comp<EntityGroupInfo>().children)
    {
        synchronizeChildren(c);
        
        for(auto &i : SynchFuncs)
            if(c->state[i.ComponentID])
                i.element(*parent, c);

    }
}


void ComponentModularity::Reparent(Entity& parent, EntityRef child, Entity& newParent)
{
    for(auto &i : ReparFuncs)
        if(child->state[i.ComponentID])
            i.element(parent, child, newParent);
    
    if(!newParent.hasComp<EntityGroupInfo>())
        newParent.set<EntityGroupInfo>(EntityGroupInfo());

    newParent.comp<EntityGroupInfo>().children.push_back(child);

    if(&parent != &newParent)
    {
        auto &children = parent.comp<EntityGroupInfo>().children;
        std::vector<EntityRef> newchildren;
        for(auto &i : children)
            if(i != child)
                newchildren.push_back(i);
        children = newchildren;
    }
};

bool ComponentModularity::canMerge(Entity& parent, EntityRef child)
{
    bool success = true;

    for(auto &i : CheckFuncs)
        if(child->state[i.ComponentID])
            success &= i.element(parent, child);
        else
        {
            WARNING_MESSAGE("TODO : Add message when there ar merge conflicts");
            break;
        }
    
    return success;
};

void ComponentModularity::mergeChild(Entity& parent, EntityRef child)
{
    for(auto &i : MergeFuncs)
        if(child->state[i.ComponentID])
            i.element(parent, child);
};

void ComponentModularity::mergeChildren(Entity& parent)
{
    if(!parent.hasComp<EntityGroupInfo>()) return;

    auto &children = parent.comp<EntityGroupInfo>().children;
    std::vector<EntityRef> newChildren;

    for(auto c : children)
    {
        mergeChildren(*c);
        Entity *ctmp = c.get();

        if(canMerge(parent, c))
        {
            assert(ctmp == c.get());

            int tmpcnt = 0;
            for(auto j : c->comp<EntityGroupInfo>().children)
            {
                Reparent(*c, j, parent);
                assert(ctmp == c.get());
            }

            assert(ctmp == c.get());
            mergeChild(parent, c);

            c->comp<EntityGroupInfo>().markedForDeletion = true;
        }
        else
        {
            newChildren.push_back(c);
        }
    }

    children = newChildren;
};
