#include <FastUI.hpp>

SimpleUiTile::SimpleUiTile(ModelState3D state, UiTileType tileType, vec4 color)
    : ModelState3D(state), tileType(tileType), color(color)
{}

SimpleUiTileBatch& SimpleUiTileBatch::add(SimpleUiTileRef tile)
{
    tiles.push_back(tile);
    return *this;
}

SimpleUiTileBatch& SimpleUiTileBatch::batch()
{
    size_t size = tiles.size();
    if(!size) return *this;
    depthWrite = false;
    // invertFaces = true;

    GenericSharedBuffer positions(new char[sizeof(vec3)*6*size]);
    GenericSharedBuffer colors(new char[sizeof(vec4)*6*size]);
    GenericSharedBuffer uvs(new char[sizeof(vec4)*6*size]);
    vec3 *p = (vec3 *)positions.get();
    vec4 *c = (vec4 *)colors.get();
    vec4 *u = (vec4 *)uvs.get();

    int usedTiles = 0;

    for(auto i : tiles)
    {
        if(i->hide) continue;
        i->update();
        mat4 m = i->modelMatrix;
        vec3 leftTop = vec3(m*vec4(0, 0, 0, 1));
        vec3 rightTop = vec3(m*vec4(1, 0, 0, 1));
        vec3 leftBottom = vec3(m*vec4(0, -1, 0, 1));
        vec3 rightBottom = vec3(m*vec4(1, -1, 0, 1));
        

        int id = usedTiles*6;
        p[id] = leftTop;
        p[id+1] = rightTop;
        p[id+2] = leftBottom;

        p[id+3] = leftBottom;
        p[id+4] = rightTop;
        p[id+5] = rightBottom;

        vec4 color[6] = {i->color, i->color, i->color, i->color, i->color, i->color};
        memcpy(&(c[id]), color, 6*sizeof(vec4));

        float aspectRatio = i->scale.x/i->scale.y;

        u[id] = vec4(-1, 1, i->tileType, aspectRatio);
        u[id+1] = vec4(1, 1, i->tileType, aspectRatio);
        u[id+2] = vec4(-1, -1, i->tileType, aspectRatio);

        u[id+3] = vec4(-1, -1, i->tileType, aspectRatio);
        u[id+4] = vec4(1, 1, i->tileType, aspectRatio);
        u[id+5] = vec4(1, -1, i->tileType, aspectRatio);

        usedTiles++;
    }

    if(!vao.get() || !vao->getHandle())
    {
        setVao(
            MeshVao(
                new VertexAttributeGroup({
                    VertexAttribute(positions, 0, usedTiles*6, 3, GL_FLOAT, false),
                    VertexAttribute(colors, 1, usedTiles*6, 4, GL_FLOAT, false),
                    VertexAttribute(uvs, 2, usedTiles*6, 4, GL_FLOAT, false)
                })
            )    
        );

        vao->generate();
    }
    else
    {
        vao->attributes[0].updateData(positions, usedTiles*6);
        vao->attributes[1].updateData(colors, usedTiles*6);
        vao->attributes[2].updateData(uvs, usedTiles*6);
    }

    return *this;
}
