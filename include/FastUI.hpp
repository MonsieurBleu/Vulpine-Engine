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

class SimpleUiTile : public ModelState3D
{
    private : 

    public : 
        SimpleUiTile(ModelState3D state, UiTileType tileType);
        UiTileType tileType;
        bool hide = false;
};

typedef std::shared_ptr<SimpleUiTile> SimpleUiTileRef;

class SimpleUiTileBatch : public MeshModel3D
{
    private : 
        std::list<SimpleUiTileRef> tiles;

    public : 


};


#endif