#include <ECS/ComponentTypeUI.hpp>
#include <Graphics/Scene.hpp>
#include <Globals.hpp>



WidgetBox& WidgetBox::set(vec2 xrange, vec2 yrange)
{
    lastMin = min;
    lastMax = max;
    lastChangeTime = globals.appTime.getElapsedTime();

    initMin = min = vec2(xrange[0], yrange[0]);
    initMax = max = vec2(xrange[1], yrange[1]);

    return *this;
}

