#pragma once

#define __COMPONENT_DEFINE_ATTRIBUTE_FUNCTION(type, funcType, params) void __##funcType##__##type params

#define __COMPONENT_ADD_ATTRIBUTE(type, funcType, params) \
    __COMPONENT_DEFINE_ATTRIBUTE_FUNCTION(type, funcType, params); \
    inline PreLaunchVectorFill<ComponentModularity::funcType> __##funcType##SetupObject__##type(ComponentModularity::funcType##s, {(uint16_t)ComponentInfos<type>::id, __##funcType##__##type});

#define COMPONENT_ADD_SYNCH(type) __COMPONENT_ADD_ATTRIBUTE(type, SynchFunc, (Entity& parent, EntityRef child))
#define COMPONENT_DEFINE_SYNCH(type) __COMPONENT_DEFINE_ATTRIBUTE_FUNCTION(type, SynchFunc, (Entity& parent, EntityRef child))

#define COMPONENT_ADD_RW(type) \
    __COMPONENT_ADD_ATTRIBUTE(type, ReadFunc, (EntityRef entity, VulpineTextBuffRef buff)) \
    __COMPONENT_ADD_ATTRIBUTE(type, WriteFunc, (EntityRef entity, VulpineTextOutputRef out)) 

#define COMPONENT_DEFINE_READ(type) \
    __COMPONENT_DEFINE_ATTRIBUTE_FUNCTION(type, ReadFunc, (EntityRef entity, VulpineTextBuffRef buff)) 

#define COMPONENT_DEFINE_WRITE(type) \
    __COMPONENT_DEFINE_ATTRIBUTE_FUNCTION(type, WriteFunc, (EntityRef entity, VulpineTextOutputRef out))
