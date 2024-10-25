#include <ECS/ModularEntityGroupping.hpp>
#include <ECS/ComponentTypeUI.hpp>

#undef CURRENT_MAX_COMP_USAGE
#define CURRENT_MAX_COMP_USAGE MAX_ENTITY

#undef CURRENT_CATEGORY
#define CURRENT_CATEGORY UI

/***************** UI BASE VULPINE COMPONENTS *****************/

Ephemeral_Component(WidgetModel)
template<> void Component<WidgetModel>::ComponentElem::init();
template<> void Component<WidgetModel>::ComponentElem::clean();

Ephemeral_Component(is2D)

