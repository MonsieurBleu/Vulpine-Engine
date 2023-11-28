#ifndef FAST_UI_HPP
#define FAST_UI_HPP

#include <Mesh.hpp>
#include <list>

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
        bool hide = false;
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

#endif