

#include <Blueprint/EngineBlueprintUI.hpp>
#include <MathsUtils.hpp>
#include <Helpers.hpp>
#include <Constants.hpp>

WidgetBox::FittingFunc VulpineBlueprintUI::SmoothSliderFittingFunc = [](Entity* parent, Entity* child){ 
    auto &cbox = child->comp<WidgetBox>();
    auto &pbutton = parent->comp<WidgetButton>();

    cbox.initMin = vec2(-1);
    float v = (pbutton.cur - pbutton.min)/(pbutton.max - pbutton.min);

    vec2 tmpMax = cbox.initMax;
    cbox.initMax = vec2(v*2.f - 1, 1);

    cbox.depth = parent->comp<WidgetBox>().depth + 0.00001;

    cbox.useClassicInterpolation = tmpMax != cbox.initMax;

    return;
};


EntityRef VulpineBlueprintUI::SmoothSlider(
    const std::string &name,
    float min, float max, int padding, 
    WidgetButton::InteractFunc ifunc, 
    WidgetButton::UpdateFunc ufunc,
    vec4 color
    )
{
    return newEntity(name
        , UI_BASE_COMP
        , WidgetBackground()
        , WidgetBox()
        , WidgetStyle()
            .setbackGroundStyle(UiTileType::SQUARE_ROUNDED)
            .setbackgroundColor1(VulpineColorUI::LightBackgroundColor2)
            
        , WidgetButton(WidgetButton::Type::SLIDER, ifunc, ufunc)
            .setmin(min).setmax(max).setpadding(padding)
        , EntityGroupInfo({
            newEntity(name + " - SLIDER HELPER" 
                , UI_BASE_COMP
                , WidgetBackground()
                , WidgetStyle()
                    .setbackGroundStyle(UiTileType::SQUARE_ROUNDED)
                    // .setbackGroundStyle(UiTileType::SQUARE)
                    .setbackgroundColor1(color)
                , WidgetBox(VulpineBlueprintUI::SmoothSliderFittingFunc)
            )
        }) 
    );
}

EntityRef  VulpineBlueprintUI::Toggable(
    const std::string &name,
    const std::string &icon,
    WidgetButton::InteractFunc ifunc, 
    WidgetButton::UpdateFunc ufunc,
    vec3 color
)
{
    auto e = newEntity(name
        , UI_BASE_COMP
        , WidgetBox()
        , WidgetBackground()
        , WidgetStyle()
            .setbackGroundStyle(UiTileType::SQUARE_ROUNDED)
            .setbackgroundColor1(vec4(color, 1.))
            // .setbackgroundColor2(VulpineColorUI::LightBackgroundColor2)
            // .setbackgroundColor2(vec4(rgb2hsv(hsv2rgb(vec3(color)) * vec3(1, .5, 2)), color.a*.5))
            .setbackgroundColor2(vec4(color, ALPHA2/255.f))
            .settextColor1(VulpineColorUI::DarkBackgroundColor1)
        , WidgetButton(WidgetButton::Type::CHECKBOX, ifunc, ufunc)
    );

    if(icon.size())
        e->set<WidgetSprite>(WidgetSprite(icon));
    else
        e->set<WidgetText>(UFTconvert.from_bytes(name));

    return e;
}

EntityRef VulpineBlueprintUI::TextInput(
    const std::string &name,
    std::function<void(std::u32string &)> fromText, 
    std::function<std::u32string()> toText
    )
{
    auto t = newEntity(name
        , UI_BASE_COMP
        , WidgetBox()
        , WidgetBackground()
        , WidgetStyle()
            .setbackGroundStyle(UiTileType::SQUARE_ROUNDED)
            .setbackgroundColor1(VulpineColorUI::DarkBackgroundColor1)
            .setbackgroundColor2(VulpineColorUI::DarkBackgroundColor2)
            .settextColor1(VulpineColorUI::LightBackgroundColor1)
            .settextColor2(VulpineColorUI::HightlightColor1)
        , WidgetText(U"...")
    );

    auto &text = t->comp<WidgetText>().text;

    t->set<WidgetButton>(WidgetButton(
        WidgetButton::Type::TEXT_INPUT,
        [&text, fromText](Entity *e, float v)
        {
            fromText(text);
        },
        [&text, toText](Entity *e)
        {
            text = toText();
            return 0.f;
        }
    ));

    return t;
}

EntityRef VulpineBlueprintUI::ValueInput(
    const std::string &name,
    std::function<void(float f)> setValue, 
    std::function<float()> getValue,
    float minV, float maxV,
    float smallIncrement, float bigIncrement
    )
{
    #define PASS_ARG_COPY setValue, getValue, minV, maxV, smallIncrement, bigIncrement

    auto textInput = TextInput(name,
        [PASS_ARG_COPY](std::u32string &t)
        {
            setValue(clamp(u32strtof2(t, getValue()), minV, maxV));
        },
        [PASS_ARG_COPY]()
        {
            return ftou32str(getValue());
        }
    );

    auto incr = Toggable("n+", "", 
        [PASS_ARG_COPY](Entity *e, float v){setValue(clamp(getValue() + smallIncrement, minV, maxV));},
        [PASS_ARG_COPY](Entity *e){return 1.f;}
    );

    auto decr = Toggable("n-", "", 
        [PASS_ARG_COPY](Entity *e, float v){setValue(clamp(getValue() - smallIncrement, minV, maxV));},
        [PASS_ARG_COPY](Entity *e){return 1.f;}
    );

    auto Bincr = Toggable("n++", "", 
        [PASS_ARG_COPY](Entity *e, float v){setValue(clamp(getValue() + bigIncrement, minV, maxV));},
        [PASS_ARG_COPY](Entity *e){return 1.f;}
    );

    auto Bdecr = Toggable("n--", "", 
        [PASS_ARG_COPY](Entity *e, float v){setValue(clamp(getValue() - bigIncrement, minV, maxV));},
        [PASS_ARG_COPY](Entity *e){return 1.f;}
    );

    return newEntity(name + " - value input"
        , UI_BASE_COMP
        , WidgetBox()
        , WidgetStyle()
            .setautomaticTabbing(1)
        , EntityGroupInfo({
            Bdecr, decr, textInput, incr, Bincr
        })
    );

    #undef PASS_ARG_COPY
}



EntityRef VulpineBlueprintUI::ValueInputSlider(
    const std::string &name,
    float min, float max, int padding, 
    WidgetButton::InteractFunc ifunc, 
    WidgetButton::UpdateFunc ufunc,
    std::function<void(std::u32string&)> fromText, 
    std::function<std::u32string()> toText,
    vec4 color
    )
{
    auto s = VulpineBlueprintUI::SmoothSlider(name, min, max, padding, ifunc, ufunc, color);

    s->comp<WidgetBox>() = WidgetBox(
        vec2(-1, 1./3.), vec2(-1, 1)
    );

    auto t = VulpineBlueprintUI::TextInput(name, fromText, toText);

    t->comp<WidgetBox>() = WidgetBox(
        vec2(1./3., 1), vec2(-1, 1)
    );

    s->comp<WidgetBox>().set(vec2(-1, 0.5), vec2(-1, 1)*0.75f );
    t->comp<WidgetBox>().set(vec2(0.5, 1),  vec2(-1, 1));

    auto p = newEntity(name + " - Menu"
        , UI_BASE_COMP
        , WidgetBox()
        , WidgetStyle()
            .setautomaticTabbing(1)
            .setuseInternalSpacing(false)
        , EntityGroupInfo({s, t})
    );

    return p;
}

EntityRef VulpineBlueprintUI::ValueInputSlider(
    const std::string &name,
    float min, float max, int padding, 
    std::function<void(float f)> setValue, 
    std::function<float()> getValue,
    vec4 color
    )
{
    auto s = VulpineBlueprintUI::SmoothSlider(name, min, max, padding, 
        [setValue, getValue, min, max](Entity *e, float f)
        {
            setValue(f);
        }, 
        [setValue, getValue](Entity *e)
        {
            return getValue();
        }, color
    );

    s->comp<WidgetBox>() = WidgetBox(
        vec2(-1, 1./3.), vec2(-1, 1)
    );

    auto t = VulpineBlueprintUI::TextInput(name, 
        [setValue, getValue, min, max](std::u32string &t)
        {
            setValue(clamp(u32strtof2(t, getValue()), min, max));
        },
        [setValue, getValue, min, max]()
        {
            return ftou32str(getValue());
        }
    );

    t->comp<WidgetBox>() = WidgetBox(
        vec2(1./3., 1), vec2(-1, 1)
    );

    auto p = newEntity(name + " - Menu"
        , UI_BASE_COMP
        , WidgetBox()
        , WidgetStyle()
            .setautomaticTabbing(1)
            .setuseInternalSpacing(false)
        , EntityGroupInfo({s, t})
    );

    return p;
}


EntityRef VulpineBlueprintUI::NamedEntry(
    const std::u32string &name,
    EntityRef entry,
    float nameRatioSize,
    bool vertical,
    vec4 color 
)
{
    int tabbing = nameRatioSize == 0.5f ? (vertical ? 2 : 1) : 0;

    nameRatioSize = nameRatioSize*2. - 1.;

    vec2 titleRange = vec2(-1, nameRatioSize-0.01);
    vec2 entryRange = vec2(nameRatioSize+0.01, 1);
    vec2 d = vec2(-1, 1);

    entry->comp<WidgetBox>().set(
        !vertical ? entryRange : d, 
        !vertical ? d : entryRange        
        );
        
    auto &entryName = entry->comp<EntityInfos>().name;
    

    return newEntity(entryName + " - Menu"
        , UI_BASE_COMP
        , WidgetBox()
        , WidgetStyle()
            .setautomaticTabbing(tabbing)
            .setuseInternalSpacing(false)
        , EntityGroupInfo({
            newEntity(entryName + " - Helper Name"
                , UI_BASE_COMP
                , WidgetBox(
                    !vertical ? titleRange : d, 
                    !vertical ? d : titleRange
                    )
                , WidgetBackground()
                , WidgetText(name)
                , WidgetStyle()
                    .setbackgroundColor1(color)
                    .settextColor1(VulpineColorUI::DarkBackgroundColor1)
                    .setbackGroundStyle(UiTileType::SQUARE_ROUNDED)
            ),
            entry
        })
    );

}

EntityRef VulpineBlueprintUI::ColorSelectionScreen(
    const std::string &name,
    std::function<vec3()> getColor, 
    std::function<void(vec3)> setColor
)
{
    auto hue = ValueInputSlider(
        name + " - hue", 0., 0.9999, 360,
        [setColor, getColor](Entity *e, float v)
        {
            vec3 hsv = rgb2hsv(getColor());
            hsv.r = v;
            setColor(hsv2rgb(hsv));
        },
        [setColor, getColor](Entity *e)
        {
            vec3 c = rgb2hsv(getColor());
            return rgb2hsv(getColor()).r;
        },
        [setColor, getColor](std::u32string text)
        {
            vec3 hsv = rgb2hsv(getColor());
            hsv.r = u32strtof2(text, round(hsv.r*360.f))/360.f;
            setColor(hsv2rgb(hsv));
        },
        [setColor, getColor]()
        {
            char str[8];
            float f = round(rgb2hsv(getColor()).r*360.f);
            std::snprintf(str, 5, "%*.f", 0, f); 
            return UFTconvert.from_bytes(str);
        }
    );

    auto saturation = ValueInputSlider(
        name + " - saturation", 0., 1.0, 100,
        [setColor, getColor](Entity *e, float v)
        {
            vec3 hsv = rgb2hsv(getColor());
            hsv.g = v;
            setColor(hsv2rgb(hsv));
        },
        [setColor, getColor](Entity *e)
        {
            return rgb2hsv(getColor()).g;
        },
        [setColor, getColor](std::u32string text)
        {
            vec3 hsv = rgb2hsv(getColor());
            hsv.g = u32strtof2(text, round(hsv.g*100.f))/100.f;
            setColor(hsv2rgb(hsv));
        },
        [setColor, getColor]()
        {
            char str[8];
            float f = round(rgb2hsv(getColor()).g*100.f);
            std::snprintf(str, 5, "%*.f", 0, f); 
            return UFTconvert.from_bytes(str);
        }
    );

    auto value = ValueInputSlider(
        name + " - value", 0., 1.0, 100,
        [setColor, getColor](Entity *e, float v)
        {
            vec3 hsv = rgb2hsv(getColor());
            hsv.b = v;
            setColor(hsv2rgb(hsv));
        },
        [setColor, getColor](Entity *e)
        {
            return rgb2hsv(getColor()).b;
        },
        [setColor, getColor](std::u32string text)
        {
            vec3 hsv = rgb2hsv(getColor());
            hsv.b = u32strtof2(text, round(hsv.b*100.f))/100.f;
            setColor(hsv2rgb(hsv));
        },
        [setColor, getColor]()
        {
            char str[8];
            float f = round(rgb2hsv(getColor()).b*100.f);
            std::snprintf(str, 5, "%*.f", 0, f); 
            return UFTconvert.from_bytes(str);
        }
    );

    auto red = ValueInputSlider(
        name + " - red", 0., 1.0, 255,
        [setColor, getColor](Entity *e, float v)
        {
            vec3 rgb = getColor();
            rgb.r = v;
            setColor(rgb);
        },
        [setColor, getColor](Entity *e)
        {
            return getColor().r;
        },
        [setColor, getColor](std::u32string text)
        {
            vec3 rgb = getColor();
            rgb.r = u32strtof2(text, round(rgb.r*255.f))/255.f;
            setColor(rgb);
        },
        [setColor, getColor]()
        {
            char str[8];
            float f = round(getColor().r*255.f);
            std::snprintf(str, 5, "%*.f", 0, f); 
            return UFTconvert.from_bytes(str);
        }
    );

    auto green = ValueInputSlider(
        name + " - green", 0., 1.0, 255,
        [setColor, getColor](Entity *e, float v)
        {
            vec3 rgb = getColor();
            rgb.g = v;
            setColor(rgb);
        },
        [setColor, getColor](Entity *e)
        {
            return getColor().g;
        },
        [setColor, getColor](std::u32string text)
        {
            vec3 rgb = getColor();
            rgb.g = u32strtof2(text, round(rgb.g*255.f))/255.f;
            setColor(rgb);
        },
        [setColor, getColor]()
        {
            char str[8];
            float f = round(getColor().g*255.f);
            std::snprintf(str, 5, "%*.f", 0, f); 
            return UFTconvert.from_bytes(str);
        }
    );

    auto blue = ValueInputSlider(
        name + " - blue", 0., 1.0, 255,
        [setColor, getColor](Entity *e, float v)
        {
            vec3 rgb = getColor();
            rgb.b = v;
            setColor(rgb);
        },
        [setColor, getColor](Entity *e)
        {
            return getColor().b;
        },
        [setColor, getColor](std::u32string text)
        {
            vec3 rgb = getColor();
            rgb.b = u32strtof2(text, round(rgb.b*255.f))/255.f;
            setColor(rgb);
        },
        [setColor, getColor]()
        {
            char str[8];
            float f = round(getColor().b*255.f);
            std::snprintf(str, 5, "%*.f", 0, f); 
            return UFTconvert.from_bytes(str);
        }
    );

    auto hex = newEntity(name
        , UI_BASE_COMP
        , WidgetBox()
        , WidgetBackground()
        , WidgetStyle()
            .setbackGroundStyle(UiTileType::SQUARE_ROUNDED)
            .setbackgroundColor1(VulpineColorUI::DarkBackgroundColor1)
            .setbackgroundColor2(VulpineColorUI::DarkBackgroundColor2)
            .settextColor1(VulpineColorUI::LightBackgroundColor1)
            .settextColor2(VulpineColorUI::HightlightColor1)
        , WidgetText(U"")
    );

    auto &hexText = hex->comp<WidgetText>().text;

    hex->set<WidgetButton>(WidgetButton(
        WidgetButton::Type::TEXT_INPUT,
        [&hexText, setColor](Entity *e, float v)
        {
            vec3 c;
            if(u32strtocolorHTML(hexText, c))
                setColor(c);
        },
        [&hexText, getColor](Entity *e)
        {
            hexText = rgbtou32str(getColor());
            return 0.f;
        }
    ));

    auto finalColorVisualisation = newEntity(name + " - hue "
        , UI_BASE_COMP
        , WidgetBox([getColor](Entity *parent, Entity *child)
            {
                child->comp<WidgetStyle>()
                    .setbackgroundColor1(
                        vec4(getColor(), 1)
                    );
            })
        , WidgetBackground()
        , WidgetStyle()
            .setbackGroundStyle(UiTileType::SQUARE_ROUNDED)
    );


    auto sliders = newEntity(name + " - Slider Menu"
        , UI_BASE_COMP
        , WidgetBox()
        , WidgetStyle()
            .setautomaticTabbing(8)
        , EntityGroupInfo({
            
            finalColorVisualisation,
            NamedEntry(U"Hex", hex, 0.25),
            
            // newEntity("separator", UI_BASE_COMP, WidgetBox()),

            NamedEntry(U"H", hue, 0.25), 
            NamedEntry(U"S", saturation, 0.25), 
            NamedEntry(U"V", value, 0.25), 
            
            // newEntity("separator", UI_BASE_COMP, WidgetBox()),

            NamedEntry(U"R", red, 0.25), 
            NamedEntry(U"G", green, 0.25), 
            NamedEntry(U"B", blue, 0.25),


            })
    );

    auto valueSatPicker = newEntity(name + " - saturation & value picker"
        , UI_BASE_COMP
        , WidgetBox(
            [getColor](Entity *p, Entity*c)
            {
                c->comp<WidgetStyle>().setbackgroundColor1(
                    vec4(getColor(), 1)
                );
            }
        )
        , WidgetBackground()
        , WidgetStyle()
            .setbackGroundStyle(UiTileType::SATURATION_VALUE_PICKER)
        , WidgetSprite("picker_cursor")
        , WidgetButton(
            WidgetButton::Type::SLIDER_2D, 
            [getColor, setColor](Entity *e, vec2 uv)
            {
                vec3 hsv = rgb2hsv(getColor());
                hsv.g = uv.x;
                hsv.b = 1.0 - uv.y;
                setColor(hsv2rgb(hsv));
            },
            [getColor, setColor](Entity *e)
            {
                vec3 hsv = rgb2hsv(getColor());
                return vec2(hsv.g, 1.0 - hsv.b);
            }
        ).setpadding(100).setmin(0.001).setmax(0.9999)
    );

    auto hueButMoreVisual = newEntity(name + " - hue "
        , UI_BASE_COMP
        , WidgetBackground()
        , WidgetBox()
        , WidgetStyle()
            .setbackGroundStyle(UiTileType::HUE_PICKER)
            .setbackgroundColor1(VulpineColorUI::LightBackgroundColor2)
        , WidgetButton(WidgetButton::Type::SLIDER, 

            [setColor, getColor](Entity *e, float v)   
            {
                vec3 hsv = rgb2hsv(getColor());
                hsv.r = v;
                setColor(hsv2rgb(hsv));
            },
            [setColor, getColor](Entity *e)
            {
                return rgb2hsv(getColor()).r;
            }
            ).setmin(0).setmax(0.99999).setpadding(360)
        , WidgetSprite("VulpineIcon")

        // , EntityGroupInfo({
        //     newEntity(name + " - SLIDER HELPER" 
        //         , UI_BASE_COMP
        //         , WidgetBackground()
        //         , WidgetStyle()
        //             .setbackGroundStyle(UiTileType::SQUARE)
        //             .setbackgroundColor1(VulpineColorUI::LightBackgroundColor1)
        //         , WidgetBox(VulpineBlueprintUI::SmoothSliderFittingFunc)
        //     )
        // }) 
    );
    


    // ValueInputSlider(
    //     name + " - hue", 0., 0.9999, 360,
    //     [setColor, getColor](float v)
    //     {
    //         // std::cout << "==== DEBUG PRINT 1 ====\n";
    //         vec3 hsv = rgb2hsv(getColor());
    //         hsv.r = v;
    //         setColor(hsv2rgb(hsv));
    //     },
    //     [setColor, getColor]()
    //     {
    //         // std::cout << "==== DEBUG PRINT 2 ====\n";
    //         return rgb2hsv(getColor()).r;
    //     },
    //     [setColor, getColor](std::u32string text)
    //     {
    //         vec3 hsv = rgb2hsv(getColor());
    //         hsv.r = u32strtof2(text, round(hsv.r*360.f))/360.f;
    //         setColor(hsv2rgb(hsv));
    //     },
    //     [setColor, getColor]()
    //     {
    //         char str[8];
    //         float f = round(rgb2hsv(getColor()).r*360.f);
    //         std::snprintf(str, 5, "%*.f", 0, f); 
    //         return UFTconvert.from_bytes(str);
    //     }
    // );

    valueSatPicker->comp<WidgetBox>().set(vec2(-1, 1), vec2(-1, 0.7));
    hueButMoreVisual->comp<WidgetBox>().set(vec2(-1, 1), vec2(0.75, 1));

    auto chromaticZone = newEntity(name + " - chromatic zone background parent"
        , UI_BASE_COMP
        , WidgetBox()
        , WidgetBackground()
        , WidgetStyle()
            .setautomaticTabbing(1)
            .setbackgroundColor1(VulpineColorUI::DarkBackgroundColor2)
        , EntityGroupInfo({
            newEntity(name + " - chromatic zone"
                , UI_BASE_COMP
                , WidgetBox()
                , EntityGroupInfo({
                    valueSatPicker, hueButMoreVisual
                })
            )

        })
    );


    auto p = newEntity(name + " + Menu "
        , UI_BASE_COMP
        , WidgetBox()
        , WidgetStyle()
            .setautomaticTabbing(1)
        , EntityGroupInfo({
            chromaticZone, sliders
        })
    );

    return p;
}

EntityRef VulpineBlueprintUI::TimerPlot(
    BenchTimer &timer, 
    vec4(color),
    std::function<vec2()> getMinmax
    )
{
    auto infos = newEntity(timer.name + " - Infos Plo"
        , UI_BASE_COMP
        , WidgetSprite(PlottingHelperRef(new PlottingHelper(color, 1024)))
        , WidgetBox(
            [&timer, getMinmax](Entity *parent, Entity *child)
            {
                if(timer.isPaused()) 
                    return;

                PlottingHelper* p = (PlottingHelper*)child->comp<WidgetSprite>().sprite.get();

                vec2 minmax = getMinmax();
                p->minv = minmax.x;
                p->maxv = minmax.y;
                p->push(timer.getDeltaMS());
                p->updateData();
            }
        )
    );

    return infos;
}

EntityRef VulpineBlueprintUI::ColoredConstEntry(
    const std::string &name,
    std::function<std::u32string()> toText,
    vec4 color
)
{
    vec4 color2 = color;
    color2.a *= 0.5;

    return newEntity(name + "- coloredConstEntry"
        , UI_BASE_COMP
        , WidgetBox()
        , WidgetStyle()
            // .setautomaticTabbing(1)
        , EntityGroupInfo({
            newEntity(name
                , UI_BASE_COMP
                , WidgetBox(vec2(-1, -0.01), vec2(-1, 1))
                , WidgetText()
                , WidgetBackground()
                , WidgetStyle()
                    .setbackGroundStyle(UiTileType::SQUARE_ROUNDED)
                    // .setbackgroundColor1(color)
                    .settextColor1(VulpineColorUI::DarkBackgroundColor1)
                    // .settextColor1(color)
                    .setbackgroundColor1(VulpineColorUI::LightBackgroundColor1)
            ),

            newEntity(name + "- text"
                , UI_BASE_COMP
                , WidgetBackground()
                , WidgetBox([toText](Entity *parent, Entity *child)
                {
                    child->comp<WidgetText>().text = toText();
                }).set(vec2(0.01, 1), vec2(-1, 1))
                , WidgetStyle()
                    // .setbackgroundColor1(color2)
                    .setbackGroundStyle(UiTileType::SQUARE_ROUNDED)
                    // .settextColor1(VulpineColorUI::LightBackgroundColor1)

                    .setbackgroundColor1(VulpineColorUI::DarkBackgroundColor2)

                    .settextColor1(color)

                , WidgetText(U" ")
            )
        })
    );
}

void VulpineBlueprintUI::AddToSelectionMenu(
    EntityRef titlesParent, 
    EntityRef infosParent,  
    EntityRef info,
    const std::string &name,
    const std::string &icon
)
{
    auto title = newEntity(name
        , UI_BASE_COMP
        , WidgetBox()
        , WidgetStyle()
            .setbackgroundColor1(VulpineColorUI::LightBackgroundColor1)
            .setbackgroundColor2(VulpineColorUI::LightBackgroundColor2)
            .settextColor1(VulpineColorUI::DarkBackgroundColor1)
            .setbackGroundStyle(UiTileType::SQUARE_ROUNDED)
        , WidgetBackground()
        , WidgetButton(WidgetButton::Type::HIDE_SHOW_TRIGGER_INDIRECT).setusr((uint64)info.get())
    );

    if(icon.size())
        title->set<WidgetSprite>(WidgetSprite(icon));
    else 
        title->set<WidgetText>(WidgetText(UFTconvert.from_bytes(name)));

    title->comp<WidgetState>().statusToPropagate = ModelStatus::HIDE;

    ComponentModularity::addChild(*titlesParent, title);
    ComponentModularity::addChild(*infosParent , info);
}


EntityRef VulpineBlueprintUI::SceneInfos(Scene& scene)
{

    return newEntity("Scene Info View"
        , UI_BASE_COMP
        , WidgetBox()
        , WidgetStyle()
            .setautomaticTabbing(1)
        , EntityGroupInfo({

            newEntity("Scene Info View Values"
                , UI_BASE_COMP
                , WidgetBox()
                , WidgetStyle()
                    .setautomaticTabbing(7)
                , EntityGroupInfo({
                    ColoredConstEntry("DRAW CALLS", [&scene](){return ftou32str(scene.getDrawCalls(), 4);}),

                    ColoredConstEntry("TRIANGLES", [&scene](){return ftou32str(scene.getPolyCount(), 6);}),

                    ColoredConstEntry("VERTEX", [&scene](){return ftou32str(scene.getVertexCount(), 6);}),

                    ColoredConstEntry("TOTAL MESHES", [&scene](){return ftou32str(scene.getTotalMeshes(), 6);}),

                    ColoredConstEntry("MATERIALS", [&scene](){return ftou32str(scene.getMaterialCount());}),

                    ColoredConstEntry("LIGHTS", [&scene](){return ftou32str(scene.getLights().size());}),

                    ColoredConstEntry("SHADOW MAPS", [&scene](){return ftou32str(scene.getShadowMapCount());}),

                    // ColoredConstEntry("BINDLESS TEXTURES", [&scene](){return scene.useBindlessTextures ? U"Activated" : U"Disabled";}),

                    ColoredConstEntry("CULL TIME", [&scene](){return ftou32str(scene.cullTime.getLastAvg().count(), 4) + U" ms";}),

                    ColoredConstEntry("DRAW CALL TIME", [&scene](){return ftou32str(scene.callsTime.getLastAvg().count(), 4) + U" ms";}),

                    ColoredConstEntry("DEPTH DRAW CALL TIME", [&scene](){return ftou32str(scene.depthOnlyCallsTime.getLastAvg().count(), 4) + U" ms";}),

                    ColoredConstEntry("SHADOWMAP CALL TIME", [&scene](){return ftou32str(scene.shadowPassCallsTime.getLastAvg().count(), 4) + U" ms";}),

                    ColoredConstEntry("LIGHT BUFFER GEN", [&scene](){return ftou32str(scene.lightBufferTime.getLastAvg().count(), 4) + U" ms";}),

                    // ColoredConstEntry("FRUSTUM CLUSTER DIMENTION", [&scene]()
                    // {
                    //     auto dim = scene.getClusteredLight().dim();
                    //     return ftou32str(dim.x) + U"x" + ftou32str(dim.y) + U"x" + ftou32str(dim.z);
                    // }),

                    ColoredConstEntry("FRUSTUM CLUSTER VFAR", [&scene](){ return ftou32str(scene.getClusteredLight().vFar, 5) + U" m";}),

                    ColoredConstEntry("CLUSTER LIGHTING", [&scene](){ return scene.isUSingClusteredLighting() ? U"Activated" : U"Disabled";;}),
                })
            ),
        })
    );
}

EntityRef VulpineBlueprintUI::StringListSelectionMenu(
    const std::string &name,
    std::unordered_map<std::string, EntityRef>& list,
    WidgetButton::InteractFunc ifunc, 
    WidgetButton::UpdateFunc ufunc,
    float verticalLenghtReduction,
    vec4 color,
    float nameSizeRatio 
)
{
    auto searchInput = NamedEntry(U"Search"
        , TextInput(
            name + " search bar", [](std::u32string &name)
            {
                if(!name.size())
                    name = U"...";
            },
            []()
            {
                return U"";
            }
        )
        , 0.5
        , false
        , color
    );

    Entity *searchInputPTR = searchInput.get();

    auto listScreen = newEntity(name + " string list"
        , UI_BASE_COMP
        , WidgetStyle()
            // .setautomaticTabbing(50)
        , WidgetBox([&list, ufunc, ifunc, searchInputPTR, color](Entity *parent, Entity *child)
        {
            auto &children = child->comp<EntityGroupInfo>().children;
            std::vector<EntityRef> childrenCopy;

            for(auto &i : children)
            {
                auto elem = list.find(i->comp<EntityInfos>().name);

                // if(elem != list.end() && elem->second.get())
                // {
                //     i = elem->second;
                // }

                if(elem != list.end() && i.get())
                {
                    elem->second = i;
                }
            }
                
            // children = childrenCopy;


            /**** Creating Children *****/
            for(auto &i : list)
                if(!i.second.get())
                {
                    EntityRef button;
                    ComponentModularity::addChild(
                        *child,
                        i.second = newEntity(i.first
                            , UI_BASE_COMP
                            , WidgetBox()
                            , WidgetStyle()
                                .setautomaticTabbing(1)
                            , EntityGroupInfo({
                                button = Toggable(
                                    i.first, "", ifunc, ufunc
                                )
                            })
                        )
                    );

                    button->comp<WidgetStyle>().setbackgroundColor1(mix(color, VulpineColorUI::LightBackgroundColor1, 0.25f));

                    i.second->comp<WidgetBox>()
                        .set(vec2(-1, 1), vec2(1, 5))
                        .type = WidgetBox::Type::FOLLOW_SIBLINGS_BOX;
                    
                    i.second->comp<WidgetBox>().useClassicInterpolation = true;
                }


            // auto &children = child->comp<EntityGroupInfo>().children;
            // std::vector<EntityRef> childrenCopy;

            childrenCopy.clear();
            for(auto i : children)
            {
                auto elem = list.find(i->comp<EntityInfos>().name);
                if(elem != list.end() && elem->second.get())
                    childrenCopy.push_back(i);
            }
                
            children = childrenCopy;



            std::sort(
                children.begin(),
                children.end(),
                [](const EntityRef &a, const EntityRef &b)
                {
                    auto str1 = a->comp<EntityInfos>().name;
                    auto str2 = b->comp<EntityInfos>().name;

                    for(auto &c : str1)
                        c = std::tolower(c);

                    for(auto &c : str2)
                        c = std::tolower(c);

                    return str1 <= str2;
                }
            );

            // child->comp<WidgetState>().status = ModelStatus::UNDEFINED;

            std::string str = UFTconvert.to_bytes(searchInputPTR
                ->comp<EntityGroupInfo>().children[1]
                // ->comp<EntityGroupInfo>().children[1]
            ->comp<WidgetText>().text);

            for(auto &c : str)
                c = std::tolower(c);
            
            // std::cout << st

            if(child->comp<WidgetState>().statusToPropagate != ModelStatus::HIDE)
            {
                for(auto c : child->comp<EntityGroupInfo>().children)
                {

                    auto str2 = c->comp<EntityInfos>().name;

                    for(auto &c : str2)
                        c = std::tolower(c);

                    if(!str.size() || str2.find(str) != std::string::npos)
                    {
                        c->comp<WidgetState>().status = ModelStatus::SHOW;

                        c->comp<WidgetState>().statusToPropagate = ModelStatus::SHOW;

                        // c->comp<WidgetBox>().useClassicInterpolation = false;
                        
                        // std::cout << str2 << "\n";
                    }
                    else
                    {
                        c->comp<WidgetState>().status = ModelStatus::HIDE;
                        c->comp<WidgetState>().statusToPropagate = ModelStatus::HIDE;
                    } 
                }
            }
            else
            {
                for(auto c : child->comp<EntityGroupInfo>().children)
                {
                    c->comp<WidgetState>().status = ModelStatus::HIDE;
                    c->comp<WidgetState>().statusToPropagate = ModelStatus::HIDE;
                }
            }
        })
    );

    

    // Entity *listScreenPTR = listScreen.get();

    // auto searchInput = NamedEntry(U"Search", TextInput(
    //     name + " search bar", [listScreenPTR](std::u32string &name)
    //     {
    //         std::string str = UFTconvert.to_bytes(name);

    //         for(auto &c : str)
    //             c = std::tolower(c);

    //         for(auto c : listScreenPTR->comp<EntityGroupInfo>().children)
    //         {

    //             auto str2 = c->comp<EntityInfos>().name;

    //             for(auto &c : str2)
    //                 c = std::tolower(c);

    //             if(!str.size() || str2.find(str) != std::string::npos)
    //             {
    //                 c->comp<WidgetState>().status = ModelStatus::SHOW;

    //                 c->comp<WidgetState>().statusToPropagate = ModelStatus::SHOW;

    //                 // c->comp<WidgetBox>().useClassicInterpolation = false;
                    
    //                 // std::cout << str2 << "\n";
    //             }
    //             else
    //             {
    //                 c->comp<WidgetState>().status = ModelStatus::HIDE;
    //                 c->comp<WidgetState>().statusToPropagate = ModelStatus::HIDE;
    //             } 
    //         }

    //         if(!name.size())
    //             name = U"...";
    //     },
    //     []()
    //     {
    //         return U"";
    //     }
    // ));
    
    auto scrollZone = newEntity(name + " scroll zone list"
        , UI_BASE_COMP
        , WidgetStyle()
            // .setautomaticTabbing(1)
            // .setbackgroundColor1(VulpineColorUI::HightlightColor1)
        // , WidgetBackground()
        , WidgetBox([](Entity *parent, Entity *child){
            auto &box = child->comp<WidgetBox>();

            vec2 off = globals.mouseScrollOffset();

            if(box.isUnderCursor)
            {
                box.scrollOffset.y += off.y*0.1;

                box.scrollOffset.y = clamp(
                    box.scrollOffset.y,
                    -0.05f*child->comp<EntityGroupInfo>().children[0]->comp<EntityGroupInfo>().children.size(),
                    0.f
                );

                globals.clearMouseScroll();
            }

            box.displayRangeMin = box.min;
            box.displayRangeMax = box.max;

        })
        , EntityGroupInfo({listScreen})
    );

    listScreen->comp<WidgetBox>().set(vec2(-1, 1), vec2(-1, -0.95 - verticalLenghtReduction*0.05));

    scrollZone->comp<WidgetBox>().set(vec2(-1, 1), vec2(-0.9 -0.1*verticalLenghtReduction, 1));

    searchInput->comp<WidgetBox>().set(vec2(-1, 1), vec2(-1, -0.9 -0.1*verticalLenghtReduction));

    searchInput->comp<WidgetStyle>().setautomaticTabbing(1);

    auto p = newEntity(name
        , UI_BASE_COMP
        , WidgetBox(vec2(-1, 1), vec2(-1, 0.9 -0.1*verticalLenghtReduction))
        , EntityGroupInfo({
            scrollZone, 
            searchInput
            
        })
        );
    if(nameSizeRatio <= 0.f)
        return p;
    else
        return NamedEntry(UFTconvert.from_bytes(name), p, nameSizeRatio, true, color);
}
