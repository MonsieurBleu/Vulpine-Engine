#pragma once 

#include <Graphics/ObjectGroup.hpp>
#include <Graphics/FastUI.hpp>
#include <functional>

#define UNINITIALIZED_FLOAT 1e12f

class Scene;

typedef FastUI_context* WidgetUI_Context;

class WidgetSprite
{
    public : 

        WidgetSprite(){};
        WidgetSprite(const std::string& name) : name(name){};

        ModelRef sprite;
        std::string name;

        Scene *scene = nullptr;
};

struct WidgetState
{
    bool upToDate = false;
    ModelStatus status = ModelStatus::UNDEFINED;
    ModelStatus statusToPropagate = ModelStatus::UNDEFINED;
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
        // displayMin(UNINITIALIZED_FLOAT),
        // displayMax(UNINITIALIZED_FLOAT)
        displayMin(0),
        displayMax(0)
        {};
    
    vec2 min;
    vec2 max;
    vec2 initMin;
    vec2 initMax;

    vec2 childrenMin;
    vec2 childrenMax;

    vec2 displayMin;
    vec2 displayMax;
    vec2 lastMin;
    vec2 lastMax;

    float depth = 0.f;

    float lastChangeTime = 0.f;

    void set(vec2 xrange, vec2 yrange);
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
    SimpleUiTileBatchRef batch;
};

struct WidgetButton 
{
    typedef std::function<void(float value)> InteractFunc;
    typedef std::function<float()> UpdateFunc;

    enum Type : uint8
    {
        HIDE_SHOW_TRIGGER, 
        CHECKBOX, 
        TEXT_INPUT, 
        SLIDER
    } type;

    WidgetButton(Type type = HIDE_SHOW_TRIGGER) : type(type){};

    WidgetButton(
        Type type, 
        InteractFunc valueChanged,
        UpdateFunc valueUpdate
        ) : type(type), valueChanged(valueChanged), valueUpdate(valueUpdate){};

    float cur = 0;
    float min = 0;
    float max = 1;
    float padding = 1;

    void* usr = nullptr;

    InteractFunc valueChanged;
    UpdateFunc valueUpdate;

    MeshMaterial material;
};

struct WidgetStyle
{
    #define ChainedMember(classt, type, name, defval) \
        type name = defval; \
        classt & set##name(const type & new_##name = defval){name = new_##name; return *this;};


    ChainedMember(WidgetStyle, vec4, textColor1, vec4(vec3(0.85), 1))
    ChainedMember(WidgetStyle, vec4, textColor2, vec4(vec3(0.85), 1))
    ChainedMember(WidgetStyle, vec4, backgroundColor1, vec4(vec3(0.35), 0.9))
    ChainedMember(WidgetStyle, vec4, backgroundColor2, vec4(vec3(0.125), 0.9))
    ChainedMember(WidgetStyle, UiTileType, backGroundStyle, UiTileType::SQUARE)
    ChainedMember(WidgetStyle, int, automaticTabbing, 0)

    bool useAltBackgroundColor = false;
    bool useAltTextColor = false;
};
