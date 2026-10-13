#pragma once

// #ifndef VULPINE_COMPONENT_IMPL
// #include <ECS/EngineComponents.hpp>
// #endif

#include <Timer.hpp>
#include <Helpers.hpp>

#define UI_BASE_COMP VulpineBlueprintUI::UIcontext, WidgetState(), WidgetBox()

namespace VulpineColorUI
{
    #define BASE_ALPHA 0.95f
    #define ALPHA2     0.5f

    inline vec4 LightBackgroundColor1       = vec4(vec3("#f2eade"_rgb), BASE_ALPHA);
    inline vec4 LightBackgroundColor2       = vec4(vec3("#f2eade"_rgb), ALPHA2);

    inline vec4 DarkBackgroundColor1        = vec4(0.75f*vec3("#463f3c"_rgb), BASE_ALPHA);
    inline vec4 DarkBackgroundColor1Opaque  = vec4(0.75f*vec3("#463f3c"_rgb), 1);    
    inline vec4 DarkBackgroundColor2        = vec4(0.25f*vec3("#353130"_rgb), BASE_ALPHA);
    inline vec4 DarkBackgroundColor2Opaque  = vec4(0.25f*vec3("#353130"_rgb), 1);

    inline vec4 HightlightColorOrange = vec4(vec3("#fd6706"_rgb), BASE_ALPHA);
    inline vec4 HightlightColorCyan = vec4(vec3("#2cd3af"_rgb), BASE_ALPHA);
    inline vec4 HightlightColorPink = vec4(vec3("#d92690"_rgb), BASE_ALPHA);
    inline vec4 HightlightColorYellow = vec4(vec3("#f9c019"_rgb), BASE_ALPHA);
    inline vec4 HightlightColorPurple = vec4(vec3("#aa3ce6"_rgb), BASE_ALPHA);
    inline vec4 HightlightColorGreen = vec4(vec3("#2cd35b"_rgb), BASE_ALPHA);
    inline vec4 HightlightColorRed = vec4(vec3("#d32c3f"_rgb), BASE_ALPHA);
    inline vec4 HightlightColorBlue = vec4(vec3("#368dfe"_rgb), BASE_ALPHA);
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
        WidgetButton::UpdateFunc ufunc,
        vec3 color = VulpineColorUI::LightBackgroundColor1
    );

    EntityRef Toggable2(
        const std::string &name,
        const std::string &icon,
        WidgetButton::InteractFunc ifunc, 
        WidgetButton::UpdateFunc ufunc,
        vec3 color = VulpineColorUI::LightBackgroundColor1
    );

    EntityRef ValueInput(
        const std::string &name,
        std::function<void(float f)> setValue, 
        std::function<float()> getValue,
        float minV, float maxV,
        float smallIncrement = 0.f, float bigIncrement = 0.f,
        vec4 color = VulpineColorUI::LightBackgroundColor1
        );

    EntityRef TextInput(
        const std::string &name,
        std::function<void(std::u32string &)> fromText, 
        std::function<std::u32string()> toText
        );

    EntityRef TextInput(
        const std::string &name
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
        vec4 color = VulpineColorUI::LightBackgroundColor1,
        bool vertical = false,
        float nameRatio = 0.5
    );

    EntityRef TimerPlot(
        BenchTimer &timer, 
        vec4(color),
        std::function<vec2(PlottingHelper*)> getMinmax
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
        float verticalLenghtReduction = 0.f,
        vec4 color = VulpineColorUI::LightBackgroundColor1,
        float nameSizeRatio = 0.f
    );

    EntityRef SimpleFolderView(
        const std::string name,
        std::vector<Entity*> & list,
        vec4 folderColor,
        vec4 fileColor,
        int columns,
        int rowsOnScreen,
        WidgetButton::InteractFunc ifunc, 
        WidgetButton::UpdateFunc ufunc,
        bool doSorting = true
    );

}