#pragma once 

#include <MappedEnum.hpp>
#include <VulpineBitSet.hpp>
#include <array>
#include <vector>
#include <cassert>

#include <sstream>
#include <Utils.hpp>

#define PreLaunchContainerFill(_containerType_, _name_) \
    template<typename T> \
    class PreLaunch##_name_##Fill \
    { \
        public : PreLaunchVectorFill(_containerType_<T> &v, T i){v.push_back(i);}; \
    }; 

template<typename T> class PreLaunchVectorFill { public : PreLaunchVectorFill(std::vector<T> &v, T i){v.push_back(i);}; };
template<typename T, typename D> class PreLaunchMapFill { public : PreLaunchMapFill(std::unordered_map<T, D> &v, const T &i, const D &j){v[i] = j;}; };

GENERATE_ENUM(ComponentCategory,
    ENTITY_LIST,
    DATA,       // Default category, plan data with no important dependencies in the engine
    GRAPHIC,    // Components that contains graphic related elements, should only be manipulated from the main thread
    PHYSIC,     // Components that contains physic related elements, should be manipulated with the physic mutex in mind, or in the physics thread directly 
    SOUND,      // Components that contains Sound related elements
    AI,         // Components that contains AI related elements, should be manipulated with the AI mutex in mind, or in the AI thread directly 
    END 
);

#ifndef MAX_ENTITY
    #define MAX_ENTITY 1024
#endif

#ifndef MAX_COMP
    #define MAX_COMP 512
#endif

#define NO_ENTITY -1

class Entity;

struct ComponentGlobals
{
    static inline int lastID = -1;
    static inline int lastID2 = -1; // Needef for clang++ way of initialazing const in vs PreLaunchxxx class
    static inline int lastFreeID[ComponentCategory::END] = {0};
    static inline int maxID[ComponentCategory::END] = {0};

    static inline std::unordered_map<std::string, int> ComponentNamesMap;
    static inline std::vector<std::string> ComponentNames;
};

template <typename T>
struct ComponentInfos
{
    static inline const int size = 0;
    static inline const int id = 0;
};

template <typename T>
class Component
{
    private : 

        // static inline int lastFreeID = 0;

    public :

        struct ComponentElem
        {
            T data;
            Entity *entity = nullptr;
            bool enabled = false;
            bool markedForDeletion = false;
            int entityListID = -1;
            void init(){};
            void clean(){};
        };

        template<typename ... Args>
        T CopyElision(Args&&... args){return T(args ...);};

        static inline const ComponentCategory category = ComponentCategory::ENTITY_LIST; 
        static inline std::array<ComponentElem, ComponentInfos<T>::size> elements;

        static void insert(Entity &entity, const T& data);      
};


#define COMPONENT(_type_, _category_, _size_) \
    static_assert(_size_ <= MAX_ENTITY); \
    template<> inline const int ComponentInfos<_type_>::size = _size_;\
    template<> inline const ComponentCategory Component<_type_>::category = _category_; \
    template<> inline const int ComponentInfos<_type_>::id = ComponentGlobals::lastID++;\
    inline PreLaunchVectorFill<std::string> __ComponentNamesSetupObject__##_type_(ComponentGlobals::ComponentNames, #_type_); \
    inline PreLaunchMapFill<std::string, int> __ComponentNamesMapSetupObject__##_type_(ComponentGlobals::ComponentNamesMap, #_type_, ComponentGlobals::lastID2++);\

struct EntityInfos
{
    std::string name;
};

COMPONENT(EntityInfos, ENTITY_LIST, MAX_ENTITY);

struct templatePass{template<typename ...T> templatePass(T...) {}};

class Entity
{
    private : 

    public :

        VulpineBitSet<MAX_COMP> state;
        int ids[ComponentCategory::END] = {NO_ENTITY, NO_ENTITY, NO_ENTITY, NO_ENTITY, NO_ENTITY, NO_ENTITY};

        Entity(){Component<EntityInfos>::insert(*this, EntityInfos());};
        Entity(const std::string &name){Component<EntityInfos>::insert(*this, EntityInfos{name});};

        template<typename ... components>
        Entity(const std::string &name, components&&... component) : Entity(name)
        {
            templatePass{(set(component), 0) ...};
        };

        ~Entity()
        {
            Component<EntityInfos>::elements[ids[ENTITY_LIST]].enabled = false;

            for(int i = 0; i < ComponentCategory::END; i++)
                if(ids[i] != NO_ENTITY)
                {
                    if(ids[i] < ComponentGlobals::lastFreeID[i])
                        ComponentGlobals::lastFreeID[i] = ids[i];

                    if(ids[i] == ComponentGlobals::maxID[i]-1)
                        ComponentGlobals::maxID[i]--;
                }
        };

        std::string toStr()
        {
            std::stringstream ss;
            ss << TERMINAL_OK << "Entity {";
            
            ss << TERMINAL_NOTIF << TERMINAL_UNDERLINE << comp<EntityInfos>().name << TERMINAL_RESET << " "; 
        
            for(auto &i : ComponentCategoryMap)
                if(i.second != ComponentCategory::END)
                    ss << "  " << TERMINAL_FILENAME << i.first << "[" << TERMINAL_TIMER << ids[i.second] << "]";

            ss << TERMINAL_OK << "}\n" << TERMINAL_RESET;
            return ss.str();
        };

        template<typename T>
        T& comp()
        {
            assert(ids[Component<T>::category] != NO_ENTITY);
            return Component<T>::elements[ids[Component<T>::category]].data;
        };

        template<typename T>
        bool hasComp()
        {
            // assert(ids[Component<T>::category] != NO_ENTITY);
            return state[ComponentInfos<T>::id];
            // return ids[Component<T>::category] != NO_ENTITY && Component<T>::elements[ids[Component<T>::category]].enabled;
        };

        template<typename T>
        void set(const T& data)
        {
            Component<T>::insert(*this, data);
            state.setTrue(ComponentInfos<T>::id);
        };

        template<typename T>
        void removeComp()
        {
            if(!hasComp<T>()) return;
            Component<T>::elements[ids[Component<T>::category]].markedForDeletion = true;
            state.setFalse(ComponentInfos<T>::id);
        };  
};

#include <memory>

typedef std::shared_ptr<Entity> EntityRef;
#define newEntity std::make_shared<Entity>

template <typename T>
void Component<T>::insert(Entity &entity, const T& data)
{
    int &id = entity.ids[category];
    if(id == NO_ENTITY)
    {
        int &lastFreeID = ComponentGlobals::lastFreeID[category];
        id = lastFreeID;
        int maxID = elements.size();
        while(elements[++lastFreeID].enabled && lastFreeID < maxID);
        ComponentGlobals::maxID[category] = std::max(ComponentGlobals::maxID[category], lastFreeID);
    }

    elements[id] = {data, &entity, true, false, entity.ids[ENTITY_LIST]};
    elements[id].init();
};


#include <functional>

template<typename ...T>
void System(std::function<void(Entity&)> f)
{
    static VulpineBitSet<MAX_COMP> mask(ComponentInfos<T>::id ...);

    // for(auto &i : Component<EntityInfos>::elements)
    //     if(i.enabled && i.entity->state == mask)
    //         f(*i.entity);

    auto &list = Component<EntityInfos>::elements;
    int maxID = ComponentGlobals::maxID[ENTITY_LIST];
    int size = list.size();
    for(int i = 0; i < size && i < maxID; i++)
        if(list[i].enabled && list[i].entity->state == mask)
            f(*list[i].entity);
};

template<typename T>
void ManageGarbage()
{   
    auto &list = Component<T>::elements;
    auto &elist = Component<EntityInfos>::elements;
    int maxID = ComponentGlobals::maxID[Component<T>::category];
    int size = list.size();

    for(int i = 0; i < size && i <= maxID; i++)
    {        

        if(list[i].markedForDeletion || (list[i].enabled && !elist[list[i].entityListID].enabled))
        {
            list[i].enabled = false;
            list[i].markedForDeletion = false;
            list[i].clean();
            list[i].data = T();
        }
    }
};

template<typename T>
struct GlobalComponentToggler
{
    static inline bool activated = false;

    template<typename C>
    static void update(C &entityList)
    {
        if(activated)
        {
            for(EntityRef &e : entityList)
                if(!e->hasComp<T>())
                    e->set<T>({});
        }
        else
        {
            for(EntityRef &e : entityList)
                if(e->hasComp<T>())
                    e->removeComp<T>();
            
            ManageGarbage<T>();
        }
    }
};


