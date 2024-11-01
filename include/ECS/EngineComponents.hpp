#include <ECS/ModularEntityGroupping.hpp>
#include <ECS/ComponentTypeUI.hpp>

#undef CURRENT_MAX_COMP_USAGE
#define CURRENT_MAX_COMP_USAGE 1024

#undef CURRENT_CATEGORY
#define CURRENT_CATEGORY UI

/***************** UI BASE VULPINE COMPONENTS *****************/

/* TODO : remove ?*/
Ephemeral_Component(WidgetModel)
template<> void Component<WidgetModel>::ComponentElem::init();
template<> void Component<WidgetModel>::ComponentElem::clean();

Adaptive_Component(WidgetBox)

Adaptive_Component(WidgetState)

Ephemeral_Component(WidgetText)
template<> void Component<WidgetText>::ComponentElem::init();

Ephemeral_Component(WidgetBackground)
template<> void Component<WidgetBackground>::ComponentElem::init();

Ephemeral_Component(WidgetButton)

Ephemeral_Component(WidgetUI_Context)


/***************** UI BASE VULPINE SYSTEMS *****************/

void updateEntityCursor(vec2 screenPos, bool down, bool click);

