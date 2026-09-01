
#pragma once

#ifndef VULPINE_COMPONENT_IMPL
#include <ECS/ModularEntityGroupping.hpp>
#endif

thread_local BenchTimer systemTimer;
std::unordered_map<std::string, BenchTimer> systemsPreciseTimer;
std::recursive_mutex systemsPreciseTimerMutex;

BenchTimer& getSystemPreciseTimer(const std::string & nameID)
{
    static BenchTimer dummy("SYSTEM TIMER NOT FOUND");

    systemsPreciseTimerMutex.lock();

    // auto elem = systemsPreciseTimer.find(nameID);
    // BenchTimer & timer = elem == systemsPreciseTimer.end() ? dummy : systemsPreciseTimer[nameID];

    // WARNING_MESSAGE((glm::uint64)nameID.data())
    // NOTIF_MESSAGE(nameID)

    BenchTimer & timer = systemsPreciseTimer[nameID];
    systemsPreciseTimerMutex.unlock();
    return timer;
}


#ifdef VULPINE_COMPONENT_IMPL
COMPONENT_IMPL(EntityInfos)

Entity::~Entity()
{
    // std::cout << "Destroying " << toStr() << "\n";

    // NOTIF_MESSAGE("Destroying ", toStr());

    Component<EntityInfos>::elements[ids[ENTITY_LIST]].enabled = false;

    for(int i = 0; i < ComponentCategory::END; i++)
        if(ids[i] != NO_ENTITY)
        {
            if(ids[i] < ComponentGlobals::lastFreeID[i])
                ComponentGlobals::lastFreeID[i] = ids[i];

            ComponentGlobals::status[i][ids[i]].enabled = false;

            if(ids[i] >= ComponentGlobals::maxID[i]-1)
            {
                // ComponentGlobals::maxID[i]--;                

                for(int j = ids[i]; j >= 0; j--)
                {
                    if(ComponentGlobals::status[i][j].enabled)
                    {
                        ComponentGlobals::maxID[i] = j;
                        break;
                    }

                    if(j == 0) ComponentGlobals::maxID[i] = 0;
                }

                // WARNING_MESSAGE(i, "   ",  ids[i], "   ", ComponentGlobals::maxID[i]);

            }
            
            
        }
    
    if(has<EntityGroupInfo>())
        comp<EntityGroupInfo>().children.clear();
}

#endif

void ComponentModularity::addChild(Entity &parent, EntityRef child)
{
    if(!parent.has<EntityGroupInfo>())
        parent.set<EntityGroupInfo>(EntityGroupInfo());

    if(!child->has<EntityGroupInfo>())
        child->set<EntityGroupInfo>(EntityGroupInfo());

    child->comp<EntityGroupInfo>().parent = &parent;

    Reparent(parent, child, parent);
};

void ComponentModularity::removeChild(Entity &parent, EntityRef child)
{
    removeChild(parent, child.get());
}

void ComponentModularity::removeChild(Entity &parent, Entity* child)
{
    std::vector<EntityRef> children;
    auto &old = parent.comp<EntityGroupInfo>().children;
    
    for(auto i : old)
        if(i.get() != child)
            children.push_back(i);
    
    old = children;
}

void ComponentModularity::synchronizeChildren(EntityRef parent)
{
    int synchCompCnt = 0;

    for(auto &i : SynchFuncs)
        if(parent->state[i.ComponentID])
        {
            synchCompCnt ++;
            i.element(*parent, parent);
        }

    if(!synchCompCnt) return;

    if(parent->has<EntityGroupInfo>())
        for(auto &c : parent->comp<EntityGroupInfo>().children)
        {
            // synchronizeChildren(c);
            
            for(auto &i : SynchFuncs)
                if(c->state[i.ComponentID])
                    i.element(*parent, c);

            synchronizeChildren(c);
        }
}


void ComponentModularity::Reparent(Entity& parent, EntityRef child, Entity& newParent)
{

    // std::cout << "REPARENTING CHILD : " << child->comp<EntityInfos>().name << "\t"
    // << " TO PARENT " << newParent.comp<EntityInfos>().name << "\n";

    for(auto &i : ReparFuncs)
        if(child->state[i.ComponentID] 
        // && parent.state[i.ComponentID]
        )
            i.element(parent, child, newParent);
    
    if(!newParent.has<EntityGroupInfo>())
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


void ComponentModularity::ReparentChildren(Entity& parent)
{
    for(auto c : parent.comp<EntityGroupInfo>().children)
    {   
        for(auto &i : ReparFuncs)
            if(c->state[i.ComponentID] 
            // && parent.state[i.ComponentID]
            )
                i.element(parent, c, parent);
                
        ReparentChildren(*c);
    }
};



bool ComponentModularity::canMerge(Entity& parent, EntityRef child)
{
    bool success = true;

    for(auto &i : CheckFuncs)
        if(!parent.state[i.ComponentID])
        {
            success = true;
        }
        else if(child->state[i.ComponentID])
            success &= i.element(parent, child);
        else
        {
            WARNING_MESSAGE("TODO : Add message when there are merge conflicts");
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
    if(!parent.has<EntityGroupInfo>()) return;

    auto &children = parent.comp<EntityGroupInfo>().children;
    std::vector<EntityRef> newChildren;

    std::vector<EntityRef> childrenCopy = children;

    for(auto c : childrenCopy)
    {
        if(!c)
        {   
            WARNING_MESSAGE("EMPTY CHILDREN WHEN MERGING ENTITY " ,  parent.comp<EntityInfos>().name ,  "\n");
            continue;
        } 
        
        mergeChildren(*c);
        
        if(canMerge(parent, c))
        {

            auto c_childrenCopy = c->comp<EntityGroupInfo>().children;
            for(auto j : c_childrenCopy)
            {
                Reparent(*c, j, parent);

                newChildren.push_back(j);
            }

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
