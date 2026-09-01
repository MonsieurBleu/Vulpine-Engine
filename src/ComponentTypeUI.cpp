#include <ECS/ComponentTypeUI.hpp>
#include <Graphics/Scene.hpp>
#include <Globals.hpp>



WidgetBox& WidgetBox::set(vec2 xrange, vec2 yrange)
{
    // if(initMin.x == xrange.x and initMin.y == yrange.x and initMax.x == xrange.y and initMax.y == yrange.y)
        // return *this;

    // lastMin = displayMin;
    // lastMax = displayMax;
    lastChangeTime = globals.appTime.getElapsedTime();

    // initMin = min = vec2(xrange[0], yrange[0]);
    // initMax = max = vec2(xrange[1], yrange[1]);

    initMin = min = vec2(xrange[0], yrange[0]);
    initMax = max = vec2(xrange[1], yrange[1]);

    return *this;
}

