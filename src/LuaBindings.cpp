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
#include <MathsUtils.hpp>

#include <Utils.hpp>

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