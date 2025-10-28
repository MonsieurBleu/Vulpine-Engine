#pragma once

#define __COMPONENT_DEFINE_ATTRIBUTE_FUNCTION(returnType, type, funcType, params) returnType __##funcType##__##type params

#define __COMPONENT_ADD_ATTRIBUTE(returnType, type, funcType, params) \
    __COMPONENT_DEFINE_ATTRIBUTE_FUNCTION(returnType, type, funcType, params); \
    inline PreLaunchVectorFill<ComponentModularity::funcType> \
        __##funcType##SetupObject__##type( \
            ComponentModularity::funcType##s, \
            { \
                (uint16)__ComponentIDSetupID__##type, \
                __##funcType##__##type \
            }); 

#define COMPONENT_ADD_RW(type) \
    __COMPONENT_ADD_ATTRIBUTE(void, type, ReadFunc, (EntityRef entity, VulpineTextBuffRef buff)) \
    __COMPONENT_ADD_ATTRIBUTE(void, type, WriteFunc, (EntityRef entity, VulpineTextOutputRef out)) 

#define COMPONENT_DEFINE_READ(type) \
    __COMPONENT_DEFINE_ATTRIBUTE_FUNCTION(void, type, ReadFunc, (EntityRef entity, VulpineTextBuffRef buff)) 

#define COMPONENT_DEFINE_WRITE(type) \
    __COMPONENT_DEFINE_ATTRIBUTE_FUNCTION(void, type, WriteFunc, (EntityRef entity, VulpineTextOutputRef out))


#define COMPONENT_ADD_SYNCH(type) __COMPONENT_ADD_ATTRIBUTE(void, type, SynchFunc, (Entity& parent, EntityRef child))
#define COMPONENT_DEFINE_SYNCH(type) __COMPONENT_DEFINE_ATTRIBUTE_FUNCTION(void, type, SynchFunc, (Entity& parent, EntityRef child))

#define COMPONENT_ADD_REPARENT(type) __COMPONENT_ADD_ATTRIBUTE(void, type, ReparFunc, (Entity& parent, EntityRef child, Entity& newParent))
#define COMPONENT_DEFINE_REPARENT(type) __COMPONENT_DEFINE_ATTRIBUTE_FUNCTION(void, type, ReparFunc, (Entity& parent, EntityRef child, Entity& newParent))

#define COMPONENT_ADD_COMPATIBILITY_CHECK(type) __COMPONENT_ADD_ATTRIBUTE(bool, type, CheckFunc, (Entity& parent, EntityRef child))
#define COMPONENT_DEFINE_COMPATIBILITY_CHECK(type) __COMPONENT_DEFINE_ATTRIBUTE_FUNCTION(bool, type, CheckFunc, (Entity& parent, EntityRef child))

#define COMPONENT_ADD_MERGE(type) __COMPONENT_ADD_ATTRIBUTE(void, type, MergeFunc, (Entity& parent, EntityRef child))
#define COMPONENT_DEFINE_MERGE(type) __COMPONENT_DEFINE_ATTRIBUTE_FUNCTION(void, type, MergeFunc, (Entity& parent, EntityRef child))


#define Component(type)                                         \
    COMPONENT(type, CURRENT_CATEGORY, CURRENT_MAX_COMP_USAGE)   \

#define Component_Synch(type)                                   \
    COMPONENT(type, CURRENT_CATEGORY, CURRENT_MAX_COMP_USAGE)   \
    COMPONENT_ADD_SYNCH(type)                                   \

#define Component_Init(type)                                    \
    COMPONENT(type, CURRENT_CATEGORY, CURRENT_MAX_COMP_USAGE)   \
    COMPONENT_ADD_REPARENT(type)                                \

#define Component_Init_Synch(type)                              \
    COMPONENT(type, CURRENT_CATEGORY, CURRENT_MAX_COMP_USAGE)   \
    COMPONENT_ADD_REPARENT(type)                                \
    COMPONENT_ADD_SYNCH(type)                                   \

#define Component_Compatible(type)                              \
    COMPONENT(type, CURRENT_CATEGORY, CURRENT_MAX_COMP_USAGE)   \
    COMPONENT_ADD_COMPATIBILITY_CHECK(type)                     \

#define Component_Init_Compatible(type)                         \
    COMPONENT(type, CURRENT_CATEGORY, CURRENT_MAX_COMP_USAGE)   \
    COMPONENT_ADD_COMPATIBILITY_CHECK(type)                     \
    COMPONENT_ADD_REPARENT(type)                                \

#define Component_Synch_Compatible(type)                        \
    COMPONENT(type, CURRENT_CATEGORY, CURRENT_MAX_COMP_USAGE)   \
    COMPONENT_ADD_COMPATIBILITY_CHECK(type)                     \
    COMPONENT_ADD_SYNCH(type)                                   \

#define Component_Init_Synch_Compatible(type)                   \
    COMPONENT(type, CURRENT_CATEGORY, CURRENT_MAX_COMP_USAGE)   \
    COMPONENT_ADD_COMPATIBILITY_CHECK(type)                     \
    COMPONENT_ADD_REPARENT(type)                                \
    COMPONENT_ADD_SYNCH(type)                                   \

#define Component_Merge(type)                                   \
    COMPONENT(type, CURRENT_CATEGORY, CURRENT_MAX_COMP_USAGE)   \
    COMPONENT_ADD_MERGE(type)                                   \

#define Component_Merge_Compatible(type)                        \
    COMPONENT(type, CURRENT_CATEGORY, CURRENT_MAX_COMP_USAGE)   \
    COMPONENT_ADD_MERGE(type)                                   \
    COMPONENT_ADD_COMPATIBILITY_CHECK(type)                     \

#define Component_Init_Merge(type)                              \
    COMPONENT(type, CURRENT_CATEGORY, CURRENT_MAX_COMP_USAGE)   \
    COMPONENT_ADD_MERGE(type)                                   \
    COMPONENT_ADD_REPARENT(type)                                \

#define Component_Init_Merge_Compatible(type)                    \
    COMPONENT(type, CURRENT_CATEGORY, CURRENT_MAX_COMP_USAGE)   \
    COMPONENT_ADD_MERGE(type)                                   \
    COMPONENT_ADD_COMPATIBILITY_CHECK(type)                     \
    COMPONENT_ADD_REPARENT(type)                                \

#define Component_Synch_Merge(type)                             \
    COMPONENT(type, CURRENT_CATEGORY, CURRENT_MAX_COMP_USAGE)   \
    COMPONENT_ADD_MERGE(type)                                   \
    COMPONENT_ADD_SYNCH(type)                                   \

#define Component_Synch_Merge_Compatible(type)                   \
    COMPONENT(type, CURRENT_CATEGORY, CURRENT_MAX_COMP_USAGE)   \
    COMPONENT_ADD_MERGE(type)                                   \
    COMPONENT_ADD_COMPATIBILITY_CHECK(type)                     \
    COMPONENT_ADD_SYNCH(type)                                   \

#define Component_Init_Synch_Merge(type)                        \
    COMPONENT(type, CURRENT_CATEGORY, CURRENT_MAX_COMP_USAGE)   \
    COMPONENT_ADD_MERGE(type)                                   \
    COMPONENT_ADD_REPARENT(type)                                \
    COMPONENT_ADD_SYNCH(type)                                   \

#define Component_Init_Synch_Merge_Compatible(type)              \
    COMPONENT(type, CURRENT_CATEGORY, CURRENT_MAX_COMP_USAGE)   \
    COMPONENT_ADD_MERGE(type)                                   \
    COMPONENT_ADD_COMPATIBILITY_CHECK(type)                     \
    COMPONENT_ADD_REPARENT(type)                                \
    COMPONENT_ADD_SYNCH(type)                                   \

