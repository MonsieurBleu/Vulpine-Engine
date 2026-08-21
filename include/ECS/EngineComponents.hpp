#include <ECS/ModularEntityGroupping.hpp>
#include <ECS/ComponentTypeUI.hpp>
#include <ECS/ComponentTypeAudio.hpp>
#include <ECS/ComponentType3D.hpp>

#ifndef CURRENT_MAX_COMP_USAGE
// #undef CURRENT_MAX_COMP_USAGE
#define CURRENT_MAX_COMP_USAGE MAX_ENTITY
#endif

// Change the entity's State3D while updating physics and 3D model in a trhead-safe and game aware way.
// Use this if you want to teleport/rotate/(de)activate any entity.
// NOTE : never give the entity State3D directly, to avoid any multithreading issue.
void setEntityTransform(Entity &e, State3D s);
// ^ For now it's writen in game code, but everything here can be moved when EntityModel is moved to Engine code

/***************** ECS BASE VULPINE COMPONENTS *****************/

#undef CURRENT_CATEGORY
#define CURRENT_CATEGORY DATA

Component_Init(EntityGroupInfo)
Component_Init(State3D)
Component_Synch(HierarchyState3D)
template<> void Component<HierarchyState3D>::ComponentElem::init();

/***************** ECS BASE DYNAMIC VULPINE COMPONENTS *****************/

#undef CURRENT_CATEGORY
#define CURRENT_CATEGORY DATA_DYNAMIC

Component(DynamicState3D)
Component(Movement)
Component(ComplexMovements)
Component(Gauges)

/***************** UI BASE VULPINE COMPONENTS *****************/

#undef CURRENT_CATEGORY
#define CURRENT_CATEGORY UI

static inline EntityRef cursorHelp;

/* TODO : remove ?*/
Component(WidgetSprite)
template<> void Component<WidgetSprite>::ComponentElem::init();
template<> void Component<WidgetSprite>::ComponentElem::clean();

Component_Synch(WidgetBox)

Component_Synch(WidgetState)

Component(WidgetStyle)

Component(WidgetText)
template<> void Component<WidgetText>::ComponentElem::init();
template<> void Component<WidgetText>::ComponentElem::clean();

Component(WidgetBackground)
template<> void Component<WidgetBackground>::ComponentElem::init();
template<> void Component<WidgetBackground>::ComponentElem::clean();

Component(WidgetButton)

Component(WidgetUI_Context)

/***************** UI BASE VULPINE SYSTEMS *****************/

void updateEntityCursor(
    vec2 screenPos, 
    bool down, 
    bool click, 
    WidgetUI_Context& ui,
    bool showEntityUnderCursorHint = true
);

void updateWidgetsStyle(); 

/***************** AUDIO VULPINE SYSTEMS *****************/

#undef CURRENT_CATEGORY
#define CURRENT_CATEGORY SOUND

Component(AudioPlayer);
Component(AudioScatterer);

/***************** PHYSICS VULPINE SYSTEMS *****************/
#ifdef ECS_JOLT_PHYSICS_INTEGRATION

    #include <JoltIntegration/JoltIntegration.hpp>

    #undef CURRENT_CATEGORY
    #define CURRENT_CATEGORY PHYSIC
    Component(JoltBody)
    template<> void Component<JoltBody>::ComponentElem::clean();
    
    #undef CURRENT_CATEGORY
    #define CURRENT_CATEGORY PHYSIC_DYNAMIC
    Component(KynematicFlag)

#endif


