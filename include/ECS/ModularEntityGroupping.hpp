#pragma once

#include <VulpineParser.hpp>
#include <ECS/Entity.hpp>
#include <ECS/ModularEntityGrouppingUtils.hpp>
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
    EntityGroupInfo(){};
    EntityGroupInfo(const std::vector<EntityRef> & children) : children(children){};

    std::vector<EntityRef> children;    
    bool markedForDeletion = false;
    bool markedForCreation = false;

    Entity *parent = nullptr;
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
        typedef Attribute<std::function<void(Entity&, EntityRef, Entity&)>> ReparFunc;
        typedef Attribute<std::function<void(EntityRef, VulpineTextBuffRef)>> ReadFunc;
        typedef Attribute<std::function<void(EntityRef, VulpineTextOutputRef)>> WriteFunc;
        // typedef Attribute<std::function<bool(Entity&, EntityRef)>> ClearFunc;

        static inline std::vector<SynchFunc> SynchFuncs;
        static inline std::vector<MergeFunc> MergeFuncs;
        static inline std::vector<CheckFunc> CheckFuncs;
        static inline std::vector<ReparFunc> ReparFuncs;
        static inline std::vector<ReadFunc>  ReadFuncs;
        static inline std::vector<WriteFunc> WriteFuncs;
        // static inline std::vector<ClearFunc> ClearFuncs;

        static void addChild(Entity &parent, EntityRef child);

        static void removeChild(Entity &parent, EntityRef child);

        static void synchronizeChildren(EntityRef parent);

        static void Reparent(Entity& parent, EntityRef child, Entity& newParent);

        static bool canMerge(Entity& parent, EntityRef child);

        static void mergeChild(Entity& parent, EntityRef child);

        static void mergeChildren(Entity& parent);
};

#ifdef VULPINE_COMPONENT_IMPL

#include <ECS/ModularEntityGroupping_IMPL.hpp>

#endif