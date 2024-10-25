#pragma once 

#include <Graphics/ObjectGroup.hpp>

class Scene;

class WidgetModel : public ObjectGroupRef
{
    public : 
        WidgetModel();
        WidgetModel(Scene * s);
        Scene* scene = nullptr;
};

struct is2D
{
    bool d;
};




