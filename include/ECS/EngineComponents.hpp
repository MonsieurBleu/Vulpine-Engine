#include <ECS/ModularEntityGroupping.hpp>
#include <ECS/ComponentTypeUI.hpp>

#ifndef CURRENT_MAX_COMP_USAGE
// #undef CURRENT_MAX_COMP_USAGE
#define CURRENT_MAX_COMP_USAGE MAX_ENTITY
#endif

#undef CURRENT_CATEGORY
#define CURRENT_CATEGORY UI

/***************** ECS BASE VULPINE COMPONENTS *****************/

Aligned_Component(EntityGroupInfo)

/***************** UI BASE VULPINE COMPONENTS *****************/

/* TODO : remove ?*/
Ephemeral_Component(WidgetSprite)
template<> void Component<WidgetSprite>::ComponentElem::init();
template<> void Component<WidgetSprite>::ComponentElem::clean();

Adaptive_Component(WidgetBox)

Adaptive_Component(WidgetState)

Ephemeral_Component(WidgetStyle)

Ephemeral_Component(WidgetText)
template<> void Component<WidgetText>::ComponentElem::init();
template<> void Component<WidgetText>::ComponentElem::clean();

Ephemeral_Component(WidgetBackground)
template<> void Component<WidgetBackground>::ComponentElem::init();
template<> void Component<WidgetBackground>::ComponentElem::clean();

Ephemeral_Component(WidgetButton)

Ephemeral_Component(WidgetUI_Context)


/***************** UI BASE VULPINE SYSTEMS *****************/

void updateEntityCursor(
    vec2 screenPos, 
    bool down, 
    bool click, 
    WidgetUI_Context& ui,
    bool showEntityUnderCursorHint = true
);

void updateWidgetsStyle();