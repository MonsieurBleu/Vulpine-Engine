
#pragma once

#ifndef VULPINE_COMPONENT_IMPL
#include <ECS/EngineComponents.hpp>
#endif

#include <Graphics/Scene.hpp>
#include <AssetManager.hpp>
#include <Globals.hpp>

template<> void Component<WidgetSprite>::ComponentElem::init()
{
    FastUI_context *ui = entity->comp<WidgetUI_Context>();
    data.sprite = newModel(ui->spriteMaterial);

    data.sprite->noBackFaceCulling = true;
    data.sprite->state.frustumCulled = false;
    // data.sprite->depthWrite = false;

    GenericSharedBuffer buff(new char[sizeof(vec3)*6]);
    vec3 *pos = (vec3*)buff.get();

    int id = 0;

    pos[id++] = vec3(0, 0, 0);
    pos[id++] = vec3(1, -1, 0);
    pos[id++] = vec3(1, 0, 0);

    pos[id++] = vec3(0, 0, 0);
    pos[id++] = vec3(0, -1, 0);
    pos[id++] = vec3(1, -1, 0);

    data.sprite->setVao(new
        VertexAttributeGroup({
            VertexAttribute(buff, 0, 6, 3, GL_FLOAT, false)
        })
    );

    data.sprite->getVao()->generate();

    data.sprite->setMap(0, Loader<Texture2D>::get(data.name));

    data.sprite->state.scaleScalar(0);

    globals.getScene2D()->add(data.sprite);
};

template<> void Component<WidgetSprite>::ComponentElem::clean()
{
    globals.getScene2D()->remove(data.sprite);
};

template<> void Component<WidgetBackground>::ComponentElem::init()
{
    FastUI_context *ui = entity->comp<WidgetUI_Context>();

    data.tile = SimpleUiTileRef(new SimpleUiTile(
        ModelState3D()
        , UiTileType::SQUARE_ROUNDED
        // , UiTileType::SQUARE
        , ui->colorTitleBackground));

    ui->tileBatch->add(data.tile);

    data.tile->scaleScalar(0);

    data.batch = ui->tileBatch;
}

template<> void Component<WidgetBackground>::ComponentElem::clean()
{
    data.batch->remove(data.tile);
}


template<> void Component<WidgetText>::ComponentElem::init()
{
    FastUI_context *ui = entity->comp<WidgetUI_Context>();

    data.mesh = SingleStringBatchRef(new SingleStringBatch);
    data.mesh->setMaterial(ui->fontMaterial);
    data.mesh->setFont(ui->font);
    data.mesh->color = ui->colorFont;
    data.mesh->uniforms.add(ShaderUniform(&data.mesh->color, 32));
    data.mesh->state.scaleScalar(0);

    if(!data.text.size()) 
        data.text = UFTconvert.from_bytes(entity->comp<EntityInfos>().name);

    globals.getScene2D()->add(data.mesh);
}

template<> void Component<WidgetText>::ComponentElem::clean()
{
    globals.getScene2D()->remove(data.mesh);
}


COMPONENT_DEFINE_SYNCH(WidgetBox)
{
    auto &box = child->comp<WidgetBox>();

    bool hidden = child->hasComp<WidgetState>() && child->comp<WidgetState>().status == ModelStatus::HIDE;

    if(child->hasComp<WidgetState>() && child->comp<WidgetState>().status == ModelStatus::HIDE)
        return;

    vec2 tmpMin = box.min;
    vec2 tmpMax = box.max;

    /********** MODYFING BOX VALUES **********/
    if(&parent != child.get())
    {
        auto &parentBox = parent.comp<WidgetBox>();

        box.depth = parentBox.depth+0.0001;

        if(parent.hasComp<WidgetStyle>() && parent.comp<WidgetStyle>().automaticTabbing != 0)
        {
            int id = 0;
            auto &parentGroup = parent.comp<EntityGroupInfo>();

            for(auto &e : parentGroup.children)
            {
                if(e.get() == child.get())
                    break;

                id++;
            }

            int nbLine = parent.comp<WidgetStyle>().automaticTabbing;
            int nbRow = (parentGroup.children.size() -1)/nbLine + 1;

            vec2 idim = 1.f/vec2(nbRow, nbLine);
            vec2 tabCoord = vec2(id%nbRow, id/nbRow)*idim;

            box.initMin = tabCoord + idim*0.1f ;
            box.initMax = tabCoord - idim*0.1f + idim;

            box.initMin = box.initMin*2.f - 1.f;
            box.initMax = box.initMax*2.f - 1.f;

            // box.min = parentBox.min + idim*0.1f + tabCoord * parentScale;
            // box.max = parentBox.min - idim*0.1f + (tabCoord + idim) * parentScale;
        }

        vec2 parentMax(+1);
        vec2 parentMin(-1);

        switch (box.type)
        {
            case WidgetBox::Type::FOLLOW_PARENT_BOX :
                parentMax = parentBox.max;
                parentMin = parentBox.min;
                break;

            case WidgetBox::Type::FOLLOW_SIBLINGS_BOX :
                parentMax = parentBox.childrenMax;
                parentMin = parentBox.childrenMin;
                break;

            default: break;
        }

        vec2 parentScale = (parentBox.max - parentBox.min)*0.5f;

        vec2 size = (box.initMax - box.initMin) * parentScale;
        // vec2 pos = (parentMax + parentMin)*0.5f + box.initMin * (parentMax - parentMin)*0.5f;
        // vec2 pos = (parentMax + parentMin)*0.5f + box.initMin * parentScale;
        vec2 pos = (parentBox.max + parentBox.min)*0.5f + box.initMin * parentScale;

        box.min = pos;
        box.max = pos + size;

        box.min += step(box.min, vec2(1)) * (parentMax - parentBox.max);
        box.max += step(box.min, vec2(1)) * (parentMax - parentBox.max);

        box.min -= step(-box.max, vec2(1)) * (parentBox.min - parentMin);
        box.max -= step(-box.max, vec2(1)) * (parentBox.min - parentMin);

        parentBox.childrenMax = max(parentBox.childrenMax, box.max);
        parentBox.childrenMax = max(parentBox.childrenMax, box.min);
        parentBox.childrenMax = max(parentBox.childrenMax, box.childrenMax);
        parentBox.childrenMax = max(parentBox.childrenMax, box.childrenMin);

        parentBox.childrenMin = min(parentBox.childrenMin, box.max);
        parentBox.childrenMin = min(parentBox.childrenMin, box.min);
        parentBox.childrenMin = min(parentBox.childrenMin, box.childrenMax);
        parentBox.childrenMin = min(parentBox.childrenMin, box.childrenMin);
    }
    else
    {
        box.childrenMax = box.max;
        box.childrenMin = box.min;
        // box.max = box.initMax;
        // box.min = box.initMin;
    }

    // if(&parent != child.get())
    //     std::cout << "parent - parent " << box.wasUninitializedLastSynch << "\n";
    // else
    //     std::cout << "parent - child  " << box.wasUninitializedLastSynch << "\n";

    /********** APPLYING SMOOTHING ON DISPLAY **********/

    float time = globals.appTime.getElapsedTime();

    if(box.min != tmpMin || box.max != tmpMax)
    {
        box.lastChangeTime = time;
        box.lastMin = tmpMin;
        box.lastMax = tmpMax;
    }
    
    if(box.displayMax.x == UNINITIALIZED_FLOAT)
    {
        // box.lastMin = mix(box.min, box.max, 0.25f) - (box.min + box.max)*0.5f;
        // box.lastMax = mix(box.max, box.min, 0.25f) - (box.min + box.max)*0.5f;
        box.lastMin = box.lastMax = vec2(0);
        box.lastChangeTime = time;
    }

    float a = smoothstep(0.0f, 1.f, (time - box.lastChangeTime)*4.f);

    box.displayMin = mix(box.lastMin, box.min, a);
    box.displayMax = mix(box.lastMax, box.max, a);

    /*
        TODO : move this into a system
    */
    /********** APPLYING DISPLAY VALUES ON ALL RELATED COMPONENTS **********/


    vec2 scale = vec2(box.displayMax - box.displayMin);
    vec2 pos = box.displayMin * vec2(1, -1);

    // vec2 scale = vec2(box.max - box.min);
    // vec2 pos = box.min * vec2(1, -1);

    scale = max(scale, vec2(0));


    if(child->hasComp<WidgetBackground>())
    {
        child->comp<WidgetBackground>().tile->setScale(vec3(scale, 1)).setPositionXY(pos).setPositionZ(box.depth);
    }

    if(child->hasComp<WidgetSprite>())
    {
        child->comp<WidgetSprite>().sprite->state.setScale(vec3(scale, 1)).setPositionXY(pos).setPositionZ(box.depth+0.000025);
    }

    float iaspectRatio = (float)(globals.windowWidth())/(float)(globals.windowHeight());

    if(child->hasComp<WidgetText>())
    {
        auto &text = child->comp<WidgetText>();
        text.mesh->text = text.text;
        text.mesh->batchText();

        vec2 tsize = text.mesh->getSize();

        tsize.y *= iaspectRatio;

        vec2 tscale = (scale*vec2(0.95, 0.8)/tsize);

        tscale = vec2(min(tscale.x, tscale.y));
        tscale.y *= iaspectRatio;

        text.mesh->state 
            .setPositionXY(
                pos + vec2(0.5f, -0.5f)*(scale - text.mesh->getSize()*tscale))
            .setPositionZ(box.depth+0.00005)
            .setScale(vec3(tscale, 1));
    }
}

COMPONENT_DEFINE_SYNCH(WidgetState)
{
    auto &up = child->comp<WidgetState>();

    if(&parent != child.get())
    {
        auto &parentUp = parent.comp<WidgetState>();

        switch (parentUp.statusToPropagate)
        {
            case ModelStatus::HIDE :
            // case ModelStateHideStatus::UNDEFINED :
                up.status = ModelStatus::HIDE;
                up.statusToPropagate = up.status;
                break;
            
            case ModelStatus::SHOW :
                if(child->hasComp<WidgetBox>() && up.status == ModelStatus::HIDE)
                {
                    auto &b = child->comp<WidgetBox>();
                    b.displayMin = b.displayMax = vec2(UNINITIALIZED_FLOAT);
                }

                up.status = ModelStatus::SHOW;
                break;

            default: break;
        }
    }

    if(up.statusToPropagate == ModelStatus::UNDEFINED)
        up.statusToPropagate = up.status;

    if(child->hasComp<WidgetBackground>() && child->comp<WidgetBackground>().tile.get())
        child->comp<WidgetBackground>().tile->setHideStatus(up.status);
    
    if(child->hasComp<WidgetText>() && child->comp<WidgetText>().mesh.get())
        child->comp<WidgetText>().mesh->state.setHideStatus(up.status);
}

// COMPONENT_DEFINE_SYNCH(WidgetStyle)
// {
//     if(&parent != child.get()) return;

//     auto &style = child->comp<WidgetStyle>();

//     if(child->hasComp<WidgetBackground>() && child->comp<WidgetBackground>().tile.get())
//     {
//         // child->comp<WidgetBackground>().tile->setHideStatus(up.status);

//         auto & b = child->comp<WidgetBackground>();

//         b.tile->color 
//     }
    
//     if(child->hasComp<WidgetText>() && child->comp<WidgetText>().mesh.get())
//         child->comp<WidgetText>().mesh->state.setHideStatus(up.status);
// }

void updateWidgetsStyle()
{
    System<WidgetStyle, WidgetBackground>([](Entity &entity)
    {
        if(entity.hasComp<WidgetState>() && entity.comp<WidgetState>().status == ModelStatus::HIDE)
            return;

        auto &s = entity.comp<WidgetStyle>();
        auto &b = entity.comp<WidgetBackground>();

        b.tile->color = s.useAltBackgroundColor ? s.backgroundColor2 : s.backgroundColor1;
    });

    System<WidgetStyle, WidgetText>([](Entity &entity)
    {
        if(entity.hasComp<WidgetState>() && entity.comp<WidgetState>().status == ModelStatus::HIDE)
            return;

        auto &s = entity.comp<WidgetStyle>();
        auto &t = entity.comp<WidgetText>();

        t.mesh->color = s.useAltTextColor ? s.textColor2 : s.textColor1;
    });
}

void updateEntityCursor(vec2 screenPos, bool down, bool click, WidgetUI_Context& ui)
{
    screenPos = (screenPos/vec2(globals.windowSize()))*2.f - 1.f;

    static EntityRef cursorHelp = newEntity("cursor button help", ui
            , WidgetState()
            , WidgetBox(vec2(0), vec2(0))
            , WidgetBackground()
            , WidgetText(U"")
            , WidgetStyle().setbackgroundColor1(vec4(vec3(0.1), 1)).settextColor1(vec4(0.9, 0.9, 0.25, 1))
    );

    static Entity *entityUnderCursor = nullptr;
    static float lastTimeChangeEUC = 0.f;

    Entity *lastEntityUnderCursor = entityUnderCursor;
    entityUnderCursor = nullptr;

    System<WidgetBox, WidgetButton, WidgetState>([screenPos, down, click](Entity &entity)
    {
        auto &box = entity.comp<WidgetBox>();
        auto &button = entity.comp<WidgetButton>();
        auto &state = entity.comp<WidgetState>();

        auto &style = entity.comp<WidgetStyle>();
        auto &group = entity.comp<EntityGroupInfo>();

        if(button.valueUpdate)
            button.cur = button.valueUpdate();

        if(entity.hasComp<WidgetStyle>())
            switch (button.type)
            {
                case WidgetButton::Type::HIDE_SHOW_TRIGGER : 
                    style.useAltBackgroundColor = state.statusToPropagate == ModelStatus::HIDE && group.children.size();
                    break;
                
                case WidgetButton::Type::CHECKBOX :
                    style.useAltBackgroundColor = button.cur == 0.f;
                    break;

                default : break; 
            }


        if(state.status == ModelStatus::HIDE)
            return;

        vec2 cursor = ((screenPos-box.min)/(box.max - box.min));

        if(cursor.x < 0 || cursor.y < 0 || cursor.x > 1 || cursor.y > 1)
            return;

        entityUnderCursor = &entity;

        switch (button.type)
        {
            case WidgetButton::Type::HIDE_SHOW_TRIGGER :
                if(click)
                {
                    switch (state.statusToPropagate)
                    {
                        case ModelStatus::HIDE :
                            state.statusToPropagate = ModelStatus::SHOW;
                            // if(entity.hasComp<WidgetStyle>())
                            //     entity.comp<WidgetStyle>().useAltBackgroundColor = false;
                            break;

                        case ModelStatus::SHOW :
                        case ModelStatus::UNDEFINED :
                            state.statusToPropagate = ModelStatus::HIDE;
                            // if(entity.hasComp<WidgetStyle>())
                            //     entity.comp<WidgetStyle>().useAltBackgroundColor = true;
                            break;

                        default: break;
                    }
                }
                break;

            case WidgetButton::Type::CHECKBOX :
                if(click)
                {
                    button.cur = button.cur == 0.f ? 1.f : 0.f;
                    button.valueChanged(button.cur);
                }
                break;

            default:
                break;
        }

    });

    if(lastEntityUnderCursor != entityUnderCursor)
        lastTimeChangeEUC = globals.appTime.getElapsedTime();

    if(entityUnderCursor)
    {
        float t = globals.appTime.getElapsedTime() - lastTimeChangeEUC;

        cursorHelp->comp<WidgetState>().status = ModelStatus::SHOW;

        cursorHelp->comp<WidgetBox>().set(
            vec2(screenPos.x, screenPos.x+0.2),
            vec2(screenPos.y-0.04, screenPos.y)
        );

        cursorHelp->comp<WidgetText>().text = UFTconvert.from_bytes(entityUnderCursor->comp<EntityInfos>().name);

        float a = smoothstep(0.f, 1.f, (t-0.5f)*2.5f)*0.9;
        cursorHelp->comp<WidgetStyle>().backgroundColor1.a = a;
        cursorHelp->comp<WidgetStyle>().textColor1.a = a;

        cursorHelp->comp<WidgetBox>().depth = 0.9;
    }
    else
    {
        cursorHelp->comp<WidgetState>().status = ModelStatus::HIDE;
    }

    /* TODO : maybe remove later*/
    ComponentModularity::synchronizeChildren(cursorHelp);
}