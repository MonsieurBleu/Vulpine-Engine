
#pragma once

#include "Entity.hpp"
#ifndef VULPINE_COMPONENT_IMPL
#include <ECS/EngineComponents.hpp>
#endif

#include <Graphics/Scene.hpp>
#include <AssetManager.hpp>
#include <Globals.hpp>

#include <Scripting/ScriptInstance.hpp>

COMPONENT_DEFINE_REPARENT(EntityGroupInfo)
{
    // if(!child->hasComp<EntityGroupInfo>())
    // {
    //     child->set<EntityGroupInfo>(EntityGroupInfo());
    // }

    // child->comp<EntityGroupInfo>().parent = &parent;
}

template<> void Component<WidgetSprite>::ComponentElem::init()
{
    if(data.name.size())
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
    }

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
    if(data.batch)
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
    // if(&parent == child.get() && child->comp<EntityGroupInfo>().parent)
    //     return;

    
    auto &box = child->comp<WidgetBox>();
    box.synchCounter ++;

    bool hidden = child->hasComp<WidgetState>() && child->comp<WidgetState>().status == ModelStatus::HIDE;

    vec2 tmpMin = box.min;
    vec2 tmpMax = box.max;

    vec2 off = child.get() != &parent && parent.hasComp<WidgetBox>() ? parent.comp<WidgetBox>().scrollOffset : vec2(0);

    /********** MODYFING BOX VALUES **********/
    if(&parent != child.get())
    {
        auto &parentBox = parent.comp<WidgetBox>();
        box.depth = parentBox.depth+0.0001;

        box.useClassicInterpolation = box.useClassicInterpolation ? true : parentBox.useClassicInterpolation;

        if(box.specialFittingScript)
            box.specialFittingScript(&parent, child.get());

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
            int nbRow = ceil((float)parentGroup.children.size()/(float)nbLine);

            vec2 idim = 1.f/vec2(nbRow, nbLine);
            ivec2 tabID(id/nbLine, id%nbLine);
            vec2 tabCoord = vec2(tabID)*idim;

            vec2 screen(globals.windowSize());
            if(screen.x > screen.y)
            {
                screen.x = screen.y/screen.x;
                screen.y = 1;
            }
            else
            {
                screen.y = screen.x/screen.y;
                screen.x = 1;
            }

            vec2 scale = screen/(parentBox.max - parentBox.min)*0.5f;

            vec2 space = 1.25f*0.0075f*scale*WidgetBox::tabbingSpacingScale;

            if(parent.hasComp<WidgetStyle>() && !parent.comp<WidgetStyle>().useInternalSpacing)
                space = vec2(0);

            box.initMin = tabCoord + space;
            box.initMax = tabCoord - space + idim;

            box.initMin = box.initMin*2.f - 1.f;
            box.initMax = box.initMax*2.f - 1.f;
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

                if(box.synchCounter < 25)
                {
                    parentMax = parentMin = vec2(0);
                    // NOTIF_MESSAGE("TYO");
                }
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

        if(!hidden)
        {
            parentBox.childrenMax = max(parentBox.childrenMax, box.max);
            parentBox.childrenMax = max(parentBox.childrenMax, box.min);
            // parentBox.childrenMax = max(parentBox.childrenMax, box.childrenMax);
            // parentBox.childrenMax = max(parentBox.childrenMax, box.childrenMin);

            parentBox.childrenMin = min(parentBox.childrenMin, box.max);
            parentBox.childrenMin = min(parentBox.childrenMin, box.min);
            // parentBox.childrenMin = min(parentBox.childrenMin, box.childrenMax);
            // parentBox.childrenMin = min(parentBox.childrenMin, box.childrenMin);


        }

        if(parentBox.displayRangeMin != vec2(-UNINITIALIZED_FLOAT))
        {
            box.displayRangeMin = parentBox.displayRangeMin;
            // box.displayRangeMin = max(box.displayRangeMin, parentBox.displayRangeMin);
        }


        if(parentBox.displayRangeMax != vec2(UNINITIALIZED_FLOAT))
        {
            box.displayRangeMax = parentBox.displayRangeMax;
            // box.displayRangeMax = min(box.displayRangeMax, parentBox.displayRangeMax);
        }


        box.min += off;
        box.max += off;

        // box.min = max(box.displayRangeMin, box.min);
        // box.max = max(box.displayRangeMin, box.max);

        // box.min = min(box.displayRangeMax, box.min);
        // box.max = min(box.displayRangeMax, box.max);

    }
    else
    {
        // box.min = clamp(box.min, box.clampEdgeMin, box.childrenMax);
        // box.max = clamp(box.max, box.clampEdgeMin, box.childrenMax);

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

    if(
        // tmpMin != vec2(-1, -1) && tmpMax != vec2(1, 1) &&
        
        (box.min != tmpMin || box.max != tmpMax)
        // || box.lastChangeTime == time
        )
    {
        box.lastChangeTime = time;
        // box.lastMin = tmpMin;
        // box.lastMax = tmpMax;
        // box.lastMin = vec2(0);
        // box.lastMax = vec2(0);

        if(!box.useClassicInterpolation)
        {
            box.lastMin = (box.min + box.max)*0.5f;
            box.lastMax = (box.min + box.max)*0.5f;
        }
        else
        {
            box.lastMin = tmpMin;
            box.lastMax = tmpMax;
        }
    }

    // if(parent.comp<WidgetBox>().initMax.x == UNINITIALIZED_FLOAT)
    // {
    //     box.displayMax = box.displayMin = vec2(UNINITIALIZED_FLOAT);
    // }
    
    // if(box.displayMax.x == UNINITIALIZED_FLOAT && child.get() != &parent)
    // {
    //     // std::cout << "YOOOOOOOOOOOOO   " << box.min.x << "\t" << child->comp<EntityInfos>().name << "\n"; 
        
    //     if(!box.useClassicInterpolation)
    //     {
    //         box.lastMin = (box.min + box.max)*0.5f;
    //         box.lastMax = (box.min + box.max)*0.5f;

    //     }
    //     else
    //     {
    //         box.useClassicInterpolation = true;
    //     }

    //     // box.lastMin = box.lastMax = vec2(0);
    //     box.lastChangeTime = time;
    // }

    // if(child.get() == &parent && child->comp<EntityGroupInfo>().parent) 
    //     return;

    if(hidden) return;

    float a = WidgetBox::smoothingAnimationSpeed == 0.f ?
        1. :smoothstep(0.0f, 1.f, (time - box.lastChangeTime)*WidgetBox::smoothingAnimationSpeed);

    if(a < 1. && Loader<ScriptInstance>::loadingInfos.find("WidgetBox_SmoothSync") != Loader<ScriptInstance>::loadingInfos.end())
    {
        Loader<ScriptInstance>::get("WidgetBox_SmoothSync").run(box.lastMin, box.lastMax, box.min, box.max, a, (int)box.useClassicInterpolation);
        box.displayMin = threadState["displayMin"];
        box.displayMax = threadState["displayMax"];
    }
    else
    {
        box.displayMin = box.min;
        box.displayMax = box.max;
    }

    box.displayMin = max(box.displayRangeMin, box.displayMin);
    box.displayMax = max(box.displayRangeMin, box.displayMax);

    box.displayMin = min(box.displayRangeMax, box.displayMin);
    box.displayMax = min(box.displayRangeMax, box.displayMax);

    box.displayMin = min(box.displayMin, box.displayMax);
    // vec2 mintmp = min(box.displayMin, box.displayMax);
    // vec2 maxtmp = max(box.displayMin, box.displayMax);
    // box.displayMin = mintmp;
    // box.displayMax = maxtmp;
    

    // if(box.displayRangeMax != vec2(UNINITIALIZED_FLOAT))
    // {
    //     std::cout << "MAX = " << box.displayRangeMax.y << "\t";
    //     std::cout << "RESULT = " << box.displayMax.y << "\n";
    // }

    // if(hidden) return;

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

    // if(child->hasComp<WidgetSprite>())
    // {
    //     child->comp<WidgetSprite>().sprite->state.setScale(vec3(scale, 1)).setPositionXY(pos).setPositionZ(box.depth+0.000025);
    // }

    float iaspectRatio = (float)(globals.windowWidth())/(float)(globals.windowHeight());

    if(child->hasComp<WidgetText>())
    {
        auto &text = child->comp<WidgetText>();
        text.mesh->text = text.text;
        text.mesh->batchText();

        vec2 tsize = text.mesh->getSize();

        tsize.y = 1.5 * text.mesh->charSize;

        tsize.y *= iaspectRatio;

        // vec2 tscale = (scale*vec2(0.95, 0.8)/tsize);

        vec2 tscale = (scale*vec2(0.9, 0.75)/tsize);

        tscale = vec2(min(tscale.x, tscale.y));
        tscale.y *= iaspectRatio;

        vec2 initialPos;

        switch(text.align)
        {
            case CENTERED :
                initialPos = vec2(0.5, -0.5f);
                break;
            
            case TO_LEFT :
                initialPos = vec2(0.0, -0.5f);
                break;
            
            default : break;
        }

        text.mesh->state 
            .setPositionXY(
                pos + initialPos*(scale - text.mesh->getSize()*tscale))
            .setPositionZ(box.depth+0.00005)
            .setScale(vec3(tscale, 1));
    }
}

COMPONENT_DEFINE_SYNCH(WidgetState)
{
    auto &up = child->comp<WidgetState>();

    up.updateCounter ++;

    bool isIndirectHST = child->hasComp<WidgetButton>() && child->comp<WidgetButton>().type == WidgetButton::Type::HIDE_SHOW_TRIGGER_INDIRECT;

    if(&parent != child.get())
    {
        if(isIndirectHST)
        {
            return;
        }
            // NOTIF_MESSAGE(child->comp<EntityInfos>().name);

        auto &parentUp = parent.comp<WidgetState>();

        switch (parentUp.statusToPropagate)
        {
            case ModelStatus::HIDE :
                up.status = ModelStatus::HIDE;
                up.statusToPropagate = up.status;
                break;
            
            // case ModelStatus::UNDEFINED :
            case ModelStatus::SHOW :
                if(child->hasComp<WidgetBox>() && up.status == ModelStatus::HIDE)
                {
                    auto &b = child->comp<WidgetBox>();
                    // b.displayMin = b.displayMax = vec2(UNINITIALIZED_FLOAT);
                    // b.lastMin = b.lastMax = vec2(0);
                    // b.lastMax.y = b.min.y;
                    // b.lastMax.y = b.max.y;
                    // b.lastChangeTime = globals.appTime.getElapsedTime();
                    if(!b.useClassicInterpolation)
                        b.set(vec2(b.initMin.x ,b.initMax.x), vec2(b.initMin.y , b.initMax.y));
                    // b.set(vec2(0), vec2(0));
                }

                // if(up.status != ModelStatus::HIDE)
                    up.status = ModelStatus::SHOW;

                if(
                    !child->hasComp<WidgetButton>() || 
                    (
                        child->comp<WidgetButton>().type != WidgetButton::Type::HIDE_SHOW_TRIGGER
                        // &&
                        // child->comp<WidgetButton>().type != WidgetButton::Type::HIDE_SHOW_TRIGGER_INDIRECT
                    )
                )
                    up.statusToPropagate = ModelStatus::SHOW;

                break;

            default: break;
        }
    }
    else
    {
        switch (up.statusToPropagate)
        {
            case ModelStatus::HIDE :
                if(isIndirectHST)
                {
                    auto &button = child->comp<WidgetButton>();
                    Entity* e = (Entity*)button.usr;   
                    e->comp<WidgetState>().statusToPropagate = ModelStatus::HIDE;
                    e->comp<WidgetState>().status            = ModelStatus::HIDE;
                }
            break;
        
            case ModelStatus::SHOW :
                if(isIndirectHST)
                {
                    auto &button = child->comp<WidgetButton>();

                    Entity* e = (Entity*)button.usr;
                    Entity* p = e->comp<EntityGroupInfo>().parent;

                    if(p)
                        for(auto c : p->comp<EntityGroupInfo>().children)
                            if(c.get() != e)
                                c->comp<WidgetState>().statusToPropagate = ModelStatus::HIDE;
                    
                    // e->comp<WidgetState>().statusToPropagate = ModelStatus::SHOW;

                    auto &newStatus = e->comp<WidgetState>().status;
                    switch (newStatus)
                    {
                    case ModelStatus::UNDEFINED :
                        e->comp<WidgetState>().statusToPropagate = ModelStatus::UNDEFINED;
                        // newStatus = ModelStatus::SHOW;
                        break;
                    
                    case ModelStatus::HIDE : 
                        e->comp<WidgetState>().statusToPropagate = ModelStatus::HIDE;

                    case ModelStatus::SHOW :
                        e->comp<WidgetState>().statusToPropagate = ModelStatus::SHOW;
                        // e->comp<WidgetState>().statusToPropagate = ModelStatus::UNDEFINED;

                        // std::cout << e->comp<EntityInfos>().name << "\n";
                        break;

                    default:
                        break;
                    }
                }
            break;

            default : break;
        }
    
    }
    
    if(up.statusToPropagate == ModelStatus::UNDEFINED)
        up.statusToPropagate = up.status;

    auto status = up.updateCounter > 2 ? up.status : ModelStatus::HIDE;

    if(child->hasComp<WidgetBackground>() && child->comp<WidgetBackground>().tile.get())
        child->comp<WidgetBackground>().tile->setHideStatus(status);

    if(child->hasComp<WidgetText>() && child->comp<WidgetText>().mesh.get())
        child->comp<WidgetText>().mesh->state.setHideStatus(status);

    if(child->hasComp<WidgetSprite>() && child->comp<WidgetSprite>().sprite.get())
        child->comp<WidgetSprite>().sprite->state.setHideStatus(status);
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
        b.tile->tileType = s.backGroundStyle;
    });

    System<WidgetStyle, WidgetText>([](Entity &entity)
    {
        if(entity.hasComp<WidgetState>() && entity.comp<WidgetState>().status == ModelStatus::HIDE)
            return;

        auto &s = entity.comp<WidgetStyle>();
        auto &t = entity.comp<WidgetText>();

        t.mesh->color = s.useAltTextColor ? s.textColor2 : s.textColor1;
    });

    System<WidgetSprite, WidgetBox>([](Entity &entity)
    {
        if(entity.hasComp<WidgetState>() && entity.comp<WidgetState>().status == ModelStatus::HIDE)
            return;

        // auto &style = entity.comp<WidgetStyle>();
        vec2 position = entity.hasComp<WidgetStyle>() ? entity.comp<WidgetStyle>().spritePosition : vec2(0);

        auto &sprite = entity.comp<WidgetSprite>();
        auto &box = entity.comp<WidgetBox>();
        vec2 scale = vec2(box.displayMax - box.displayMin);

        vec2 spriteScale = scale;

        if(entity.hasComp<WidgetStyle>())
            spriteScale *= entity.comp<WidgetStyle>().spriteScale;

// child->comp<WidgetSprite>().sprite->state.setScale(vec3(scale, 1)).setPositionXY(pos).setPositionZ(box.depth+0.000025);

        sprite.sprite->state.setPositionXY(
            // mix(box.displayMin, box.displayMax, position*0.5f + 0.5f) * vec2(1, -1)

            (box.displayMin + scale*position) * vec2(1, -1)

            // box.displayMin * vec2(1, -1)
        )
            .setPositionZ(box.depth+0.000025)
            .setScale(vec3(spriteScale, 1))
            ;
    });
}

void updateEntityCursor(vec2 screenPos, bool down, bool click, WidgetUI_Context& ui, bool showEntityUnderCursorHint)
{
    screenPos = (screenPos/vec2(globals.windowSize()))*2.f - 1.f;

    if(!cursorHelp)
        cursorHelp = newEntity("cursor button help", ui
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

    static Entity *lastEntityClicked = nullptr;
    if(!down)
        lastEntityClicked = nullptr;

    System<WidgetBox>([screenPos, down, click](Entity &entity){

        auto &box = entity.comp<WidgetBox>();
        if(box.displayMin.x > box.displayMax.x || box.displayMin.y > box.displayMax.y)
        {
            box.isUnderCursor = false;
            WARNING_MESSAGE(
                "Wrong WidgetBox logic for entity '" 
                << entity.comp<EntityInfos>().name 
                << "' display minimal range is greated than display maximal range")
            return;
        }

        vec2 cursor = ((screenPos-box.displayMin)/(box.displayMax - box.displayMin));
        box.isUnderCursor = cursor.x >= 0 && cursor.y >= 0 && cursor.x <= 1 && cursor.y <= 1;

        cursor = ((screenPos-box.childrenMin)/(box.childrenMax - box.childrenMin));
        box.areChildrenUnderCurosor = cursor.x >= 0 && cursor.y >= 0 && cursor.x <= 1 && cursor.y <= 1;
    });

    System<WidgetBox, WidgetButton, WidgetState>([screenPos, down, click](Entity &entity)
    {
        auto &box = entity.comp<WidgetBox>();
        auto &button = entity.comp<WidgetButton>();
        auto &state = entity.comp<WidgetState>();

        auto &style = entity.comp<WidgetStyle>();
        auto &group = entity.comp<EntityGroupInfo>();

        if(button.valueUpdate)
            button.cur = button.valueUpdate(&entity);
        
        if(button.valueUpdate2D)
        {
            vec2 uv = button.valueUpdate2D(&entity);
            button.cur = uv.x;
            button.cur2 = uv.y;
        }

        vec2 cursor = ((screenPos-box.displayMin)/(box.displayMax - box.displayMin));

        
        switch (button.type)
        {
            case WidgetButton::Type::HIDE_SHOW_TRIGGER : 
                if(entity.hasComp<WidgetStyle>())
                    style.useAltBackgroundColor = state.statusToPropagate == ModelStatus::HIDE && group.children.size();
                break;

            case WidgetButton::Type::HIDE_SHOW_TRIGGER_INDIRECT : 
                if(entity.hasComp<WidgetStyle>())
                    style.useAltTextColor = style.useAltBackgroundColor = state.statusToPropagate == ModelStatus::HIDE;
                break;

            case WidgetButton::Type::CHECKBOX :
                if(entity.hasComp<WidgetStyle>())
                    style.useAltBackgroundColor = style.useAltTextColor = button.cur > 0.f;
                break;
            
            case WidgetButton::Type::SLIDER :
                if(lastEntityClicked == &entity)
                {
                    float v = clamp(cursor.x, 0.f, 1.f);
                    v = round(v*button.padding)/button.padding;
                    button.cur = button.min + v*(button.max - button.min);
                    
                    button.valueChanged(&entity, button.cur);

                }
                if(entity.hasComp<WidgetSprite>() && entity.hasComp<WidgetStyle>())
                {
                    float v = button.cur/(button.max-button.min) - button.min;
                    auto &style = entity.comp<WidgetStyle>();
                    style.setspritePosition(vec2(v - 0.5 + style.spriteScale*0.5f*button.min, 0));
                }
                break;
            
            case WidgetButton::Type::SLIDER_2D :
                if(lastEntityClicked == &entity)
                {
                    vec2 v = clamp(cursor, vec2(0.f), vec2(1.f));

                    v = round(v*button.padding)/button.padding;

                    button.cur = button.min + v.x*(button.max - button.min);
                    button.cur2 = button.min + v.y*(button.max - button.min);
                    
                    button.valueChanged2D(&entity, vec2(button.cur, button.cur2));
                }
                if(entity.hasComp<WidgetSprite>() && entity.hasComp<WidgetStyle>())
                {
                    vec2 v = vec2(button.cur, button.cur2)/(button.max-button.min) - button.min;
                    auto &style = entity.comp<WidgetStyle>();
                    style.setspritePosition(v - 0.5f + style.spriteScale*0.5f*button.min);
                }
            break;

            case WidgetButton::Type::TEXT_INPUT :
                // if(globals.canUseTextInputs(&entity))
                if(entity.hasComp<WidgetText>() && globals.canUseTextInputs(&entity))
                {
                    auto &text = entity.comp<WidgetText>().text;

                    globals.getTextInputs(&entity, text);
                    if(text.back() == U'\n')
                    {
                        globals.endTextInputs(&entity);
                        text.pop_back();

                        if(button.valueChanged)
                            button.valueChanged(&entity, button.cur);
                    }
                    if(click && (cursor.x < 0 || cursor.y < 0 || cursor.x > 1 || cursor.y > 1))
                    {
                        globals.endTextInputs(&entity);

                        if(button.valueChanged)
                            button.valueChanged(&entity, button.cur);
                    }
                }

                if(entity.hasComp<WidgetStyle>())
                {
                    style.useAltBackgroundColor = style.useAltTextColor = globals.canUseTextInputs(&entity);
                }

                break;

            default : break; 
        }

        if(state.status == ModelStatus::HIDE)
            return;

        if(!box.isUnderCursor)
            return;

        if(click)
            lastEntityClicked = &entity;

        entityUnderCursor = &entity;

        switch (button.type)
        {
            case WidgetButton::Type::HIDE_SHOW_TRIGGER :
            case WidgetButton::Type::HIDE_SHOW_TRIGGER_INDIRECT :
                if(click)
                {
                    switch (state.statusToPropagate)
                    {
                        case ModelStatus::HIDE :
                            state.statusToPropagate = ModelStatus::SHOW;
                            if(button.type == WidgetButton::Type::HIDE_SHOW_TRIGGER_INDIRECT)
                            {
                                auto usr = ((Entity*)button.usr);
                                usr->comp<WidgetBox>().displayMin = vec2(UNINITIALIZED_FLOAT);
                                usr->comp<WidgetBox>().displayMax = vec2(UNINITIALIZED_FLOAT);

                                Entity* p = entity.comp<EntityGroupInfo>().parent;
                                if(p)
                                    for(auto c : p->comp<EntityGroupInfo>().children)
                                        if(c.get() != &entity)
                                        {
                                            c->comp<WidgetState>().statusToPropagate = ModelStatus::HIDE;
                                        }
                            }
                            break;

                        case ModelStatus::SHOW :
                        case ModelStatus::UNDEFINED :
                            state.statusToPropagate = ModelStatus::HIDE;
                            break;

                        default: break;
                    }
                }
                break;

            case WidgetButton::Type::CHECKBOX :
                if(click)
                {
                    button.cur = button.cur == 0.f ? 1.f : 0.f;
                    button.valueChanged(&entity, button.cur);
                }
                break;

            case WidgetButton::Type::TEXT_INPUT :
                if(entity.hasComp<WidgetText>() && click)
                {
                    globals.useTextInputs(&entity);
                }
                break;

            default:
                break;
        }

    });

    if(lastEntityUnderCursor != entityUnderCursor)
        lastTimeChangeEUC = globals.appTime.getElapsedTime();

    if(entityUnderCursor && showEntityUnderCursorHint)
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
