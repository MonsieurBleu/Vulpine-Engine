#pragma once

#include <VulpineParser.hpp>
#include <Entity.hpp>
#include <ModularEntityGrouppingUtils.hpp>
#include <AssetManagerUtils.hpp>

/*
    Modular Entitiy Groupping provides tools to group entities in a simplified hierarchy pattern.
    To maintain its ease of use, and prevent system repetition or overblow, it  must maintains the 
    following principes, regaardless of the actual implementation :

        1 - All instanciable MEG all have an unique ID name
        2 - MEG updates should be done from top layer to bottom 
        3 - MEG Components can be:
            1. Statics : if no system other than clean and init should be applied to the components
            2. Synchronizable : if the data from the parent can affect this component of the children 
            3. Mergeable : if the component can be merged with the data from the parent
            4. Verifiably compatible : if compatibility should be tested with the parent data before merging.
            5. Dummifiable : if the component should be always pre-loaded when the parent spawns 
        4 - Satics MEG only update spawning and despawning and a possible LOD implementation
        5 - Non statics MEG update all synchronizable components continuesly
        6 - Merge between two entities is legal if : 
            1. The parent has all of the children's components 
            1. All components of the children are mergeable AND compatible wit the parent's data.
        7 - All MEG have a cateogy and a dummy set of componment that match the category template
        9 - All MEG dummy are used to determine spawning and despawning conditions 


    Notes :
        * There is a lot of leaway to abuse component unique compile-time IDs for MEG and ECS manipulation
        * MEG loading and reading should be defined by the Vulpine Asset Manager only
        * In the actual game, almost all MEG should be statics, with rare exceptions 
        * In map/level designer, all MEG should be temporarly loaded as non-statics
*/


/*
    1 - An Entity can be reparent if his parent is mergeable and compatible with his new parent

*/

struct EntityGroupInfo
{
    std::vector<EntityRef> children;
    bool markedForDeletion = false;
    bool markedForCreation = false;
};

COMPONENT(EntityGroupInfo, ENTITY_LIST, MAX_ENTITY);


class ComponentModularity
{
    private : 

    public :

        template <typename T> struct Attribute
        {
            uint16_t ComponentID = 0;
            T element;
        };

        typedef Attribute<std::function<void(Entity&, EntityRef)>> SynchFunc;
        typedef Attribute<std::function<void(Entity&, EntityRef)>> MergeFunc;
        typedef Attribute<std::function<bool(Entity&, EntityRef)>> CheckFunc;
        typedef Attribute<std::function<bool(Entity&, EntityRef, Entity&)>> ReparFunc;
        typedef Attribute<std::function<bool(Entity&, EntityRef)>> ClearFunc;

        typedef Attribute<std::function<void(EntityRef, VulpineTextBuffRef)>> ReadFunc;
        typedef Attribute<std::function<void(EntityRef, VulpineTextOutputRef)>> WriteFunc;

        static inline std::vector<SynchFunc> SynchFuncs;
        static inline std::vector<MergeFunc> MergeFuncs;
        static inline std::vector<CheckFunc> CheckFuncs;
        static inline std::vector<ReparFunc> ReparFuncs;
        static inline std::vector<ClearFunc> ClearFuncs;
        static inline std::vector<ReadFunc>  ReadFuncs;
        static inline std::vector<WriteFunc> WriteFuncs;

        static void addChild(Entity &parent, EntityRef child)
        {
            parent.comp<EntityGroupInfo>().children.push_back(child);
        };

        static void removeChild(Entity &parent, EntityRef child)
        {
            std::vector<EntityRef> children;
            auto &old = parent.comp<EntityGroupInfo>().children;
            
            for(auto i : old)
                if(i != child)
                    children.push_back(i);
            
            old = children;
        };

        static void synchronizeChildren(Entity &parent)
        {
            for(auto &c : parent.comp<EntityGroupInfo>().children)
                for(auto &i : SynchFuncs)
                    if(c->state[i.ComponentID])
                        i.element(parent, c);
        };

        static void Reparent(Entity& parent, EntityRef child, Entity& newParent)
        {
            for(auto &i : ReparFuncs)
                if(child->state[i.ComponentID])
                    i.element(parent, child, newParent);
            
            newParent.comp<EntityGroupInfo>().children.push_back(child);

            auto &children = parent.comp<EntityGroupInfo>().children;
            std::vector<EntityRef> newchildren;
            for(auto i : children)
                if(i != child)
                    newchildren.push_back(i);
            children = newchildren;
        };

        static bool canMerge(Entity& parent, EntityRef child)
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

        static void mergeChild(Entity& parent, EntityRef child)
        {
            for(auto &i : MergeFuncs)
                if(child->state[i.ComponentID])
                    i.element(parent, child);
        };

        static void mergeChildren(Entity& parent)
        {
            auto &children = parent.comp<EntityGroupInfo>().children;

            for(uint64_t i = 0; i < children.size(); i++)
                if(canMerge(parent, children[i]))
                    {
                        auto &c = children[i];
                        mergeChildren(*c);

                        for(auto &j : c->comp<EntityGroupInfo>().children)
                            Reparent(*c, j, parent);

                        mergeChild(parent, c);

                        c->comp<EntityGroupInfo>().markedForDeletion = true;
                    }
        };
};
