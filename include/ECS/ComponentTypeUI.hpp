#pragma once 

#include <Graphics/ObjectGroup.hpp>
#include <Graphics/FastUI.hpp>
#include <functional>

#define UNINITIALIZED_FLOAT 1e12f

class Scene;

typedef FastUI_context* WidgetUI_Context;

class WidgetModel
{
    public : 
        WidgetModel();
        WidgetModel(Scene * s);
        Scene* scene = nullptr;

        ObjectGroupRef model;
};

struct WidgetState
{
    bool upToDate = false;
    ModelStateHideStatus status = ModelStateHideStatus::UNDEFINED;
    ModelStateHideStatus statusToPropagate = ModelStateHideStatus::UNDEFINED;
};

struct WidgetBox
{
    enum Type : uint8
    {
        FOLLOW_PARENT_BOX, FOLLOW_SIBLINGS_BOX
    } type;

    WidgetBox(
        vec2 xrange = vec2(-1, 1), 
        vec2 yrange = vec2(-1, 1),
        Type type = WidgetBox::Type::FOLLOW_PARENT_BOX
        ) : 
        type(type),
        min(xrange[0], yrange[0]), 
        max(xrange[1], yrange[1]), 
        initMin(min), 
        initMax(max),
        childrenMin(min),
        childrenMax(max),
        displayMin(UNINITIALIZED_FLOAT),
        displayMax(UNINITIALIZED_FLOAT)
        {};
    
    vec2 min;
    vec2 max;
    vec2 initMin;
    vec2 initMax;

    vec2 childrenMin;
    vec2 childrenMax;

    vec2 displayMin;
    vec2 displayMax;

    float lastChangeTime = 0.f;

};

struct WidgetText
{
    WidgetText(){};
    WidgetText(std::u32string newText) : text(newText){};
    SingleStringBatchRef mesh;
    std::u32string text;
};

struct WidgetBackground
{
    SimpleUiTileRef tile;
};

struct WidgetButton 
{
    enum Type : uint8
    {
        HIDE_SHOW_TRIGGER, TEXT_INPUT, CHECKBOX, SLIDER
    } type;

    WidgetButton(Type type = HIDE_SHOW_TRIGGER) : type(type){};

    double min = 0;
    double max = 100;
    double padding = 1;

    void* usr = nullptr;

    std::function<void(double value)> valueChanged;

    MeshMaterial material;
};



