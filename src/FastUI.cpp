#include <FastUI.hpp>
#include <Globals.hpp>
#include <MathsUtils.hpp>
#include <Scene.hpp>
#include <Constants.hpp>

SimpleUiTile::SimpleUiTile(ModelState3D state, UiTileType tileType, vec4 color)
    : ModelState3D(state), tileType(tileType), color(color)
{
    frustumCulled = false;
}

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
        if(i->hide == ModelStateHideStatus::HIDE) continue;
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

    vec2 padding = vec2(1.0, 0.5)*title->charSize;
    vec2 bSize = vec2(title->getSize().x, title->charSize*title->state.scale.y*1.6);
    bSize += padding + padding;

    background = SimpleUiTileRef(new SimpleUiTile(
        ModelState3D()
            .setScale(vec3(bSize, 1)),
        UiTileType::SQUARE,
        ui.colorTitleBackground));

    ui.tileBatch->add(background);

    // background->setPosition(vec3(vec2(-0.5, 0.5)*title->getSize()*padding,0));
    // title->state.setPosition(vec3(padding.x, -padding.y, 0));
    title->state.setPosition(vec3(vec2(0.5, -0.5)*(bSize-title->getSize()), 0));

    (*this)->add(background);

    this->size = vec2(background->scale);

    ui.batchNeedUpdate = true;

    // ui.scene.add(dynamic_cast<ObjectGroupRef&>(*this));
}

UFT32Stream& FastUI_value::toString(UFT32Stream& os)
{
    if(constValue)
        os << U"**";

    os << textPrev;

    if(constValue || !globals.canUseTextInputs(this))
    {
        const void *v = constValue ? constValue : value;

        switch (type)
        {
            case FUI_float : os << *(const float*)v; break;
            case FUI_int : os << *(const int*)v; break;
            case FUI_bool :
                if(*(const bool*)v)
                    os << U"true";
                else
                    os << U"false";
            break;
            case FUI_vec3DirectionPhi :
                os << degrees(getPhiTheta(*(const vec3*)v).x);
                break;
            case FUI_vec3DirectionTheta : 
                os << -degrees(getPhiTheta(*(const vec3*)v).y); 
                break;
            
            case FUI_vec3Color : 
                os << rgbtou32str(*(const vec3*)v);
                break;
            
            case FUI_vec3Hue :
                os << (int)round(rgb2hsv(*(const vec3*)v).r*360.f);
                break;

            case FUI_vec3Saturation :
                os << (int)round(rgb2hsv(*(const vec3*)v).g*100.f);
                break;

            case FUI_vec3Value :
                os << (int)round(rgb2hsv(*(const vec3*)v).b*100.f);
                break;
            
            case FUI_floatAngle : 
                os << degrees(*(const float*)v); break;

            case FUI_noValue : break;
            default: os << U"[error type]";break;
        }

        os << textAfter;
    }
    else
    {
        std::u32string ti;
        globals.getTextInputs(this, ti);
        if(ti.back() == U'\n')
        {
            ti.pop_back();
            globals.endTextInputs(this);

            float mod = 0.f;

            if(ti.size() > 2)
            {
                if(ti[0] == U'+' && ti[1] == U'+')
                {
                    mod = 1.0;
                    ti.erase(0, 2);
                }
                if(ti[0] == U'-' && ti[1] == U'-')
                {
                    mod = 1.0;
                    ti.erase(0, 1);
                }
            }

            switch (type)
            {
            case FUI_float :
            {
                float f;
                float &v = *(float*)value;
                v = !u32strtof(ti, f) ? v : mod == 0.f ? f : mod*v + f;
            }
                break;
            
            case FUI_int :
            {
                float f;
                int &v = *(int*)value;
                v = !u32strtof(ti, f) ? v : mod == 0.f ? f : mod*v + f;
            }
                break;
            
            case FUI_vec3DirectionPhi :
            {
                float f;
                if(u32strtof(ti, f))
                {
                    vec3 &v = *(vec3*)value;
                    v = setPhi(v, radians(f));
                }
            }
                break;

            case FUI_vec3DirectionTheta :
            {
                float f;
                if(u32strtof(ti, f))
                {
                    vec3 &v = *(vec3*)value;
                    v = setTheta(v, radians(-f));
                }
            }
                break;

            case FUI_vec3Color : 
                {
                    vec3 v;
                    if(u32strtocolorHTML(ti, v))
                        *(vec3*)value = v;
                }
                break;

            case FUI_vec3Hue :
                {
                    float f;
                    if(u32strtof(ti, f))
                    {
                        vec3 &v = *(vec3*)value;
                        vec3 hsv = rgb2hsv(v);
                        f /= 360.f;
                        float hue = mod == 0.f ? f : hsv.x*mod + f;

                        v = hsv2rgb(vec3(hue, hsv.y, hsv.z));
                    }
                }
                break;

            case FUI_vec3Saturation :
                {
                    float f;
                    if(u32strtof(ti, f))
                    {
                        vec3 &v = *(vec3*)value;
                        vec3 hsv = rgb2hsv(v);
                        f /= 100.f;
                        float sat = mod == 0.f ? f : hsv.y*mod + f;

                        v = hsv2rgb(vec3(hsv.x, sat, hsv.z));
                    }
                }
                break;

            case FUI_vec3Value :
                {
                    float f;
                    if(u32strtof(ti, f))
                    {
                        vec3 &v = *(vec3*)value;
                        vec3 hsv = rgb2hsv(v);
                        f /= 100.f;
                        float val = mod == 0.f ? f : hsv.z*mod + f;

                        v = hsv2rgb(vec3(hsv.x, hsv.y, val));
                    }
                }
                break;

            case FUI_floatAngle : 
            {
                float f;
                if(u32strtof(ti, f))
                {
                    float &v = *(float*)value;
                    f = radians(f);
                    v = mod == 0.f ? f : v*mod + f;
                    v = glm::mod(v, (float)(2.f*PI));
                }
            }
                break;

            default:
                break;
            }


            os << textAfter;
        }
        else
        {
            float time = globals.appTime.getElapsedTime();
            time = mod(time, 2.f);

            if(time > 1.0)
                ti.push_back(U'.');
            else
                ti.push_back(U' ');
                
            if(time > 1.3)
                ti.push_back(U'.');

            if(time > 1.6)
                ti.push_back(U'.');

            ti = U"= " + ti;

            // std::cout << time << "\n";
        }

        os << ti;
    }

    if(constValue)
        os << U"**";

    return os;
}

 void FastUI_value::getModifiedByTextInput()
 {
    if(type == FUI_bool && !constValue)
    {
        bool &v = *(bool*)value;
        v = !v;
    }
    else if(type != FUI_noValue)
        globals.useTextInputs(this);
 }

FastUI_valueTab::FastUI_valueTab(FastUI_context& ui, const std::vector<FastUI_value> &values) 
    : FastUI_element(ui), std::vector<FastUI_value>(values)
{
    text = SingleStringBatchRef(new SingleStringBatch);
    text->setMaterial(ui.fontMaterial);
    text->setFont(ui.font);
    text->uniforms.add(ShaderUniform(&ui.colorFont, 32));
    padding = vec2(1.0)*vec2(text->charSize);
    text->state.setPosition(vec3(padding.x, -padding.y,0));
    background = SimpleUiTileRef(new SimpleUiTile(ModelState3D(), UiTileType::SQUARE,ui.colorBackground));

    ui.tileBatch->add(background);
    (*this)->add(background);
    (*this)->add(text);

    // ui.scene.add(dynamic_cast<ObjectGroupRef&>(*this));
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

    this->FastUI_element::size = vec2(background->scale);

    return *this;
}

void FastUI_valueTab::changeBackgroundColor(vec4 color)
{
    background->color = color;
    ui.batchNeedUpdate = true;
}

void FastUI_menuTitle::changeBackgroundColor(vec4 color)
{
    background->color = color;
    ui.batchNeedUpdate = true;
}

FastUI_valueMenu::FastUI_valueMenu(FastUI_context& ui, const std::vector<FastUI_titledValueTab> &values)
    : FastUI_element(ui), 
    std::vector<FastUI_titledValueTab>(values), 
    elements(*this)
{

}

void FastUI_valueMenu::batch()
{
    ui.scene.remove(dynamic_cast<ObjectGroupRef&>(*this));
    ObjectGroupRef& g = dynamic_cast<ObjectGroupRef&>(*this);

    int size = elements.size();

    vec2 titleOffset = vec2(0.f);

    for(int i = 0; i < size; i++)
    {
        FastUI_titledValueTab &e = elements[i];
        
        g->add(dynamic_cast<ObjectGroupRef&>(e.title));

        e.title->state.setPosition(vec3(titleOffset.x, 0, 0));

        vec2 tSize = e.title.getSize();
        titleOffset.x += tSize.x;
        titleOffset.y = tSize.y > titleOffset.y ? tSize.y : titleOffset.y;
    }

    for(int i = 0; i < size; i++)
    {
        FastUI_titledValueTab &e = elements[i];

        // e.title->state.setPosition(
        //     e.title->state.position - 
        //     vec3(0, titleOffset.y-e.title.getSize().y, 0)
        // );

        e.tab.batch();
        e.tab->state.scaleScalar(1.0);
        e.tab->state.setPosition(vec3(0, -titleOffset.y, 0));
        e.tab->state.setHideStatus(ModelStateHideStatus::HIDE);
        g->add(dynamic_cast<ObjectGroupRef&>(e.tab));
    }

    // elements[0].tab->state.hide = false;

    ui.scene.add(dynamic_cast<ObjectGroupRef&>(*this));
}

void FastUI_valueMenu::setCurrentTab(int id)
{
    // if(currentTab == id) return;
    if(currentTab >= 0)
    {
        elements[currentTab].tab->state.setHideStatus(ModelStateHideStatus::HIDE);
        elements[currentTab].title.changeBackgroundColor(ui.colorTitleBackground);
    }
    if(currentTab != id)
    {
        currentTab = id;
        elements[currentTab].tab->state.setHideStatus(ModelStateHideStatus::SHOW);
        elements[currentTab].title.changeBackgroundColor(ui.colorCurrentTitleBackground);
    }
    else
        currentTab = -1;
}

void FastUI_valueMenu::trackCursor()
{
    currentTab = currentTab < (int)elements.size() ? currentTab : -1;

    vec2 mpos = globals.mousePosition()/vec2(globals.windowSize());
    float iaspectRatio = (float)(globals.windowHeight())/(float)(globals.windowWidth());
    // mpos = mpos*vec2(2.0) - vec2(1.0);
    mpos = vec2(mpos.x*2.0 - 1.0, 1.0 - mpos.y*2.0);
    mpos.y *= iaspectRatio;

    get()->state.update();
    
    mpos = vec2(vec4( inverse(get()->state.modelMatrix) * vec4(mpos, 0, 1) ));    

    mpos.y *= -1;

    int size = elements.size();
    for(int i = 0; i < size; i++)
    {
        FastUI_titledValueTab &e = elements[i];
        vec2 boundmin = vec2(e.title->state.position);
        vec2 bound = boundmin + e.title.getSize();

        if(mpos.x >= boundmin.x && mpos.y >= boundmin.y && mpos.x <= bound.x && mpos.y <= bound.y)
        {
            if(globals.mouseLeftClick())
                setCurrentTab(i);
            else if (i != currentTab)
                e.title.changeBackgroundColor(ui.colorHighlightedTitleBackground);
        }
        else if(i != currentTab)
            e.title.changeBackgroundColor(ui.colorTitleBackground);
    }  

    if(globals.mouseLeftClick() && currentTab >= 0)
    {
        FastUI_titledValueTab &e = elements[currentTab];
        vec2 boundmin = vec2(e.tab->state.position)*vec2(1, -1);
        vec2 bound = boundmin + e.tab.getSize() * vec2(e.tab->state.scale);

        // std::cout << boundmin.x << " " << boundmin.y << "\t";
        // std::cout << bound.x << " " << bound.y << "\t";
        // std::cout << mpos.x << " " << mpos.y << "\t";

        if(mpos.x >= boundmin.x && mpos.y >= boundmin.y && mpos.x <= bound.x && mpos.y <= bound.y)
        {
            int line = (mpos.y-boundmin.y)/(bound.y-boundmin.y)*e.tab.std::vector<FastUI_value>::size();
            e.tab[line].getModifiedByTextInput();
        }
    }
}

void FastUI_valueMenu::updateText()
{
    int size = elements.size();
    for(int i = 0; i < size; i++)
    {
        FastUI_titledValueTab &e = elements[i];
        e.tab.batch();
    }
}

void BenchTimer::setMenu(FastUI_valueMenu &menu)
{
    std::u32string name32 = name.size() ? UFTconvert.from_bytes(name) : U"untitled";

    menu.push_back(
        // {FastUI_menuTitle(menu.ui, UFTconvert.from_bytes(name)), FastUI_valueTab(menu.ui, {
        {FastUI_menuTitle(menu.ui, name32), FastUI_valueTab(menu.ui, {
            FastUI_value((const float*)&avgLast,   U"Short avg\t", U" ms"),
            FastUI_value((const float*)&avgTotal,   U"Total avg\t", U" ms"),
            FastUI_value((const int*)&updateCounter,   U"Updates (frames)\t", U""),
            FastUI_value((const float*)&elapsedTime,   U"Elapsed time\t", U" s"),
            FastUI_value((bool*)&paused,   U"Paused\t", U""),
            FastUI_value((const float*)&min,   U"Min\t", U" ms"),
            FastUI_value((const float*)&max,   U"Max\t", U" ms"),
            FastUI_value((float*)&speed,   U"Speed Factor\t")
        })}
    );
}

void BenchTimer::setMenuConst(FastUI_valueMenu &menu) const
{
    std::u32string name32 = name.size() ? UFTconvert.from_bytes(name) : U"untitled";

    menu.push_back(
        {FastUI_menuTitle(menu.ui, U"**"+name32+U"**"), FastUI_valueTab(menu.ui, {
            FastUI_value((const float*)&avgLast,   U"Short avg\t", U" ms"),
            FastUI_value((const float*)&avgTotal,   U"Total avg\t", U" ms"),
            FastUI_value((const int*)&updateCounter,   U"Updates (frames)\t", U""),
            FastUI_value((const float*)&elapsedTime,   U"Elapsed time\t", U" s"),
            FastUI_value((const bool*)&paused,   U"Paused\t", U""),
            FastUI_value((const float*)&min,   U"Min\t", U" ms"),
            FastUI_value((const float*)&max,   U"Max\t", U" ms"),
            FastUI_value((const float*)&speed,   U"Speed Factor\t")
        })}
    );
}

void LimitTimer::setMenu(FastUI_valueMenu &menu)
{
    menu.push_back(
        {FastUI_menuTitle(menu.ui, U"FPS Limiter"), FastUI_valueTab(menu.ui, {
            FastUI_value(&freq, U"Frequency\t", U"\ffps"), 
            FastUI_value(&activated, U"Activated\t", U"")
        })}
    );
}

void DirectionLight::setMenu(FastUI_valueMenu &menu, std::u32string name)
{
    menu.push_back(
        {FastUI_menuTitle(menu.ui, name), FastUI_valueTab(menu.ui, {
            FastUI_value(&infos._color.a, U"Intensity\t"),
            FastUI_value(U"Direction"),
            FastUI_value((vec3*)&infos._direction,  U"\fphi\t", U"°", FUI_vec3DirectionPhi),
            FastUI_value((vec3*)&infos._direction,  U"\ftheta\t", U"°", FUI_vec3DirectionTheta),
            FastUI_value(U"Color"),
            FastUI_value((vec3*)&infos._color, U"\fHEX\t", U"", FUI_vec3Color),
            FastUI_value((vec3*)&infos._color, U"\fHue\t", U"°", FUI_vec3Hue),
            FastUI_value((vec3*)&infos._color, U"\fSaturation\t", U"%", FUI_vec3Saturation),
            FastUI_value((vec3*)&infos._color, U"\fLuminosity\t", U"%", FUI_vec3Value)
        })}
    );
}

void MeshModel3D::setMenu(FastUI_valueMenu &menu, std::u32string name)
{
    menu.push_back(
        {FastUI_menuTitle(menu.ui, name), FastUI_valueTab(menu.ui, {
            FastUI_value(U"State"),
            FastUI_value((bool*)&state, U"\fUpdate\t"),
            FastUI_value(U"\fPosition\t"),
            FastUI_value(&state.position.x,   U"\f\fx\t"),
            FastUI_value(&state.position.y,   U"\f\fy\t"),
            FastUI_value(&state.position.z,   U"\f\fz\t"),
            FastUI_value(U"\fScale\t"),
            FastUI_value(&state.scale.x,   U"\f\fx\t"),
            FastUI_value(&state.scale.y,   U"\f\fy\t"),
            FastUI_value(&state.scale.z,   U"\f\fz\t"),
            FastUI_value(U"\fRotation\t"),
            FastUI_value(&state.rotation.x,   U"\f\fx\t", U"°", FUI_floatAngle),
            FastUI_value(&state.rotation.y,   U"\f\fy\t", U"°", FUI_floatAngle),
            FastUI_value(&state.rotation.z,   U"\f\fz\t", U"°", FUI_floatAngle),
            FastUI_value(U"Tesselation Parameters"),            
            FastUI_value(&lodTessLevelDistance.x,   U"\fMin Tessellation\t", U" levels"),
            FastUI_value(&lodTessLevelDistance.y,   U"\fMax Tessellation\t", U" levels"),
            FastUI_value(&lodTessLevelDistance.z,   U"\fMin Distance\t", U"m"),
            FastUI_value(&lodTessLevelDistance.w,   U"\fMax Distance\t", U"m"),
            FastUI_value(U"Displacement Mapping Factors"),        
            FastUI_value(&lodHeightDispFactors.x,   U"\fUV Scale\t"),
            FastUI_value(&lodHeightDispFactors.y,   U"\fMax Height\t", U"m"),
            FastUI_value(U"Height Mapping Factors"),        
            FastUI_value(&lodHeightDispFactors.z,   U"\fUV Scale\t"),
            FastUI_value(&lodHeightDispFactors.w,   U"\fMax Height\t", U"m"),
        })}
    );
}

void ObjectGroup::setMenu(FastUI_valueMenu &menu, std::u32string name)
{
    menu.push_back(
        {FastUI_menuTitle(menu.ui, name), FastUI_valueTab(menu.ui, {
            FastUI_value(U"State"),
            FastUI_value((bool*)&state, U"\fUpdate\t"),
            FastUI_value(U"\fPosition\t"),
            FastUI_value(&state.position.x,   U"\f\fx\t"),
            FastUI_value(&state.position.y,   U"\f\fy\t"),
            FastUI_value(&state.position.z,   U"\f\fz\t"),
            FastUI_value(U"\fScale\t"),
            FastUI_value(&state.scale.x,   U"\f\fx\t"),
            FastUI_value(&state.scale.y,   U"\f\fy\t"),
            FastUI_value(&state.scale.z,   U"\f\fz\t"),
            FastUI_value(U"\fRotation\t"),
            FastUI_value(&state.rotation.x,   U"\f\fx\t", U"°", FUI_floatAngle),
            FastUI_value(&state.rotation.y,   U"\f\fy\t", U"°", FUI_floatAngle),
            FastUI_value(&state.rotation.z,   U"\f\fz\t", U"°", FUI_floatAngle)
        })}
    );

    for(auto i : meshes)
        i->setMenu(menu, name+U":Mesh");
}