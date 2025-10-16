#include <ECS/ModularEntityGroupping.hpp>
#include <ECS/ComponentTypeUI.hpp>

#ifndef CURRENT_MAX_COMP_USAGE
// #undef CURRENT_MAX_COMP_USAGE
#define CURRENT_MAX_COMP_USAGE MAX_ENTITY
#endif

#undef CURRENT_CATEGORY
#define CURRENT_CATEGORY UI

/***************** ECS BASE VULPINE COMPONENTS *****************/

Component_Init(EntityGroupInfo)

/***************** UI BASE VULPINE COMPONENTS *****************/

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