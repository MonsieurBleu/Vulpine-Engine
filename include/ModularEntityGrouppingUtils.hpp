#pragma once

#define __COMPONENT_DEFINE_ATTRIBUTE_FUNCTION(type, funcType, params) void __##funcType##__##type params

#define __COMPONENT_ADD_ATTRIBUTE(type, funcType, params) \
    __COMPONENT_DEFINE_ATTRIBUTE_FUNCTION(type, funcType, params); \
    inline PreLaunchVectorFill<ComponentModularity::funcType> \
        __##funcType##SetupObject__##type( \
            ComponentModularity::funcType##s, \
            { \
                (uint16)__ComponentIDSetupID__##type, \
                __##funcType##__##type \
            }); 


    // inline PreLaunchVectorFill<ComponentModularity::funcType> __##funcType##SetupObject__##type(ComponentModularity::funcType##s, {(uint16_t)ComponentInfos<type>::id, __##funcType##__##type});



// ComponentGlobals::ComponentNamesMap

#define COMPONENT_ADD_RW(type) \
    __COMPONENT_ADD_ATTRIBUTE(type, ReadFunc, (EntityRef entity, VulpineTextBuffRef buff)) \
    __COMPONENT_ADD_ATTRIBUTE(type, WriteFunc, (EntityRef entity, VulpineTextOutputRef out)) 

#define COMPONENT_DEFINE_READ(type) \
    __COMPONENT_DEFINE_ATTRIBUTE_FUNCTION(type, ReadFunc, (EntityRef entity, VulpineTextBuffRef buff)) 

#define COMPONENT_DEFINE_WRITE(type) \
    __COMPONENT_DEFINE_ATTRIBUTE_FUNCTION(type, WriteFunc, (EntityRef entity, VulpineTextOutputRef out))


#define COMPONENT_ADD_SYNCH(type) __COMPONENT_ADD_ATTRIBUTE(type, SynchFunc, (Entity& parent, EntityRef child))
#define COMPONENT_DEFINE_SYNCH(type) __COMPONENT_DEFINE_ATTRIBUTE_FUNCTION(type, SynchFunc, (Entity& parent, EntityRef child))


#define COMPONENT_ADD_REPARENT(type) __COMPONENT_ADD_ATTRIBUTE(type, ReparFunc, (Entity& parent, EntityRef child, Entity& newParent))
#define COMPONENT_DEFINE_REPARENT(type) __COMPONENT_DEFINE_ATTRIBUTE_FUNCTION(type, ReparFunc, (Entity& parent, EntityRef child, Entity& newParent))

#define COMPONENT_ADD_COMPATIBILITY_CHECK(type) __COMPONENT_ADD_ATTRIBUTE(type, CheckFunc, (Entity& parent, EntityRef child))
#define COMPONENT_DEFINE_COMPATIBILITY_CHECK(type) __COMPONENT_DEFINE_ATTRIBUTE_FUNCTION(type, CheckFunc, (Entity& parent, EntityRef child))

#define COMPONENT_ADD_MERGE(type) __COMPONENT_ADD_ATTRIBUTE(type, MergeFunc, (Entity& parent, EntityRef child))
#define COMPONENT_DEFINE_Merge(type) __COMPONENT_DEFINE_ATTRIBUTE_FUNCTION(type, MergeFunc, (Entity& parent, EntityRef child))

