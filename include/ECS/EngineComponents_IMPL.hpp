
#pragma once

#ifndef VULPINE_COMPONENT_IMPL
#include <ECS/EngineComponents.hpp>
#endif

#include <Graphics/Scene.hpp>

template<> void Component<WidgetModel>::ComponentElem::init()
{
    if(data.model.get())
        data.model = newObjectGroup();

    data.scene->add(data.model);
};

template<> void Component<WidgetModel>::ComponentElem::clean()
{
    if(data.model.get())
        data.scene->remove(data.model);
    else
        WARNING_MESSAGE("Trying to clean null component from entity " << entity->ids[ENTITY_LIST] << " named " << entity->comp<EntityInfos>().name)
};

template<> void Component<WidgetBackground>::ComponentElem::init()
{
    FastUI_context *ui = entity->comp<FastUI_context*>();

    data.tile = SimpleUiTileRef(new SimpleUiTile(
        ModelState3D()
        , UiTileType::SQUARE_ROUNDED
        , ui->colorTitleBackground));

    ui->tileBatch->add(data.tile);
}

template<> void Component<WidgetText>::ComponentElem::init()
{
    FastUI_context *ui = entity->comp<FastUI_context*>();

    data.mesh = SingleStringBatchRef(new SingleStringBatch);
    data.mesh->setMaterial(ui->fontMaterial);
    data.mesh->setFont(ui->font);
    data.mesh->color = ui->colorFont;
    data.mesh->uniforms.add(ShaderUniform(&data.mesh->color, 32));

    if(!data.text.size()) 
        data.text = UFTconvert.from_bytes(entity->comp<EntityInfos>().name);

    ui->scene.add(data.mesh);
}

COMPONENT_DEFINE_SYNCH(WidgetBox)
{
    auto &box = child->comp<WidgetBox>();

    bool hidden = child->hasComp<WidgetState>() && child->comp<WidgetState>().status == ModelStateHideStatus::HIDE;

    if(child->hasComp<WidgetState>() && child->comp<WidgetState>().status == ModelStateHideStatus::HIDE)
        return;

    vec2 tmpMin = box.min;
    vec2 tmpMax = box.max;

    /********** MODYFING BOX VALUES **********/
    if(&parent != child.get())
    {
        auto &parentBox = parent.comp<WidgetBox>();

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
    }

    // if(&parent != child.get())
    //     std::cout << "parent - parent " << box.wasUninitializedLastSynch << "\n";
    // else
    //     std::cout << "parent - child  " << box.wasUninitializedLastSynch << "\n";

    /********** APPLYING SMOOTHING ON DISPLAY **********/
    
    float time = globals.appTime.getElapsedTime();

    if(box.displayMax.x == UNINITIALIZED_FLOAT)
    {
        box.displayMin = mix(box.min, box.max, 0.25f) - (box.min + box.max)*0.5f;
        box.displayMax = mix(box.max, box.min, 0.25f) - (box.min + box.max)*0.5f;
        box.lastChangeTime = time;
    }

    if(box.min != tmpMin || box.max != tmpMax)
        box.lastChangeTime = time;
    
    float a = smoothstep(0.0f, 1.f, (time - box.lastChangeTime));

    // if(a < 1e-9) a = 0.1;

    box.displayMin = mix(box.displayMin, box.min, a);
    box.displayMax = mix(box.displayMax, box.max, a);

    /*
        TODO : move this into a system
    */
    /********** APPLYING DISPLAY VALUES ON ALL RELATED COMPONENTS **********/


    vec2 scale(box.displayMax - box.displayMin);
    vec2 pos(box.displayMin.x, -box.displayMin.y);

    scale = max(scale, vec2(0));

    if(child->hasComp<WidgetBackground>())
    {
        child->comp<WidgetBackground>().tile->setScale(vec3(scale, 1)).setPosition(vec3(pos, 0));
    }

    float iaspectRatio = (float)(globals.windowWidth())/(float)(globals.windowHeight());

    if(child->hasComp<WidgetText>())
    {
        auto &text = child->comp<WidgetText>();
        text.mesh->text = text.text;
        text.mesh->batchText();

        vec2 tsize = text.mesh->getSize();

        tsize.y *= iaspectRatio;

        vec2 tscale = (scale*0.95f/tsize);

        tscale = vec2(min(tscale.x, tscale.y));
        tscale.y *= iaspectRatio;

        text.mesh->state 
            .setPosition(vec3(
                pos 
                + vec2(0.5f, -0.5f)*(scale - text.mesh->getSize()*tscale), 0))
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
            case ModelStateHideStatus::HIDE :
            // case ModelStateHideStatus::UNDEFINED :
                up.status = ModelStateHideStatus::HIDE;
                up.statusToPropagate = up.status;
                break;
            
            case ModelStateHideStatus::SHOW :
                if(child->hasComp<WidgetBox>() && up.status == ModelStateHideStatus::HIDE)
                {
                    auto &b = child->comp<WidgetBox>();
                    b.displayMin = b.displayMax = vec2(UNINITIALIZED_FLOAT);
                }

                up.status = ModelStateHideStatus::SHOW;
                break;

            default: break;
        }
    }

    if(up.statusToPropagate == ModelStateHideStatus::UNDEFINED)
        up.statusToPropagate = up.status;

    if(child->hasComp<WidgetBackground>() && child->comp<WidgetBackground>().tile.get())
        child->comp<WidgetBackground>().tile->setHideStatus(up.status);
    
    if(child->hasComp<WidgetText>() && child->comp<WidgetText>().mesh.get())
        child->comp<WidgetText>().mesh->state.setHideStatus(up.status);
}

void updateEntityCursor(vec2 screenPos, bool down, bool click)
{
    screenPos = (screenPos/vec2(globals.windowSize()))*2.f - 1.f;


    System<WidgetBox, WidgetButton, WidgetState>([screenPos, down, click](Entity &entity)
    {
        auto &box = entity.comp<WidgetBox>();
        auto &button = entity.comp<WidgetButton>();
        auto &state = entity.comp<WidgetState>();

        if(state.status == ModelStateHideStatus::HIDE)
            return;


        vec2 cursor = ((screenPos-box.min)/(box.max - box.min));


        if(cursor.x < 0 || cursor.y < 0 || cursor.x > 1 || cursor.y > 1)
            return;

        switch (button.type)
        {
            case WidgetButton::Type::HIDE_SHOW_TRIGGER :
                if(click)
                {
                    switch (state.statusToPropagate)
                    {
                        case ModelStateHideStatus::HIDE :
                            state.statusToPropagate = ModelStateHideStatus::SHOW;
                            break;

                        case ModelStateHideStatus::SHOW :
                        case ModelStateHideStatus::UNDEFINED :
                            state.statusToPropagate = ModelStateHideStatus::HIDE;
                            break;

                        default: break;
                    }


                }
                break;
            
            default:
                break;
        }

    });
}
