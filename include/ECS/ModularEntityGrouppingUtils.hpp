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


#define Ephemeral_Component(type)                               \
    COMPONENT(type, CURRENT_CATEGORY, CURRENT_MAX_COMP_USAGE)   \

#define Adaptive_Component(type)                                \
    COMPONENT(type, CURRENT_CATEGORY, CURRENT_MAX_COMP_USAGE)   \
    COMPONENT_ADD_SYNCH(type)                                   \

#define Aligned_Component(type)                                 \
    COMPONENT(type, CURRENT_CATEGORY, CURRENT_MAX_COMP_USAGE)   \
    COMPONENT_ADD_REPARENT(type)                                \

#define Attached_Component(type)                                \
    COMPONENT(type, CURRENT_CATEGORY, CURRENT_MAX_COMP_USAGE)   \
    COMPONENT_ADD_REPARENT(type)                                \
    COMPONENT_ADD_SYNCH(type)                                   \

#define Coherent_Component(type)                                \
    COMPONENT(type, CURRENT_CATEGORY, CURRENT_MAX_COMP_USAGE)   \
    COMPONENT_ADD_COMPATIBILITY_CHECK(type)                     \

#define Contextual_Component(type)                              \
    COMPONENT(type, CURRENT_CATEGORY, CURRENT_MAX_COMP_USAGE)   \
    COMPONENT_ADD_COMPATIBILITY_CHECK(type)                     \
    COMPONENT_ADD_REPARENT(type)                                \

#define Consonant_Component(type)                               \
    COMPONENT(type, CURRENT_CATEGORY, CURRENT_MAX_COMP_USAGE)   \
    COMPONENT_ADD_COMPATIBILITY_CHECK(type)                     \
    COMPONENT_ADD_SYNCH(type)                                   \

#define Convergent_Component(type)                              \
    COMPONENT(type, CURRENT_CATEGORY, CURRENT_MAX_COMP_USAGE)   \
    COMPONENT_ADD_COMPATIBILITY_CHECK(type)                     \
    COMPONENT_ADD_REPARENT(type)                                \
    COMPONENT_ADD_SYNCH(type)                                   \

#define Intrinsic_Component(type)                               \
    COMPONENT(type, CURRENT_CATEGORY, CURRENT_MAX_COMP_USAGE)   \
    COMPONENT_ADD_MERGE(type)                                   \

#define Intricate_Component(type)                               \
    COMPONENT(type, CURRENT_CATEGORY, CURRENT_MAX_COMP_USAGE)   \
    COMPONENT_ADD_MERGE(type)                                   \
    COMPONENT_ADD_COMPATIBILITY_CHECK(type)                     \

#define Aligned_Intrinsic_Component(type)                       \
    COMPONENT(type, CURRENT_CATEGORY, CURRENT_MAX_COMP_USAGE)   \
    COMPONENT_ADD_MERGE(type)                                   \
    COMPONENT_ADD_REPARENT(type)                                \

#define Aligned_Intricate_Component(type)                       \
    COMPONENT(type, CURRENT_CATEGORY, CURRENT_MAX_COMP_USAGE)   \
    COMPONENT_ADD_MERGE(type)                                   \
    COMPONENT_ADD_COMPATIBILITY_CHECK(type)                     \
    COMPONENT_ADD_REPARENT(type)                                \

#define Seamless_Resonant_Component(type)                       \
    COMPONENT(type, CURRENT_CATEGORY, CURRENT_MAX_COMP_USAGE)   \
    COMPONENT_ADD_MERGE(type)                                   \
    COMPONENT_ADD_SYNCH(type)                                   \

#define Coherent_Resonant_Component(type)                       \
    COMPONENT(type, CURRENT_CATEGORY, CURRENT_MAX_COMP_USAGE)   \
    COMPONENT_ADD_MERGE(type)                                   \
    COMPONENT_ADD_COMPATIBILITY_CHECK(type)                     \
    COMPONENT_ADD_SYNCH(type)                                   \

#define Aligned_Resonant_Component(type)                        \
    COMPONENT(type, CURRENT_CATEGORY, CURRENT_MAX_COMP_USAGE)   \
    COMPONENT_ADD_MERGE(type)                                   \
    COMPONENT_ADD_REPARENT(type)                                \
    COMPONENT_ADD_SYNCH(type)                                   \

#define Contextual_Resonant_Component(type)                     \
    COMPONENT(type, CURRENT_CATEGORY, CURRENT_MAX_COMP_USAGE)   \
    COMPONENT_ADD_MERGE(type)                                   \
    COMPONENT_ADD_COMPATIBILITY_CHECK(type)                     \
    COMPONENT_ADD_REPARENT(type)                                \
    COMPONENT_ADD_SYNCH(type)                                   \

