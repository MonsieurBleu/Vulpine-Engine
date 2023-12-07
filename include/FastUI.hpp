#ifndef FAST_UI_HPP
#define FAST_UI_HPP

#include <list>
#include <Fonts.hpp>
#include <Scene.hpp>

#include <ObjectGroup.hpp>

enum UiTileType
{
    SQUARE,
    SQUARE_ROUNDED,
    CIRCLE
};


/*
    TODO : add ancor 
*/
class SimpleUiTile : public ModelState3D
{
    private : 

    public : 
        SimpleUiTile(ModelState3D state, UiTileType tileType, vec4 color = vec4(0.85));
        UiTileType tileType;
        vec4 color;
};

typedef std::shared_ptr<SimpleUiTile> SimpleUiTileRef;

class SimpleUiTileBatch : public MeshModel3D
{
    private : 
        std::list<SimpleUiTileRef> tiles;

    public : 
        SimpleUiTileBatch& add(SimpleUiTileRef tile);
        /*
            TODO : add inteligent buffer overwrite (the whole thing is not re-allocated if the place is arleady here)
        */
        SimpleUiTileBatch& batch();

};

typedef std::shared_ptr<SimpleUiTileBatch> SimpleUiTileBatchRef;

struct FastUI_context
{
    FastUI_context(
        SimpleUiTileBatchRef& tileBatch, 
        FontRef& font, 
        Scene& scene, 
        MeshMaterial fontMaterial);
    
    SimpleUiTileBatchRef& tileBatch;
    FontRef& font;
    Scene& scene;
    MeshMaterial fontMaterial;

    bool batchNeedUpdate = false;

    vec4 colorTitleBackground = vec4(0, 0.4, 0.8, 0.75);
    vec4 colorCurrentTitleBackground = vec4(0, 0.3, 0.95, 0.75);
    vec4 colorHighlightedTitleBackground = vec4(0.3, 0.5, 0.9, 0.75);
    vec3 colorTitleFont = vec3(1.0);

    vec4 colorBackground = vec4(0, 0.1, 0.2, 0.8);
    vec3 colorFont = vec3(0.9);
};

struct FastUI_element : public ObjectGroupRef
{
    protected : 
        vec2 size = vec2(0);

    public : 
        FastUI_context& ui;

        FastUI_element(FastUI_context& ui);
        vec2 getSize();
};

class FastUI_menuTitle : public FastUI_element
{
    private : 
        SingleStringBatchRef title;
        SimpleUiTileRef background;

    public : 
        FastUI_menuTitle(FastUI_context& ui, std::u32string name);
        void changeBackgroundColor(vec4 color);
};

enum FastUi_supportedValueType
{
    FUI_float,
    FUI_int
};

class FastUI_value
{
    private : 
        const void *constValue = nullptr;
        void *value = nullptr;
        FastUi_supportedValueType type;

    public :

        std::u32string textPrev;
        std::u32string textAfter;

        FastUI_value(const float* value, std::u32string textPrev = U"", std::u32string textAfter = U"")
        : constValue(value), type(FUI_float), textPrev(textPrev), textAfter(textAfter)
        {};

        FastUI_value(float* value, std::u32string textPrev = U"", std::u32string textAfter = U"")
        : value(value), type(FUI_float), textPrev(textPrev), textAfter(textAfter)
        {};

        FastUI_value(const int* value, std::u32string textPrev = U"", std::u32string textAfter = U"")
        : constValue(value), type(FUI_int), textPrev(textPrev), textAfter(textAfter)
        {};

        FastUI_value(int* value, std::u32string textPrev = U"", std::u32string textAfter = U"")
        : value(value), type(FUI_int), textPrev(textPrev), textAfter(textAfter)
        {};

        UFT32Stream& toString(UFT32Stream& os);
};

class FastUI_valueTab : public FastUI_element, public std::vector<FastUI_value>
{
    private :
        SingleStringBatchRef text;
        SimpleUiTileRef background;
        vec2 padding;

    public : 
        FastUI_valueTab(FastUI_context& ui, const std::vector<FastUI_value> &values);
        // FastUI_valueTab& add(FastUI_value v);
        
        FastUI_valueTab& batch();
        void changeBackgroundColor(vec4 color);
};

struct FastUI_titledValueTab
{
    FastUI_menuTitle title;
    FastUI_valueTab tab;
};

class FastUI_valueMenu : public FastUI_element, public std::vector<FastUI_titledValueTab>
{
    private : 
        std::vector<FastUI_titledValueTab> &elements;
        int currentTab = -1;
        int highlighted = -1;
        void currentHighlighted(int id);

    public : 
        FastUI_valueMenu(FastUI_context& ui, const std::vector<FastUI_titledValueTab> &values);
        void batch();
        void updateText();

        void setCurrentTab(int id);

        void trackCursor();
};


#endif