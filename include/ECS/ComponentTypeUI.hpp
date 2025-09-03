#pragma once 

#include <Graphics/ObjectGroup.hpp>
#include <Graphics/FastUI.hpp>
#include <functional>

#define UNINITIALIZED_FLOAT 1e12f

#define ChainedMember(classt, type, name, defval) \
    type name = defval; \
    classt & set##name(const type & new_##name = defval){name = new_##name; return *this;};

class Scene;

typedef FastUI_context* WidgetUI_Context;

class WidgetSprite
{
    public : 

        WidgetSprite(){};
        WidgetSprite(const std::string& name) : name(name){};
        WidgetSprite(ModelRef sprite) : sprite(sprite){};

        ModelRef sprite;
        std::string name;

        Scene *scene = nullptr;
};

struct WidgetState
{
    bool upToDate = false;
    ModelStatus status = ModelStatus::UNDEFINED;
    ModelStatus statusToPropagate = ModelStatus::UNDEFINED;

    int updateCounter = 0;
};

class Entity;

struct WidgetBox
{
    static inline vec2 tabbingSpacingScale = vec2(1.);

    static inline float smoothingAnimationSpeed = 4.f;

    typedef std::function<void(Entity* parent, Entity* child)> FittingFunc;

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
        childrenMax(max)
        // displayMin(UNINITIALIZED_FLOAT),
        // displayMax(UNINITIALIZED_FLOAT)
        // displayMin(0),
        // displayMax(0)
        {};
    
    WidgetBox(
        FittingFunc specialFittingScript
    ) :
        type(FOLLOW_PARENT_BOX), 
        initMin(min), initMax(max),
        childrenMin(min),childrenMax(max),
        // displayMin(0), displayMax(0),
        specialFittingScript(specialFittingScript)
    {};
    
    vec2 min = vec2(-1, -1);
    vec2 max = vec2(1, 1);
    vec2 initMin;
    vec2 initMax;

    vec2 childrenMin;
    vec2 childrenMax;

    vec2 displayMin = vec2(UNINITIALIZED_FLOAT);
    vec2 displayMax = vec2(UNINITIALIZED_FLOAT);
    vec2 lastMin;
    vec2 lastMax;

    vec2 displayRangeMin = vec2(-UNINITIALIZED_FLOAT);
    vec2 displayRangeMax = vec2(UNINITIALIZED_FLOAT);

    float depth = 0.f;

    float lastChangeTime = 0.f;

    bool useClassicInterpolation = false;

    bool isUnderCursor = false;
    bool areChildrenUnderCurosor = false;

    vec2 scrollOffset = vec2(0);

    WidgetBox& set(vec2 xrange, vec2 yrange);

    FittingFunc specialFittingScript;

};

struct WidgetText
{
    WidgetText(){};
    WidgetText(std::u32string newText, StringAlignement align = StringAlignement::CENTERED) : text(newText), align(align){};
    SingleStringBatchRef mesh;
    std::u32string text;
    StringAlignement align;
};

struct WidgetBackground
{
    SimpleUiTileRef tile;
    SimpleUiTileBatchRef batch;
};

struct WidgetButton 
{
    typedef std::function<void(Entity *caller, float value)> InteractFunc;
    typedef std::function<float(Entity *caller)> UpdateFunc;

    typedef std::function<void(Entity *caller, vec2 value)> InteractFunc2D;
    typedef std::function<vec2(Entity *caller)> UpdateFunc2D;

    enum Type : uint8
    {
        HIDE_SHOW_TRIGGER, 
        HIDE_SHOW_TRIGGER_INDIRECT,
        CHECKBOX, 
        TEXT_INPUT, 
        SLIDER,
        SLIDER_2D
    } type;

    WidgetButton(Type type = HIDE_SHOW_TRIGGER) : type(type){};

    WidgetButton(
        Type type, 
        InteractFunc valueChanged,
        UpdateFunc valueUpdate
        ) : type(type), valueChanged(valueChanged), valueUpdate(valueUpdate){};

    WidgetButton(
        Type type, 
        InteractFunc2D valueChanged2D,
        UpdateFunc2D valueUpdate2D
        ) : type(type), valueChanged2D(valueChanged2D), valueUpdate2D(valueUpdate2D){};

    ChainedMember(WidgetButton, float, cur, 0)
    ChainedMember(WidgetButton, float, cur2, 0)
    ChainedMember(WidgetButton, float, min, 0)
    ChainedMember(WidgetButton, float, max, 1)
    ChainedMember(WidgetButton, float, padding, 1e3f)
    ChainedMember(WidgetButton, uint64, usr, 0)

    InteractFunc valueChanged;
    UpdateFunc valueUpdate;

    InteractFunc2D valueChanged2D;
    UpdateFunc2D valueUpdate2D;

    MeshMaterial material;
};

struct WidgetStyle
{
    ChainedMember(WidgetStyle, vec4, textColor1, vec4(vec3(0.85), 1))
    ChainedMember(WidgetStyle, vec4, textColor2, vec4(0.85, 0.85, 0.25, 1))
    ChainedMember(WidgetStyle, vec4, backgroundColor1, vec4(vec3(0.35), 0.9))
    ChainedMember(WidgetStyle, vec4, backgroundColor2, vec4(vec3(0.125), 0.9))
    ChainedMember(WidgetStyle, UiTileType, backGroundStyle, UiTileType::SQUARE)
    ChainedMember(WidgetStyle, int, automaticTabbing, 0)
    ChainedMember(WidgetStyle, float, spriteScale, 1)

    ChainedMember(WidgetStyle, bool, useInternalSpacing, false)

    ChainedMember(WidgetStyle, vec2, spritePosition, vec2(0))

    bool useAltBackgroundColor = false;
    bool useAltTextColor = false;
};

