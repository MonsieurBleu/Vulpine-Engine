#pragma once 

#ifndef ENTITY_HEADER_SAFE_INCLUDE
    static_assert(
        false, 
        "Entity.hpp included outside of the safe environement. Please, only include this header once inside the game's header ECS definition. Always include the game version of the ECS, not this one, no matter what."
    );
#endif

#include <MappedEnum.hpp>
#include <VulpineBitSet.hpp>
#include <array>
#include <vector>
#include <cassert>

#include <Utils.hpp>

#ifdef VULPINE_COMPONENT_IMPL
#include <sstream>
#endif

#ifdef VULPINE_COMPONENT_IMPL
#define PreLaunchContainerFill(_containerType_, _name_) \
    template<typename T> \
    class PreLaunch##_name_##Fill \
    { \
        public : PreLaunchVectorFill(_containerType_<T> &v, T i){v.push_back(i);}; \
    }; 
#else
#define PreLaunchContainerFill(_containerType_, _name_) \
    template<typename T> \
    class PreLaunch##_name_##Fill \
    { \
        public : PreLaunchVectorFill(_containerType_<T> &v, T i); \
    }; 
#endif

template<typename T> class PreLaunchVectorFill { public : PreLaunchVectorFill(std::vector<T> &v, T i){v.push_back(i);}; };
template<typename T, typename D> class PreLaunchMapFill { public : PreLaunchMapFill(std::unordered_map<T, D> &v, const T &i, const D &j){v[i] = j;}; };

GENERATE_ENUM(ComponentCategory,
    ENTITY_LIST,
    DATA,       // Default category, plan data with no important dependencies in the engine
    UI,
    GRAPHIC,    // Components that contains graphic related elements, should only be manipulated from the main thread
    PHYSIC,     // Components that contains physic related elements, should be manipulated with the physic mutex in mind, or in the physics thread directly 
    SOUND,      // Components that contains Sound related elements
    AI,         // Components that contains AI related elements, should be manipulated with the AI mutex in mind, or in the AI thread directly 
    SCRIPTING,  // Components that contains Scripting related elements
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

struct CategoryStatus
{
    bool enabled = false;
};


struct ComponentGlobals
{


    static inline int lastID = 0;
    static inline int lastFreeID[ComponentCategory::END] = {0};
    static inline int maxID[ComponentCategory::END] = {0};

    static inline CategoryStatus status[ComponentCategory::END][MAX_ENTITY];

    static inline std::unordered_map<std::string, int> * ComponentNamesMap;
    static inline std::vector<std::string> ComponentNames;

    static std::unordered_map<std::string, int>& getCNmap()
    {
        if(!ComponentNamesMap)
            ComponentNamesMap = new std::unordered_map<std::string, int>;
        return *ComponentGlobals::ComponentNamesMap;
    };
};

template <typename T>
struct ComponentInfos
{
    #ifdef __clang__
    static inline const int size = 0;
    static inline const int id = 0;
    #elif defined(__GNUC__)
    static inline const int size;
    static inline const int id;
    #endif
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

            bool isGood(){return entity and !markedForDeletion and entityListID != -1;};
        };

        template<typename ... Args>
        T CopyElision(Args&&... args){return T(args ...);};

        #ifdef __clang__
        static inline const ComponentCategory category = ComponentCategory::ENTITY_LIST; 
        #elif defined(__GNUC__)
        static inline const ComponentCategory category; 
        #endif
        
        static inline std::array<ComponentElem, ComponentInfos<T>::size> elements;

        static void insert(Entity &entity, const T& data);      
};

#ifdef __clang__
#define COMPILER_INLINE_PREFIX inline
#elif defined(__GNUC__)
#define COMPILER_INLINE_PREFIX
#endif


#define COMPONENT_BASE(_type_, _category_, _size_) \
    static_assert(_size_ <= MAX_ENTITY); \
    inline const int __ComponentIDSetupID__##_type_ = ComponentGlobals::lastID++; \
    template<> COMPILER_INLINE_PREFIX const int ComponentInfos<_type_>::size = _size_;\
    template<> COMPILER_INLINE_PREFIX const ComponentCategory Component<_type_>::category = _category_; \
    template<> COMPILER_INLINE_PREFIX const int ComponentInfos<_type_>::id = __ComponentIDSetupID__##_type_;\
    inline PreLaunchVectorFill<std::string> __ComponentNamesSetupObject__##_type_(ComponentGlobals::ComponentNames, #_type_); \
    inline PreLaunchMapFill<std::string, int> __ComponentNamesMapSetupObject__##_type_(ComponentGlobals::getCNmap(), #_type_, __ComponentIDSetupID__##_type_);

#ifdef VULPINE_COMPONENT_IMPL
#define COMPONENT_IMPL(_type_) \
    template _type_ & Entity::comp<_type_>(); \
    template bool Entity::has<_type_>(); \
    template void Entity::set<_type_>(const _type_ & data); \
    template void Entity::remove<_type_>(); \
    

#define COMPONENT(_type_, _category_, _size_) COMPONENT_BASE(_type_, _category_, _size_) COMPONENT_IMPL(_type_)
#else
#define COMPONENT(_type_, _category_, _size_) COMPONENT_BASE(_type_, _category_, _size_) 
#endif

struct EntityInfos
{
    std::string name;
};

COMPONENT_BASE(EntityInfos, ENTITY_LIST, MAX_ENTITY);

struct templatePass{template<typename ...T> templatePass(T...) {}};

class Entity
{
    private : 

    public :

        VulpineBitSet<MAX_COMP> state;
        int ids[ComponentCategory::END] = {NO_ENTITY, NO_ENTITY, NO_ENTITY, NO_ENTITY, NO_ENTITY, NO_ENTITY, NO_ENTITY};

        Entity() : Entity("Unamed Entity"){};
        Entity(const std::string &name){set<EntityInfos>(EntityInfos{name});};

        template<typename ... components>
        Entity(const std::string &name, components&&... component) : Entity(name)
        {
            templatePass{(set(component), 0) ...};

            setChildrenInfos();
        };

        void setChildrenInfos();

        ~Entity();

        std::string toStr()
        {
            std::stringstream ss;
            ss << TERMINAL_OK << "Entity {";
            
            ss << TERMINAL_NOTIF << TERMINAL_UNDERLINE << comp<EntityInfos>().name << TERMINAL_RESET << " "; 
        
            for(auto &i : ComponentCategoryMap)
                if(i.second != ComponentCategory::END && ids[i.second] != -1)
                    ss << "  " << TERMINAL_FILENAME << i.first << "[" << TERMINAL_TIMER << ids[i.second] << TERMINAL_FILENAME << "]";

            ss << TERMINAL_OK << "}\n" << TERMINAL_RESET;
            return ss.str();
        }
        ;

        template<typename T>
        T& comp()
        #ifdef VULPINE_COMPONENT_IMPL
        {
            assert(ids[Component<T>::category] != NO_ENTITY);
            return Component<T>::elements[ids[Component<T>::category]].data;
        }
        #endif
        ;

        template<typename T>
        bool has()
        #ifdef VULPINE_COMPONENT_IMPL
        {
            return state[ComponentInfos<T>::id];
        }
        #endif
        ;

        template<typename T>
        void set(const T& data)
        #ifdef VULPINE_COMPONENT_IMPL
        {
            Component<T>::insert(*this, data);
            state.setTrue(ComponentInfos<T>::id);
        }
        #endif
        ;

        template<typename T>
        void remove()
        #ifdef VULPINE_COMPONENT_IMPL
        {
            if(!has<T>()) return;
            Component<T>::elements[ids[Component<T>::category]].markedForDeletion = true;
            state.setFalse(ComponentInfos<T>::id);
        }
        #endif
        ;  

        bool is(const Entity &other) const{return *this == other;};

        bool operator==(const Entity &other) const
        {
            for(int i = 0; i < ComponentCategory::END; i++)
                if(ids[i] != other.ids[i])
                    return false;
            
            return true;
        }
};

#include <memory>

typedef std::shared_ptr<Entity> EntityRef;
#define newEntity std::make_shared<Entity>

#ifdef VULPINE_COMPONENT_IMPL
template <typename T>
void Component<T>::insert(Entity &entity, const T& data)
{
    int &id = entity.ids[category];
    if(id == NO_ENTITY)
    {
        int &lastFreeID = ComponentGlobals::lastFreeID[category];
        id = lastFreeID;
        ComponentGlobals::status[category][id].enabled = true;
        int maxID = elements.size();

        // while(elements[++lastFreeID].enabled && lastFreeID < maxID);

        while(ComponentGlobals::status[category][++lastFreeID].enabled  && lastFreeID < maxID && lastFreeID < MAX_ENTITY);

        if(lastFreeID >= MAX_ENTITY-1)
            ERROR_MESSAGE("ECS has reached max Entity number of " ,  MAX_ENTITY ,  ". Things are goind to be bad!");

        ComponentGlobals::maxID[category] = std::max(ComponentGlobals::maxID[category], lastFreeID);

    }

    elements[id] = {data, &entity, true, false, entity.ids[ENTITY_LIST]};
    elements[id].init();
}
;
#endif


#include <functional>

template<typename ...T>
void System(std::function<void(Entity&)> f)
{
    static VulpineBitSet<MAX_COMP> mask(ComponentInfos<T>::id ...);

    auto &list = Component<EntityInfos>::elements;
    int maxID = ComponentGlobals::maxID[ENTITY_LIST];
    int size = list.size();
    for(int i = 0; i < size && i < maxID; i++)
        if(list[i].enabled && list[i].entity->state == mask)
            f(*list[i].entity);
}
;

template<typename T>
void ManageGarbage()
{   
    auto &list = Component<T>::elements;
    auto &elist = Component<EntityInfos>::elements;
    int maxID = ComponentGlobals::maxID[Component<T>::category];
    int size = list.size();

    for(int i = 0; 
    i < size 
    // && i <= maxID
    ; i++)
    {        
        #define NAME(type) #type
        
        if(list[i].markedForDeletion || (list[i].enabled && !elist[list[i].entityListID].enabled))
        {
            list[i].enabled = false;
            list[i].markedForDeletion = false;
            list[i].clean();
            list[i].data = T();
        }
    }
}
;

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
                if(!e->has<T>())
                    e->set<T>({});
        }
        else
        {
            for(EntityRef &e : entityList)
                if(e->has<T>())
                    e->remove<T>();
            
            ManageGarbage<T>();
        }
    }

    static void updateALL()
    {
        if(activated)
        {
            System<EntityInfos>([](Entity &entity){
                if(!entity.has<T>())
                    entity.set<T>({});
            });
        }
        else
        {
            System<EntityInfos>([](Entity &entity){
                if(entity.has<T>())
                    entity.remove<T>();
            });

            ManageGarbage<T>();
        }
    }

    static bool needUpdate()
    {
        if(activated)
        {
            bool uptodate = true;

            System<EntityInfos>([& uptodate](Entity &entity){
                uptodate &= entity.has<T>();
            });

            return !uptodate;
        }
        else
        {
            bool uptodate = true;

            System<EntityInfos>([& uptodate](Entity &entity){
                uptodate &= !entity.has<T>();
            });

            return !uptodate;
        }
    }
};


