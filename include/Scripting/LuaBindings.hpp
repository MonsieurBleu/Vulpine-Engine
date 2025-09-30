#pragma once


#define SOL_LUAJIT 1
#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

namespace VulpineLuaBindings
{
    void bindAll(sol::state& lua);

    void glm(sol::state& lua);

    void VulpineTypes(sol::state& lua);

    // static void globals(sol::state& lua);

    // static void controls(sol::state& lua);

    // static void states(sol::state& lua);

    static void Entities(sol::state& lua);
}

#define OVERLOAD_OP(type1, type2)[](const type1 &v1, const type2 &v2){return v1 VLB_GLM_CUR_OPERATOR v2;}
#define OVERLOAD_OP_ALL(type1, type2) OVERLOAD_OP(type1, type2), OVERLOAD_OP(type2, type1)

#define LAMBDA_BIND_0(f) [](){return f();}
#define LAMBDA_BIND_1(f, t1) [](t1 &v1){return f(v1);}
#define LAMBDA_BIND_2(f, t1, t2) [](t1 &v1, t2 &v2){return f(v1, v2);}
#define LAMBDA_BIND_3(f, t1, t2, t3) [](t1 &v1, t2 &v2, t3 &v3){return f(v1, v2, v3);}
#define LAMBDA_BIND_4(f, t1, t2, t3, t4) [](t1 &v1, t2 &v2, t3 &v3, t4 &v4){return f(v1, v2, v3, v4);}

#define LAMBDA_BIND_1_CPY(f, t1) [](t1 v1){return f(v1);}
#define LAMBDA_BIND_2_CPY(f, t1, t2) [](t1 v1, t2 v2){return f(v1, v2);}
#define LAMBDA_BIND_3_CPY(f, t1, t2, t3) [](t1 v1, t2 v2, t3 v3){return f(v1, v2, v3);}
#define LAMBDA_BIND_4_CPY(f, t1, t2, t3, t4) [](t1 v1, t2 v2, t3 v3, t4 v4){return f(v1, v2, v3, v4);}

// #define LAMBDA_BIND(f, ...) [](){return f();}

#ifdef VLB_ALL_IMPL
    #ifndef USE_CUSTOM_LUA_BINDINGS

        void VulpineLuaBindings::bindAll(sol::state& lua)
        {
            glm(lua);
            VulpineTypes(lua);
            Entities(lua);
        }

    #endif
#endif 


#ifdef VLB_GLM_IMPL
    
    #include <glm/glm.hpp>
    #include <glm/gtc/quaternion.hpp>
    #define GLM_ENABLE_EXPERIMENTAL
    #include <glm/gtx/string_cast.hpp>
    using namespace glm;
    #include<MathsUtils.hpp>
    
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
            sol::call_constructor, sol::constructors<mat3(), mat3(float), mat3(float, float, float, float, float, float, float, float, float)>(),
            SET_OVERLOAD_OPS(mat3)
        );

        lua.new_usertype<mat4>("mat4",
            sol::call_constructor, sol::constructors<mat4(), mat4(float), mat4(float, float, float, float, float, float, float, float, float, float, float, float, float, float, float, float)>(),
            SET_OVERLOAD_OPS(mat4)
        );

        lua.new_usertype<quat>("quat",
            sol::call_constructor, sol::constructors<quat(), quat(float, float, float, float), quat(vec3), quat(mat3), quat(mat4), quat(quat), quat(vec3, vec3), quat(float, vec3)>(),
            sol::meta_function::multiplication, sol::overload(
                [](const quat &v1, const quat &v2){return v1*v2;},
                [](const quat &v1, const vec3 &v2){return v1*v2;}
            ),
            sol::meta_function::addition, quat_add,
            sol::meta_function::subtraction, quat_sub
        );

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
        
    }
#endif

#include "MappedEnum.hpp"

#define CURRENT_CLASS_BINDING
#define METHOD_BINDING(method) class_binding[#method] = & CURRENT_CLASS_BINDING::method;
#define ADD_METHOD_BINDINGS(...) MAPGEN_FOR_EACH(METHOD_BINDING, __VA_ARGS__)

#define TO_STR(maccro) #maccro
#define CLASS_CONSTRUCTOR(args) , CURRENT_CLASS_BINDING args
#define CREATE_CLASS_USERTYPE(class, default, ...) sol::usertype<CURRENT_CLASS_BINDING> class_binding = lua.new_usertype<CURRENT_CLASS_BINDING>(#class, sol::call_constructor, sol::constructors<CURRENT_CLASS_BINDING default MAPGEN_FOR_EACH(CLASS_CONSTRUCTOR, __VA_ARGS__)>());

// #define VLB_VLT_IMPL

#ifdef VLB_VLT_IMPL

    #include <Timer.hpp>

    void VulpineLuaBindings::VulpineTypes(sol::state &lua)
    {
        #undef CURRENT_CLASS_BINDING
        #define CURRENT_CLASS_BINDING BenchTimer

        CREATE_CLASS_USERTYPE(BenchTimer, (), (std::string))
        ADD_METHOD_BINDINGS(
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

#endif 

#ifdef VLB_ENT_IMPL
    #include "ECS/Entity.hpp"
    #include "ECS/ComponentTypeUI.hpp"
    #include "ECS/ModularEntityGroupping.hpp"

    void VulpineLuaBindings::Entities(sol::state &lua)
    {
        // TODO: figure out how to add bindings for sanctia types
        #undef CURRENT_CLASS_BINDING
        #define CURRENT_CLASS_BINDING Entity
        CREATE_CLASS_USERTYPE(Entity, (), ())
        class_binding["toStr"] = & Entity::toStr;
        // ADD_METHOD_BINDINGS(
            // toStr

            // comp<EntityGroupInfo>,
            // comp<EntityInfos>,
            // comp<WidgetBackground>,
            // comp<WidgetBox>,
            // comp<WidgetButton>,
            // comp<WidgetSprite>,
            // comp<WidgetState>,
            // comp<WidgetStyle>,
            // comp<WidgetText>,
            // comp<WidgetUI_Context>,

            // hasComp<EntityGroupInfo>,
            // hasComp<EntityInfos>,
            // hasComp<WidgetBackground>,
            // hasComp<WidgetBox>,
            // hasComp<WidgetButton>,
            // hasComp<WidgetSprite>,
            // hasComp<WidgetState>,
            // hasComp<WidgetStyle>,
            // hasComp<WidgetText>,
            // hasComp<WidgetUI_Context>,

            // set<EntityGroupInfo>,
            // set<EntityInfos>,
            // set<WidgetBackground>,
            // set<WidgetBox>,
            // set<WidgetButton>,
            // set<WidgetSprite>,
            // set<WidgetState>,
            // set<WidgetStyle>,
            // set<WidgetText>,
            // set<WidgetUI_Context>,

            // removeComp<EntityGroupInfo>,
            // removeComp<EntityInfos>,
            // removeComp<WidgetBackground>,
            // removeComp<WidgetBox>,
            // removeComp<WidgetButton>,
            // removeComp<WidgetSprite>,
            // removeComp<WidgetState>,
            // removeComp<WidgetStyle>,
            // removeComp<WidgetText>,
            // removeComp<WidgetUI_Context>,
        // );
    }
#endif 