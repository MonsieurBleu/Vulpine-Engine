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

FastUI_context::FastUI_context(
        SimpleUiTileBatchRef& tileBatch, 
        FontRef& font, 
        Scene& scene, 
        MeshMaterial fontMaterial)
    : tileBatch(tileBatch), font(font), scene(scene), fontMaterial(fontMaterial)
{}

FastUI_element::FastUI_element(FastUI_context& ui) : ObjectGroupRef(new ObjectGroup), ui(ui)
{}

vec2 FastUI_element::getSize()
{
    return size;
}

FastUI_menuTitle::FastUI_menuTitle(FastUI_context& ui, std::u32string name) : FastUI_element(ui)
{
    title = SingleStringBatchRef(new SingleStringBatch);
    title->setMaterial(ui.fontMaterial);
    title->setFont(ui.font);
    title->uniforms.add(ShaderUniform(&ui.colorTitleFont, 32));

    title->text = name;
    title->batchText();
    title->state.setPosition(vec3(0, 0, 1.0));
    (*this)->add(title);

    vec2 padding = vec2(0.025);
    vec2 bSize = title->getSize() + padding + padding;

    background = SimpleUiTileRef(new SimpleUiTile(
        ModelState3D()
            .setScale(vec3(bSize, 1)),
        UiTileType::SQUARE,
        ui.colorTitleBackground));

    ui.tileBatch->add(background);

    // background->setPosition(vec3(vec2(-0.5, 0.5)*title->getSize()*padding,0));
    title->state.setPosition(vec3(padding.x, -padding.y,0));

    (*this)->add(background);

    this->size = vec2(background->scale);

    ui.batchNeedUpdate = true;

    ui.scene.add(dynamic_cast<ObjectGroupRef&>(*this));
}

UFT32Stream& FastUI_value::toString(UFT32Stream& os)
{
    os << textPrev;

    const void *v = constValue ? constValue : value;

    switch (type)
    {
        case FUI_float : os << *(const float*)v; break;
        case FUI_int : os << *(const int*)v; break;
        default: os << U"[error type]";break;
    }

    os << textAfter;

    return os;
}

// FastUI_valueTab& FastUI_valueTab::add(FastUI_value v)
// {
//     push_back(v);
//     return *this;
// }


FastUI_valueTab::FastUI_valueTab(FastUI_context& ui, std::vector<FastUI_value> values) 
    : FastUI_element(ui), std::vector<FastUI_value>(values)
{
    text = SingleStringBatchRef(new SingleStringBatch);
    text->setMaterial(ui.fontMaterial);
    text->setFont(ui.font);
    text->uniforms.add(ShaderUniform(&ui.colorFont, 32));
    text->state.setPosition(vec3(padding.x, -padding.y,0));

    background = SimpleUiTileRef(new SimpleUiTile(ModelState3D(), UiTileType::SQUARE,ui.colorBackground));

    ui.tileBatch->add(background);
    (*this)->add(background);
    (*this)->add(text);

    ui.scene.add(dynamic_cast<ObjectGroupRef&>(*this));
}

FastUI_valueTab& FastUI_valueTab::batch()
{
    UFT32Stream newText;

    int size = dynamic_cast<std::vector<FastUI_value>&>(*this).size();
    int i = 0;
    for(; i < size-1; i++)
        (*this)[i].toString(newText) << U"\n\n";
    (*this)[i].toString(newText);

    text->text = newText.str();
    text->batchText();

    background->setScale(vec3(text->getSize() + padding + padding, 1));

    ui.batchNeedUpdate = true;

    return *this;
}
