#pragma once

// #ifndef VULPINE_COMPONENT_IMPL
// #include <ECS/EngineComponents.hpp>
// #endif

#include <Timer.hpp>

#define UI_BASE_COMP VulpineBlueprintUI::UIcontext, WidgetState()

namespace VulpineColorUI
{
    #define BASE_ALPHA 0.95f*255.f
    #define ALPHA2     0.8f*255.f

    inline vec4 LightBackgroundColor1 = vec4(242, 234,  222, BASE_ALPHA)/255.f;
    inline vec4 LightBackgroundColor2 = vec4(242, 234,  222, ALPHA2)/255.f;

    // inline vec4 LightBackgroundColor1 = vec4(vec3(.1), BASE_ALPHA/255.);
    // inline vec4 LightBackgroundColor2 = vec4(vec3(.1), ALPHA2/255.);

    // inline vec4 DarkBackgroundColor1  = vec4(1.);
    // inline vec4 DarkBackgroundColor1Opaque  = vec4(1.);
    // inline vec4 DarkBackgroundColor2  = vec4(1.);
    // inline vec4 DarkBackgroundColor2Opaque  = vec4(1.);

    inline vec4 DarkBackgroundColor1  = vec4( 70,  63,  60, BASE_ALPHA)/255.f;
    inline vec4 DarkBackgroundColor1Opaque  = vec4( 70,  63,  60, 255)/255.f;    
    inline vec4 DarkBackgroundColor2  = 0.5f*vec4( 53,  49,  48, BASE_ALPHA)/255.f;
    inline vec4 DarkBackgroundColor2Opaque  = 0.5f*vec4( 53,  49,  48, 255 * 2)/255.f;

    inline vec4 HightlightColor1 = vec4(253, 103,  6,   BASE_ALPHA)/255.f;
    inline vec4 HightlightColor2 = vec4(44, 211,  175,  BASE_ALPHA)/255.f;
    inline vec4 HightlightColor3 = vec4(217, 38,  144,  BASE_ALPHA)/255.f;
    inline vec4 HightlightColor4 = vec4(249, 192,  25,  BASE_ALPHA)/255.f;
    inline vec4 HightlightColor5 = vec4(170, 60,   230, BASE_ALPHA)/255.f;
}   

namespace VulpineBlueprintUI
{
    inline WidgetUI_Context UIcontext;

    extern WidgetBox::FittingFunc SmoothSliderFittingFunc;

    EntityRef SmoothSlider(
        const std::string &name,
        float min, float max, int padding, 
        WidgetButton::InteractFunc ifunc, 
        WidgetButton::UpdateFunc ufunc,
        vec4 color = VulpineColorUI::LightBackgroundColor1
        );

        EntityRef Toggable(
            const std::string &name,
            const std::string &icon,
            WidgetButton::InteractFunc ifunc, 
            WidgetButton::UpdateFunc ufunc
        );

        EntityRef ValueInput(
            const std::string &name,
            std::function<void(float f)> setValue, 
            std::function<float()> getValue,
            float minV, float maxV,
            float smallIncrement, float bigIncrement
            );

        EntityRef TextInput(
            const std::string &name,
            std::function<void(std::u32string &)> fromText, 
            std::function<std::u32string()> toText
            );

        EntityRef ValueInputSlider(
            const std::string &name,
            float min, float max, int padding, 
            WidgetButton::InteractFunc ifunc, 
            WidgetButton::UpdateFunc ufunc,
            std::function<void(std::u32string &)> fromText, 
            std::function<std::u32string()> toText,
            vec4 color = VulpineColorUI::LightBackgroundColor1
            );
        
        EntityRef ValueInputSlider(
            const std::string &name,
            float min, float max, int padding, 
            std::function<void(float f)> setValue, 
            std::function<float()> getValue,
            vec4 color = VulpineColorUI::LightBackgroundColor1
            );

        EntityRef ColorSelectionScreen(
            const std::string &name,
            std::function<vec3()> getColor, 
            std::function<void(vec3)> setColor
        );

        EntityRef NamedEntry(
            const std::u32string &name,
            EntityRef entry,
            float nameRatioSize = 0.5f,
            bool vertical = false,
            vec4 color = VulpineColorUI::LightBackgroundColor1
        );


        EntityRef ColoredConstEntry(
            const std::string &name,
            std::function<std::u32string()> toText,
            vec4 color = VulpineColorUI::LightBackgroundColor1
        );

        EntityRef TimerPlot(
            BenchTimer &timer, 
            vec4(color),
            std::function<vec2()> getMinmax
        );

        void AddToSelectionMenu(
            EntityRef titlesParent, 
            EntityRef infosParent,  
            EntityRef info,
            const std::string &name,
            const std::string &icon = ""
        );
    
        EntityRef SceneInfos(Scene& scene);


        EntityRef StringListSelectionMenu(
            const std::string &name,
            std::unordered_map<std::string, EntityRef>& list,
            WidgetButton::InteractFunc ifunc, 
            WidgetButton::UpdateFunc ufunc,
            float verticalLenghtReduction = 0.f
        );

}