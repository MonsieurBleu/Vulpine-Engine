#include <Scripting/LuaBindings.hpp>
#include <lua.h>
#include <sol/variadic_args.hpp>

// MARK: Bind all
void VulpineLuaBindings::bindAll(sol::state& lua)
{
    glm(lua);
    VulpineTypes(lua);
    Entities(lua);
    Utils(lua);
}

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
using namespace glm;
#include<MathsUtils.hpp>

// MARK: GLM Bindings
void VulpineLuaBindings::glm(sol::state &lua)
{

    #define GENERATE_MUL_OPERATOR_OVERLOAD_OP(ov) \
        auto vec2_##ov = sol::overload(OVERLOAD_OP(vec2, vec2), OVERLOAD_OP_ALL(vec2, float), OVERLOAD_OP_ALL(vec2, mat2)); \
        auto vec3_##ov = sol::overload(OVERLOAD_OP(vec3, vec3), OVERLOAD_OP_ALL(vec3, float), OVERLOAD_OP_ALL(vec3, mat3)); \
        auto vec4_##ov = sol::overload(OVERLOAD_OP(vec4, vec4), OVERLOAD_OP_ALL(vec4, float), OVERLOAD_OP_ALL(vec4, mat3),  OVERLOAD_OP_ALL(vec4, mat4)); \
        auto mat2_##ov = sol::overload(OVERLOAD_OP(mat2, mat2), OVERLOAD_OP_ALL(mat2, float), OVERLOAD_OP_ALL(vec2, mat2)); \
        auto mat3_##ov = sol::overload(OVERLOAD_OP(mat3, mat3), OVERLOAD_OP_ALL(mat3, float), OVERLOAD_OP_ALL(vec3, mat3)); \
        auto mat4_##ov = sol::overload(OVERLOAD_OP(mat4, mat4), OVERLOAD_OP_ALL(mat4, float), OVERLOAD_OP_ALL(vec4, mat4)); 

    #define GENERATE_ADD_OPERATOR_OVERLOAD_OP(ov) \
        auto vec2_##ov = sol::overload(OVERLOAD_OP(vec2, vec2)); \
        auto vec3_##ov = sol::overload(OVERLOAD_OP(vec3, vec3)); \
        auto vec4_##ov = sol::overload(OVERLOAD_OP(vec4, vec4)); \
        auto mat2_##ov = sol::overload(OVERLOAD_OP(mat2, mat2)); \
        auto mat3_##ov = sol::overload(OVERLOAD_OP(mat3, mat3)); \
        auto mat4_##ov = sol::overload(OVERLOAD_OP(mat4, mat4)); \
        auto quat_##ov = sol::overload(OVERLOAD_OP(quat, quat));

    #undef VLB_GLM_CUR_OPERATOR
    #define VLB_GLM_CUR_OPERATOR *
    GENERATE_MUL_OPERATOR_OVERLOAD_OP(mul)

    #undef VLB_GLM_CUR_OPERATOR
    #define VLB_GLM_CUR_OPERATOR /
    GENERATE_MUL_OPERATOR_OVERLOAD_OP(div)

    #undef VLB_GLM_CUR_OPERATOR
    #define VLB_GLM_CUR_OPERATOR +
    GENERATE_ADD_OPERATOR_OVERLOAD_OP(add)

    #undef VLB_GLM_CUR_OPERATOR
    #define VLB_GLM_CUR_OPERATOR -
    GENERATE_ADD_OPERATOR_OVERLOAD_OP(sub)


    #define SET_OVERLOAD_OPS(type) \
        sol::meta_function::multiplication, type##_mul, \
        sol::meta_function::division,       type##_div, \
        sol::meta_function::addition,       type##_add, \
        sol::meta_function::subtraction,    type##_sub,  \
        sol::meta_function::index, [](type & m, const int i){return m[i];}
// MARK: GLM Types
    /*** Setting up glm vector type with operator bindings ***/
    lua.new_usertype<vec2>("vec2", 
        sol::call_constructor, sol::constructors<vec2(), vec2(float), vec2(float, float)>(),
        "x", &vec2::x, "r", &vec2::r,
        "y", &vec2::y, "g", &vec2::g,
        SET_OVERLOAD_OPS(vec2)
    );

    lua.new_usertype<vec3>("vec3", 
        sol::call_constructor, sol::constructors<vec3(), vec3(float), vec3(float, float, float)>(),
        "x", &vec3::x, "r", &vec3::r,
        "y", &vec3::y, "g", &vec3::g,
        "z", &vec3::z, "b", &vec3::b,
        SET_OVERLOAD_OPS(vec3)
    );

    lua.new_usertype<vec4>("vec4", 
        sol::call_constructor, sol::constructors<vec4(), vec4(float), vec4(float, float, float, float)>(),
        "x", &vec4::x, "r", &vec4::r,
        "y", &vec4::y, "g", &vec4::g,
        "z", &vec4::z, "b", &vec4::b,
        "w", &vec4::w, "a", &vec4::a,
        SET_OVERLOAD_OPS(vec4)
    );

    /*** Setting up glm matrices type with operator bindings ***/
        lua.new_usertype<mat2>("mat2",
            sol::call_constructor, sol::constructors<mat2(), mat2(float), mat2(float, float, float, float)>(),
            SET_OVERLOAD_OPS(mat2)
        );

        lua.new_usertype<mat3>("mat3",
            sol::call_constructor, sol::constructors<mat3(), mat3(quat), mat3(float), mat3(float, float, float, float, float, float, float, float, float)>(),
            SET_OVERLOAD_OPS(mat3)
        );

        lua.new_usertype<mat4>("mat4",
            sol::call_constructor, sol::constructors<mat4(), mat4(quat), mat4(float), mat4(float, float, float, float, float, float, float, float, float, float, float, float, float, float, float, float)>(),
            SET_OVERLOAD_OPS(mat4)
        );

        lua.new_usertype<quat>("quat",
            sol::call_constructor, sol::constructors<quat(), quat(float, float, float, float), quat(vec3), quat(mat3), quat(mat4), quat(quat), quat(vec3, vec3), quat(float, vec3)>(),
            sol::meta_function::multiplication, sol::overload(
                [](const quat &v1, const quat &v2){return v1*v2;},
                [](const quat &v1, const vec3 &v2){return v1*v2;}
            ),
            sol::meta_function::addition, quat_add,
            sol::meta_function::subtraction, quat_sub,
            "x", &quat::x,
            "y", &quat::y,
            "z", &quat::z,
            "w", &quat::w
        );

// MARK: GLM Funcs
    /*** Setting up glm functions bindings ***/
    lua.set_function("mix",
        sol::overload(
            LAMBDA_BIND_3(mix, const float, const float, const float),
            LAMBDA_BIND_3(mix, const vec2,  const vec2,  const float),
            LAMBDA_BIND_3(mix, const vec3,  const vec3,  const float),
            LAMBDA_BIND_3(mix, const vec4,  const vec4,  const float),
            LAMBDA_BIND_3(mix, const vec2,  const vec2,  const vec2),
            LAMBDA_BIND_3(mix, const vec3,  const vec3,  const vec3),
            LAMBDA_BIND_3(mix, const vec4,  const vec4,  const vec4)
        )
    );

    lua.set_function("smoothstep",
        sol::overload(
            LAMBDA_BIND_3(smoothstep, const float, const float, const float),
            LAMBDA_BIND_3(smoothstep, const vec2,  const vec2,  const vec2),
            LAMBDA_BIND_3(smoothstep, const vec3,  const vec3,  const vec3),
            LAMBDA_BIND_3(smoothstep, const vec4,  const vec4,  const vec4)
        )
    );

    lua.set_function("dot", 
        sol::overload(
            LAMBDA_BIND_2(dot, vec2, vec2),
            LAMBDA_BIND_2(dot, vec3, vec3),
            LAMBDA_BIND_2(dot, vec4, vec4)
        )
    );

    lua.set_function("cross", 
        sol::overload(
            LAMBDA_BIND_2(cross, vec3, vec3)
        )
    );

    lua.set_function("normalize", 
        sol::overload(
            LAMBDA_BIND_1(normalize, vec2),
            LAMBDA_BIND_1(normalize, vec3),
            LAMBDA_BIND_1(normalize, vec4)
        )
    );

    lua.set_function("inverse", 
        sol::overload(
            LAMBDA_BIND_1(inverse, quat),
            LAMBDA_BIND_1(inverse, mat2),
            LAMBDA_BIND_1(inverse, mat3),
            LAMBDA_BIND_1(inverse, mat4)
        )
    );

// MARK: Vulpine Math
    /*** Setting up vulpine math utils functions bindings ***/
    lua.set_function("hsv2rgb",LAMBDA_BIND_1(hsv2rgb, vec3));
    lua.set_function("rgb2hsv", LAMBDA_BIND_1(rgb2hsv, vec3));
    lua.set_function("ColorHexToV",LAMBDA_BIND_1_CPY(ColorHexToV, uint));

    lua.set_function("slerpDirClamp", LAMBDA_BIND_4_CPY(slerpDirClamp, vec3, vec3, float, vec3));
    lua.set_function("PhiThetaToDir", LAMBDA_BIND_1_CPY(PhiThetaToDir, vec2));
    lua.set_function("getPhiTheta", LAMBDA_BIND_1_CPY(getPhiTheta, vec3));
    lua.set_function("angle", LAMBDA_BIND_2_CPY(angle, vec2, vec2));
    lua.set_function("directionToQuat", LAMBDA_BIND_1_CPY(directionToQuat, vec3));
    lua.set_function("directionToEuler", LAMBDA_BIND_1_CPY(directionToEuler, vec3));

    lua.set_function("projectPointOntoPlane", LAMBDA_BIND_3_CPY(projectPointOntoPlane, vec3, vec3, vec3));
    lua.set_function("rayAlignedPlaneIntersect", LAMBDA_BIND_4_CPY(rayAlignedPlaneIntersect, vec3, vec3, float, float));
 
    lua.set_function("angle", LAMBDA_BIND_1(angle, quat));
    lua.set_function("axis", LAMBDA_BIND_1(axis, quat));
    lua.set_function("angleAxis", LAMBDA_BIND_2(angleAxis, const float, const vec3));
    lua.set_function("radians", LAMBDA_BIND_1(radians, const float));
}

#include <Timer.hpp>
#include <Matrix.hpp>
#include <Utils.hpp>
void VulpineLuaBindings::VulpineTypes(sol::state &lua)
{
    // MARK: Bench Timer
    {
        #undef CURRENT_CLASS_BINDING
        #define CURRENT_CLASS_BINDING BenchTimer

        CREATE_CLASS_USERTYPE(BenchTimer, (), (std::string))
        ADD_MEMBER_BINDINGS(
            stop, 
            hold, 
            start, 
            toggle, 
            resume, 
            pause, 
            isPaused, 
            setAvgLengthMS, 
            getDelta, 
            getElapsedTime,
            getElapsedTimeAddr,
            getUpdateCounter,
            getLastAvg,
            getMax,
            reset
        )
    }
    // MARK: ModelState3D
    {
        #undef CURRENT_CLASS_BINDING
        #define CURRENT_CLASS_BINDING ModelState3D

        CREATE_CLASS_USERTYPE(ModelState3D, (), ())
        ADD_MEMBER_BINDINGS(
            position,
            scale,
            rotation,
            lookAtPoint,
            rotationMatrix,
            modelMatrix,
            frustumCulled,
            hide,
            quaternion,
            useQuaternion,
            setHideStatus,
            scaleScalar,
            setScale,
            setPosition,
            setPositionXY,
            setPositionZ,
            setQuaternion,
            setRotation,
            lookAt,
            update,
            forceUpdate,
            needUpdate
        );

        lua.new_enum(
            "ModelStatus",
            "HIDE", ModelStatus::HIDE,
            "SHOW", ModelStatus::SHOW,
            "UNDEFINED", ModelStatus::UNDEFINED
        );
    }

    // MARK: Utils
    lua.set_function("STR_CASE_STR", [](const std::string &a, const std::string &b){return STR_CASE_STR(a.c_str(), b.c_str()) != nullptr;});
        
    lua.set_function("replace", 
        [](std::string a, const std::string &b, const std::string &c)
        {
            replace(a, b, c);
            return a;
        }
    );
}

#include <ECS/EngineComponents.hpp>
#include "AssetManager.hpp"
#include "VulpineParser.hpp"
#include "Utils.hpp"
#include <Graphics/FastUI.hpp>
#include <Graphics/Scene.hpp>

void VulpineLuaBindings::Entities(sol::state &lua)
{
    // MARK: Entity
    {
        #undef CURRENT_CLASS_BINDING
        #define CURRENT_CLASS_BINDING Entity
        CREATE_CLASS_USERTYPE(Entity, (), ())

        // METHOD_BINDING_TEMPLATED_SINGLE(comp, EntityInfos)
        // METHOD_BINDING_TEMPLATED_SINGLE(comp, WidgetBox)
        METHOD_BINDING_TEMPLATED(
            comp, 
                EntityInfos,
                EntityGroupInfo,
                WidgetBackground,
                WidgetBox,
                WidgetButton,
                WidgetSprite,
                WidgetState,
                WidgetStyle,
                WidgetText,
        )

        METHOD_BINDING_TEMPLATED(
            hasComp, 
                EntityInfos,
                EntityGroupInfo,
                WidgetBackground,
                WidgetBox,
                WidgetButton,
                WidgetSprite,
                WidgetState,
                WidgetStyle,
                WidgetText,
        )

        METHOD_BINDING_TEMPLATED(
            set, 
                EntityInfos,
                EntityGroupInfo,
                WidgetBackground,
                WidgetBox,
                WidgetButton,
                WidgetSprite,
                WidgetState,
                WidgetStyle,
                WidgetText,
        )

        METHOD_BINDING_TEMPLATED(
            removeComp, 
                EntityInfos,
                EntityGroupInfo,
                WidgetBackground,
                WidgetBox,
                WidgetButton,
                WidgetSprite,
                WidgetState,
                WidgetStyle,
                WidgetText,
        )

        ADD_MEMBER_BINDINGS(
            toStr,
        );
    }
    // MARK: Components
    {
        #undef CURRENT_CLASS_BINDING
        #define CURRENT_CLASS_BINDING EntityInfos
        CREATE_CLASS_USERTYPE(EntityInfos, (), ())
        lua.new_usertype<EntityInfos>(
            "EntityInfos",
            "name", &EntityInfos::name
        );
    }
        {
        #undef CURRENT_CLASS_BINDING
        #define CURRENT_CLASS_BINDING EntityGroupInfo
        CREATE_CLASS_USERTYPE(EntityGroupInfo, (), (std::vector<EntityRef> &))
        ADD_MEMBER_BINDINGS(
            children,
            // markedForDeletion, // TODO: check if we even want to expose those two
            // markedForCreation,
            parent
        );
    }
    {
        #undef CURRENT_CLASS_BINDING
        #define CURRENT_CLASS_BINDING WidgetBox
        CREATE_CLASS_USERTYPE(WidgetBox, (vec2, vec2, WidgetBox::Type), (WidgetBox::FittingFunc))
        ADD_MEMBER_BINDINGS(
            min,
            max,
            initMin,
            initMax,
            childrenMin,
            childrenMax,
            displayMin,
            displayMax,
            lastMin,
            lastMax,
            displayRangeMin,
            displayRangeMax,
            depth,
            lastChangeTime,
            synchCounter,
            useClassicInterpolation,
            isUnderCursor,
            areChildrenUnderCurosor,
            scrollOffset,
            type
        );

        lua.new_enum(
            "WidgetBoxType",
            "FOLLOW_PARENT_BOX", WidgetBox::Type::FOLLOW_PARENT_BOX,
            "FOLLOW_SIBLINGS_BOX", WidgetBox::Type::FOLLOW_SIBLINGS_BOX
        );
    }
    {
        #undef CURRENT_CLASS_BINDING
        #define CURRENT_CLASS_BINDING WidgetBackground
        CREATE_CLASS_USERTYPE(WidgetBackground, (), ())
        ADD_MEMBER_BINDINGS(
            tile,
            batch
        );
    }
    {
        #undef CURRENT_CLASS_BINDING
        #define CURRENT_CLASS_BINDING WidgetButton
        CREATE_CLASS_USERTYPE(
            WidgetButton, 
            (WidgetButton::Type), ()
        );
        ADD_MEMBER_BINDINGS(
            type,
            cur,
            setcur,
            cur2,
            setcur2,
            min,
            setmin,
            max,
            setmax,
            padding,
            setpadding,
            usr,
            setusr
            // valueChanged,
            // valueUpdate,
            // valueChanged2D,
            // valueUpdate2D,
            // material
        );

        lua.new_enum(
            "WidgetButtonType",
            "HIDE_SHOW_TRIGGER", WidgetButton::HIDE_SHOW_TRIGGER,
            "HIDE_SHOW_TRIGGER_INDIRECT", WidgetButton::HIDE_SHOW_TRIGGER_INDIRECT,
            "CHECKBOX", WidgetButton::CHECKBOX,
            "TEXT_INPUT", WidgetButton::TEXT_INPUT,
            "SLIDER", WidgetButton::SLIDER,
            "SLIDER_2D", WidgetButton::SLIDER_2D
        );
    }
    {
        #undef CURRENT_CLASS_BINDING
        #define CURRENT_CLASS_BINDING WidgetSprite
        CREATE_CLASS_USERTYPE(WidgetSprite, (), (std::string&), (ModelRef))
        ADD_MEMBER_BINDINGS(
            sprite,
            name,
            scene
        );
    }
    {
        #undef CURRENT_CLASS_BINDING
        #define CURRENT_CLASS_BINDING WidgetState
        CREATE_CLASS_USERTYPE(WidgetState, (), ())
        ADD_MEMBER_BINDINGS(
            upToDate,
            status,
            statusToPropagate,
            updateCounter
        );
    }
    {
        #undef CURRENT_CLASS_BINDING
        #define CURRENT_CLASS_BINDING WidgetStyle
        CREATE_CLASS_USERTYPE(WidgetStyle, (), ())
        ADD_MEMBER_BINDINGS(
            textColor1,
            settextColor1,
            textColor2,
            settextColor2,
            backgroundColor1,
            setbackgroundColor1,
            backgroundColor2,
            setbackgroundColor2,
            backGroundStyle,
            setbackGroundStyle,
            automaticTabbing,
            setautomaticTabbing,
            spriteScale,
            setspriteScale,
            useInternalSpacing,
            setuseInternalSpacing,
            spritePosition,
            setspritePosition,
            useAltBackgroundColor,
            useAltTextColor
        );

        lua.new_enum(
            "UiTileType",
            "SQUARE", UiTileType::SQUARE,
            "SQUARE_ROUNDED", UiTileType::SQUARE_ROUNDED,
            "CIRCLE", UiTileType::CIRCLE,
            "SATURATION_VALUE_PICKER", UiTileType::SATURATION_VALUE_PICKER,
            "HUE_PICKER", UiTileType::HUE_PICKER,
            "ATMOSPHERE_VIEWER", UiTileType::ATMOSPHERE_VIEWER
        );
    }
    {
        #undef CURRENT_CLASS_BINDING
        #define CURRENT_CLASS_BINDING WidgetText
        CREATE_CLASS_USERTYPE(
            WidgetText, 
            (), 
            (std::u32string, StringAlignment)
        );
        ADD_MEMBER_BINDINGS(
            text,
            align
        );

        lua.new_enum(
            "StringAlignment",
            "TO_LEFT", StringAlignment::TO_LEFT,
            "CENTERED", StringAlignment::CENTERED
        );
    }
// MARK: Entity file IO
    {
        lua.set_function(
            "entityWriteToFile",
            [](const Entity& entity, const char* filename)
            {
                // TODO: improve this with asset checking
                VulpineTextOutputRef out(new VulpineTextOutput());
                // std::cout << "Writing entity to " << filename << "\n";
                EntityRef e = std::make_shared<Entity>(entity);
                DataLoader<EntityRef>::write(e, out);
                out->saveAs(filename);
            }
        );

        lua.set_function(
            "entityReadFromFile",
            [](const char* asset_name, Entity& parent) -> Entity&
            {
                auto info = Loader<EntityRef>::loadingInfos.find(asset_name);
                if (info == Loader<EntityRef>::loadingInfos.end()) {
                    FILE_ERROR_MESSAGE(asset_name, "Entity not found in asset manager");
                    throw std::runtime_error(std::string("Could not find asset with name ") + asset_name);
                }

                std::string filename = info->second->buff->getSource();
                VulpineTextBuffRef in(new VulpineTextBuff(filename.c_str()));
                EntityRef e = DataLoader<EntityRef>::read(in);

                if(!e)
                    throw std::runtime_error("Could not read entity from file");
                
                ComponentModularity::addChild(parent, e);
                return *e;
            }
        );
    }
    // MARK: Modularity
    {
        sol::table componentModularityTable = lua.create_table();
        componentModularityTable.set_function(
            "addChild",
            [](Entity &parent, Entity& child)
            {
                ComponentModularity::addChild(parent, std::make_shared<Entity>(child));
            }
        );

        componentModularityTable.set_function(
            "removeChild",
            [](Entity &parent, Entity& child)
            {
                ComponentModularity::removeChild(parent, &child);
            }
        );

        componentModularityTable.set_function(
            "synchronizeChildren",
            [](Entity& parent)
            {
                ComponentModularity::synchronizeChildren(std::make_shared<Entity>(parent));
            }
        );

        componentModularityTable.set_function(
            "reparent",
            [](Entity& oldParent, Entity& child, Entity& newParent)
            {
                ComponentModularity::Reparent(oldParent, std::make_shared<Entity>(child), newParent);
            }
        );

        componentModularityTable.set_function(
            "reparentChildren",
            [](Entity& parent)
            {
                ComponentModularity::ReparentChildren(parent);
            }
        );

        componentModularityTable.set_function(
            "canMerge",
            [](Entity& parent, Entity& child) -> bool
            {
                return ComponentModularity::canMerge(parent, std::make_shared<Entity>(child));
            }
        );

        componentModularityTable.set_function(
            "mergeChild",
            [](Entity& parent, Entity& child) -> bool
            {
                if(ComponentModularity::canMerge(parent, std::make_shared<Entity>(child)))
                {
                    for(auto &i : ComponentModularity::MergeFuncs)
                        if(parent.state[i.ComponentID] && child.state[i.ComponentID])
                            i.element(parent, std::make_shared<Entity>(child));
                    
                    ComponentModularity::removeChild(parent, &child);
                    return true;
                }
                return false;
            }
        );

        componentModularityTable.set_function(
            "mergeChildren",
            [](Entity& parent)
            {
                ComponentModularity::mergeChildren(parent);
            }
        );
        lua["ComponentModularity"] = componentModularityTable;
    }
}

// MARK: Utils
void VulpineLuaBindings::Utils(sol::state &lua)
{
    {
        lua.set_function(
            "print",
            [&](sol::variadic_args args)
            {
                lua_Debug ar;
                lua_getstack(lua, 1, &ar);
                lua_getinfo(lua, "Sl", &ar);
                std::string file = ar.short_src;
                int line = ar.currentline;
                // TODO: either add a define or remove the `Sanctia-Release/` for release builds
                std::cout << "[" << "Sanctia-Release/" << file << ":" << line << "] ";
                for(auto arg : args)
                {
                    std::cout << lua["tostring"](arg.get<sol::object>()).get<std::string>() << " ";
                }
                std::cout << std::endl;
            }
        );
    }
}