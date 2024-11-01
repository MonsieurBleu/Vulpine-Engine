#include <ECS/ComponentTypeUI.hpp>
#include <Graphics/Scene.hpp>
#include <Globals.hpp>

WidgetModel::WidgetModel()
{
    scene = globals.getScene2D();
}

WidgetModel::WidgetModel(Scene * s)
{
    assert(s != nullptr);
    scene = s;
}




