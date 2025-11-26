#include <Scripting/LuaBindingUtils.hpp>
#include <lua.h>
// #include <sol/variadic_args.hpp>
// #include <sol/sol.hpp>

#include <fstream>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
using namespace glm;
#include <MathsUtils.hpp>

#include <AssetManagerUtils.hpp>

#include <Utils.hpp>

#include <Timer.hpp>

// MARK: Bind all
void VulpineLuaBindings::bindAll(sol::state& lua)
{
    #undef CURRENT_CLASS_BINDING
    luaHeader.clear();
    VBIND_INIT_HEADER

    VBIND_CLASS_DECLARE_ALIAS(void, nil)
    VBIND_CLASS_DECLARE_ALIAS(bool, boolean)

    VBIND_CLASS_DECLARE_ALIAS(int, integer)
    VBIND_CLASS_DECLARE_ALIAS(short, integer)
    VBIND_CLASS_DECLARE_ALIAS(long, integer)
    VBIND_CLASS_DECLARE_ALIAS(unsigned int, integer)
    VBIND_CLASS_DECLARE_ALIAS(unsigned short, integer)
    VBIND_CLASS_DECLARE_ALIAS(unsigned long, integer)

    VBIND_CLASS_DECLARE_ALIAS(long long, integer)
    VBIND_CLASS_DECLARE_ALIAS(unsigned long long, integer)

    VBIND_CLASS_DECLARE_ALIAS(float, number)
    VBIND_CLASS_DECLARE_ALIAS(double, number)

    VBIND_CLASS_DECLARE_ALIAS(std::string, string)

    glm(lua);
    VulpineTypes(lua);
    Entities(lua);
    Utils(lua);
}

int foo(int i, float e, const std::string &test){return 0;}
float foo(int i){return 0.f;}

int foo2(float j, ivec2 b){return 0;};

// MARK: GLM Bindings
void VulpineLuaBindings::glm(sol::state &lua)
{    
    VBIND_INIT_HEADER_CATEGORY("GLM")

    VBIND_CLASS_DECLARE(vec2)
    VBIND_CLASS_DECLARE(vec3)
    VBIND_CLASS_DECLARE(vec4)

    VBIND_CLASS_DECLARE(ivec2)
    VBIND_CLASS_DECLARE(ivec3)
    VBIND_CLASS_DECLARE(ivec4)

    
    VBIND_CLASS_DECLARE(mat2)
    VBIND_CLASS_DECLARE(mat3)
    VBIND_CLASS_DECLARE(mat4)
    
    VBIND_CLASS_DECLARE(imat2x2)
    VBIND_CLASS_DECLARE(imat3x3)
    VBIND_CLASS_DECLARE(imat4x4)

    VBIND_CLASS_DECLARE(quat)

    luaHeader << luaHeaderAppendBuffer.str() << "\n";

    #define CURRENT_CLASS_BINDING ivec2
    VBIND_CLASS_DECLARE(CURRENT_CLASS_BINDING)
    {
        VBIND_CREATE_CLASS
        VBIND_ADD_CONSTRUCTORS(
            ((int), (int, int), (ivec2), (vec2), (ivec3), (vec3), (ivec4), (vec4)),
            (("x"), ("x", "y"))
        )
        VBIND_ADD_MEMBERS(x, y, r, g)
        VBIND_ADD_OPERATOR_ADD(CURRENT_CLASS_BINDING, int)
        VBIND_ADD_OPERATOR_SUB(CURRENT_CLASS_BINDING, int)
        VBIND_ADD_OPERATOR_MUL(CURRENT_CLASS_BINDING, int)
        VBIND_ADD_OPERATOR_DIV(CURRENT_CLASS_BINDING, int)
        VBIND_ADD_OPERATOR_INDEX(int)
    }
    VBIND_CLASS_END
    #undef CURRENT_CLASS_BINDING

    #define CURRENT_CLASS_BINDING ivec3
    VBIND_CLASS_DECLARE(CURRENT_CLASS_BINDING)
    {
        VBIND_CREATE_CLASS
        VBIND_ADD_CONSTRUCTORS(
            ((int), (int, int, int), (ivec3), (vec3), (ivec4), (vec4)),
            (("x"), ("x", "y", "z"))
        )
        VBIND_ADD_MEMBERS(x, y, z, r, g, b)
        VBIND_ADD_OPERATOR_ADD(CURRENT_CLASS_BINDING, int)
        VBIND_ADD_OPERATOR_SUB(CURRENT_CLASS_BINDING, int)
        VBIND_ADD_OPERATOR_MUL(CURRENT_CLASS_BINDING, int)
        VBIND_ADD_OPERATOR_DIV(CURRENT_CLASS_BINDING, int)
        VBIND_ADD_OPERATOR_INDEX(int)
    }
    VBIND_CLASS_END
    #undef CURRENT_CLASS_BINDING

    #define CURRENT_CLASS_BINDING ivec4
    VBIND_CLASS_DECLARE(CURRENT_CLASS_BINDING)
    {
        VBIND_CREATE_CLASS
        VBIND_ADD_CONSTRUCTORS(
            ((int), (int, int, int, int), (ivec4), (vec4)),
            (("x"), ("x", "y", "z", "w"))
        )
        VBIND_ADD_MEMBERS(x, y, z, w, r, g, b, a)
        VBIND_ADD_OPERATOR_ADD(CURRENT_CLASS_BINDING, int)
        VBIND_ADD_OPERATOR_SUB(CURRENT_CLASS_BINDING, int)
        VBIND_ADD_OPERATOR_MUL(CURRENT_CLASS_BINDING, int)
        VBIND_ADD_OPERATOR_DIV(CURRENT_CLASS_BINDING, int)
        VBIND_ADD_OPERATOR_INDEX(int)
    }
    VBIND_CLASS_END
    #undef CURRENT_CLASS_BINDING



    #define CURRENT_CLASS_BINDING vec2
    VBIND_CLASS_DECLARE(CURRENT_CLASS_BINDING)
    {
        VBIND_CREATE_CLASS
        VBIND_ADD_CONSTRUCTORS(
            ((float), (float, float), (ivec2), (vec2), (ivec3), (vec3), (ivec4), (vec4)),
            (("x"), ("x", "y"))
        )
        VBIND_ADD_MEMBERS(x, y, r, g)
        VBIND_ADD_OPERATOR_ADD(CURRENT_CLASS_BINDING, float)
        VBIND_ADD_OPERATOR_SUB(CURRENT_CLASS_BINDING, float)
        VBIND_ADD_OPERATOR_MUL(CURRENT_CLASS_BINDING, float, mat2)
        VBIND_ADD_OPERATOR_DIV(CURRENT_CLASS_BINDING, float, mat2)
        VBIND_ADD_OPERATOR_INDEX(int)
    }
    VBIND_CLASS_END
    #undef CURRENT_CLASS_BINDING

    #define CURRENT_CLASS_BINDING vec3
    VBIND_CLASS_DECLARE(CURRENT_CLASS_BINDING)
    {
        VBIND_CREATE_CLASS
        VBIND_ADD_CONSTRUCTORS(
            ((float), (float, float, float), (ivec3), (vec3), (ivec4), (vec4)),
            (("x"), ("x", "y", "z"))
        )
        VBIND_ADD_MEMBERS(x, y, z, r, g, b)
        VBIND_ADD_OPERATOR_ADD(CURRENT_CLASS_BINDING, float)
        VBIND_ADD_OPERATOR_SUB(CURRENT_CLASS_BINDING, float)
        VBIND_ADD_OPERATOR_MUL(CURRENT_CLASS_BINDING, float, mat3, quat)
        VBIND_ADD_OPERATOR_DIV(CURRENT_CLASS_BINDING, float, mat3)
        VBIND_ADD_OPERATOR_INDEX(int)
    }
    VBIND_CLASS_END
    #undef CURRENT_CLASS_BINDING

    #define CURRENT_CLASS_BINDING vec4
    VBIND_CLASS_DECLARE(CURRENT_CLASS_BINDING)
    {
        VBIND_CREATE_CLASS
        VBIND_ADD_CONSTRUCTORS(
            ((float), (float, float, float, float), (ivec4), (vec4)),
            (("x"), ("x", "y", "z", "w"))
        )
        VBIND_ADD_MEMBERS(x, y, z, w, r, g, b, a)
        VBIND_ADD_OPERATOR_ADD(CURRENT_CLASS_BINDING, float)
        VBIND_ADD_OPERATOR_SUB(CURRENT_CLASS_BINDING, float)
        VBIND_ADD_OPERATOR_MUL(CURRENT_CLASS_BINDING, float, mat4)
        VBIND_ADD_OPERATOR_DIV(CURRENT_CLASS_BINDING, float, mat4)
        VBIND_ADD_OPERATOR_INDEX(int)
    }
    VBIND_CLASS_END
    #undef CURRENT_CLASS_BINDING



    #define CURRENT_CLASS_BINDING imat2x2
    VBIND_CLASS_DECLARE(CURRENT_CLASS_BINDING)
    {
        VBIND_CREATE_CLASS
        VBIND_ADD_CONSTRUCTORS(
            ((int), (int, int, int, int), (imat2x2), (mat2), (imat3x3), (mat3), (imat4x4), (mat4)),
            ()
        )
        VBIND_ADD_OPERATOR_ADD(CURRENT_CLASS_BINDING, int)
        VBIND_ADD_OPERATOR_SUB(CURRENT_CLASS_BINDING, int)
        VBIND_ADD_OPERATOR_MUL(CURRENT_CLASS_BINDING, int)
        VBIND_ADD_OPERATOR_DIV(int)
        VBIND_ADD_OPERATOR_INDEX(int)
    }
    VBIND_CLASS_END
    #undef CURRENT_CLASS_BINDING

    #define CURRENT_CLASS_BINDING imat3x3
    VBIND_CLASS_DECLARE(CURRENT_CLASS_BINDING)
    {
        VBIND_CREATE_CLASS
        VBIND_ADD_CONSTRUCTORS(
            ((int), (int, int, int, int, int, int, int, int, int), (imat3x3), (mat3), (imat4x4), (mat4)),
            ()
        )
        VBIND_ADD_OPERATOR_ADD(CURRENT_CLASS_BINDING, int)
        VBIND_ADD_OPERATOR_SUB(CURRENT_CLASS_BINDING, int)
        VBIND_ADD_OPERATOR_MUL(CURRENT_CLASS_BINDING, int)
        VBIND_ADD_OPERATOR_DIV(int)
        VBIND_ADD_OPERATOR_INDEX(int)
    }
    VBIND_CLASS_END
    #undef CURRENT_CLASS_BINDING


    #define CURRENT_CLASS_BINDING imat4x4
    VBIND_CLASS_DECLARE(CURRENT_CLASS_BINDING)
    {
        VBIND_CREATE_CLASS
        VBIND_ADD_CONSTRUCTORS(
            ((int), (int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int), (imat4x4), (mat4)),
            ()
        )
        VBIND_ADD_OPERATOR_ADD(CURRENT_CLASS_BINDING, int)
        VBIND_ADD_OPERATOR_SUB(CURRENT_CLASS_BINDING, int)
        VBIND_ADD_OPERATOR_MUL(CURRENT_CLASS_BINDING, int)
        VBIND_ADD_OPERATOR_DIV(int)
        VBIND_ADD_OPERATOR_INDEX(int)
    }
    VBIND_CLASS_END
    #undef CURRENT_CLASS_BINDING


    #define CURRENT_CLASS_BINDING mat2
    VBIND_CLASS_DECLARE(CURRENT_CLASS_BINDING)
    {
        VBIND_CREATE_CLASS
        VBIND_ADD_CONSTRUCTORS(
            ((float), (float, float, float, float), (imat2x2), (mat2), (imat3x3), (mat3), (imat4x4), (mat4)),
            ()
        )
        VBIND_ADD_OPERATOR_ADD(CURRENT_CLASS_BINDING, float)
        VBIND_ADD_OPERATOR_SUB(CURRENT_CLASS_BINDING, float)
        VBIND_ADD_OPERATOR_MUL(CURRENT_CLASS_BINDING, float, vec2)
        VBIND_ADD_OPERATOR_DIV(CURRENT_CLASS_BINDING, float, vec2)
        VBIND_ADD_OPERATOR_INDEX(int)
    }
    VBIND_CLASS_END
    #undef CURRENT_CLASS_BINDING

    #define CURRENT_CLASS_BINDING mat3
    VBIND_CLASS_DECLARE(CURRENT_CLASS_BINDING)
    {
        VBIND_CREATE_CLASS
        VBIND_ADD_CONSTRUCTORS(
            ((float), (float, float, float, float, float, float, float, float, float), (imat3x3), (mat3), (imat4x4), (mat4)),
            ()
        )
        VBIND_ADD_OPERATOR_ADD(CURRENT_CLASS_BINDING, float)
        VBIND_ADD_OPERATOR_SUB(CURRENT_CLASS_BINDING, float)
        VBIND_ADD_OPERATOR_MUL(CURRENT_CLASS_BINDING, float, vec3)
        VBIND_ADD_OPERATOR_DIV(CURRENT_CLASS_BINDING, float, vec3)
        VBIND_ADD_OPERATOR_INDEX(int)
    }
    VBIND_CLASS_END
    #undef CURRENT_CLASS_BINDING

    #define CURRENT_CLASS_BINDING mat4
    VBIND_CLASS_DECLARE(CURRENT_CLASS_BINDING)
    {
        VBIND_CREATE_CLASS
        VBIND_ADD_CONSTRUCTORS(
            ((float), (float, float, float, float, float, float, float, float, float, float, float, float, float, float, float, float), (imat4x4), (mat4)),
            ()
        )
        VBIND_ADD_OPERATOR_ADD(CURRENT_CLASS_BINDING, float)
        VBIND_ADD_OPERATOR_SUB(CURRENT_CLASS_BINDING, float)
        VBIND_ADD_OPERATOR_MUL(CURRENT_CLASS_BINDING, float, vec4)
        VBIND_ADD_OPERATOR_DIV(CURRENT_CLASS_BINDING, float, vec4)
        VBIND_ADD_OPERATOR_INDEX(int)
    }
    VBIND_CLASS_END
    #undef CURRENT_CLASS_BINDING




    #define CURRENT_CLASS_BINDING quat
    {
        VBIND_CREATE_CLASS
        VBIND_ADD_CONSTRUCTORS(
            ((float, float, float, float), (mat3),  (vec3)),
            (("w"  , "x"  , "y"  , "z") ,  ("rot"), ("euler"))
        )
        VBIND_ADD_OPERATOR_ADD(CURRENT_CLASS_BINDING)
        VBIND_ADD_OPERATOR_SUB(CURRENT_CLASS_BINDING)
        VBIND_ADD_OPERATOR_MUL(CURRENT_CLASS_BINDING, float, vec3)
    }
    VBIND_CLASS_END
    #undef CURRENT_CLASS_BINDING

    VBIND_ADD_FUNCTION_OVERLOAD(mix, 
        (
            ((float), const float, const float, const float), 
            ((vec2), const vec2 &, const vec2 &, const float),
            ((vec3), const vec3 &, const vec3 &, const float),
            ((vec4), const vec4 &, const vec4 &, const float)
            // ((vec2), const vec2 &, const vec2 &, const vec2 &),
            // ((vec3), const vec3 &, const vec3 &, const vec3 &),
            // ((vec4), const vec4 &, const vec4 &, const vec4 &)
        ), 
        ()
    );

    VBIND_ADD_FUNCTION_OVERLOAD(smoothstep,
        (
            ((float), const float, const float, const float), 
            ((vec2), const vec2 &, const vec2 &, const vec2 &),
            ((vec3), const vec3 &, const vec3 &, const vec3 &),
            ((vec4), const vec4 &, const vec4 &, const vec4 &)
        ),
        (
            ("lowerBound", "higherBound", "x"),
            ("lowerBound", "higherBound", "x"),
            ("lowerBound", "higherBound", "x"),
            ("lowerBound", "higherBound", "x")
        )
    );

    VBIND_ADD_FUNCTION_OVERLOAD(dot, (
        ((float), const vec2 &, const vec2 &),
        ((float), const vec3 &, const vec3 &),
        ((float), const vec4 &, const vec4 &)
    ), ());


    VBIND_ADD_FUNCTION_OVERLOAD(cross, (
        ((vec3), const vec3 &, const vec3 &)
    ), ());

    VBIND_ADD_FUNCTION_OVERLOAD(normalize, (
        ((vec2), const vec2 &),
        ((vec3), const vec3 &),
        ((vec4), const vec4 &),
        ((quat), const quat &)
    ), ());

    VBIND_ADD_FUNCTION_OVERLOAD(inverse, (
        ((mat2), const mat2 &),
        ((mat3), const mat3 &),
        ((mat4), const mat4 &),
        ((quat), const quat &)
    ), ());


    VBIND_ADD_FUNCTION_OVERLOAD(distance, (
        ((float), const float &, const float &),
        ((float), const vec2 &, const vec2 &),
        ((float), const vec3 &, const vec3 &),
        ((float), const vec4 &, const vec4 &)
    ), ());

    VBIND_ADD_FUNCTION_OVERLOAD(pow, (
        ((vec2), const vec2 &, const vec2 &),
        ((vec3), const vec3 &, const vec3 &),
        ((vec4), const vec4 &, const vec4 &)
    ), ());

    VBIND_ADD_FUNCTION_OVERLOAD(angle, (((float), const quat &), ((float), const vec2 &, const vec2 &)), ());
    VBIND_ADD_FUNCTION_OVERLOAD(axis, (((vec3), const quat &)), ());
    VBIND_ADD_FUNCTION_OVERLOAD(slerp, (((quat), const quat &, const quat &, const float)), ());

    VBIND_ADD_FUNCTION_OVERLOAD(radians, (
        ((float), const float),
        ((vec2), const vec2 &),
        ((vec3), const vec3 &),
        ((vec4), const vec4 &)
    ), ());

    VBIND_INIT_HEADER_CATEGORY("VULPINE MATH HELPER FUNCTIONS")

    VBIND_ADD_FUNCTIONS(
        (rgb2hsv, ("rgb")),
        (hsv2rgb, ("hsv")),
        (getSaturation, ("rgb")),
        (ColorHexToV, ("hex")),

        (slerpDirClamp, ("dir1", "dir2", "a", "wfront")),
        (PhiThetaToDir, ("phi", "theta")),
        (getPhiTheta, ("dir")),
        (directionToQuat, ()),
        (directionToEuler, ()),
        (projectPointOntoPlane, ()),
        (rayAlignedPlaneIntersect, ()),
        (viewToWorld, ("pos", "inverseProjectionView"))
    );
}

#include <Timer.hpp>
#include <Matrix.hpp>
#include <Utils.hpp>

bool lua__STR_CASE_STR (const std::string &a, const std::string &b){return STR_CASE_STR(a.c_str(), b.c_str()) != nullptr;};

std::string lua__replace (std::string a, const std::string &b, const std::string &c)
{
    replace(a, b, c);
    return a;
}

void VulpineLuaBindings::VulpineTypes(sol::state &lua)
{
    VBIND_INIT_HEADER_CATEGORY("VULPINE COMMONS")

    VBIND_CLASS_DECLARE(duration)

    #define CURRENT_CLASS_BINDING duration
    VBIND_CLASS_DECLARE(CURRENT_CLASS_BINDING)
    {
        VBIND_CREATE_CLASS
        VBIND_ADD_CONSTRUCTORS((), ())
        VBIND_ADD_METHODS(
            count,
        )
        VBIND_ADD_OPERATOR_ADD(duration)
        VBIND_ADD_OPERATOR_SUB(duration)
    }
    VBIND_CLASS_END
    #undef CURRENT_CLASS_BINDING

    // MARK: Bench Timer
    #define CURRENT_CLASS_BINDING BenchTimer
    VBIND_CLASS_DECLARE(CURRENT_CLASS_BINDING)
    {
        VBIND_CREATE_CLASS
        VBIND_ADD_CONSTRUCTORS(((std::string)), (("name")))
        VBIND_ADD_MEMBERS(
            speed
        );
        VBIND_ADD_METHODS(
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
            reset,
        )
    }
    VBIND_CLASS_END
    #undef CURRENT_CLASS_BINDING

    #define CURRENT_CLASS_BINDING LimitTimer
    VBIND_CLASS_DECLARE(CURRENT_CLASS_BINDING)
    {
        VBIND_CREATE_CLASS
        VBIND_ADD_CONSTRUCTORS(((float)), (("maxFPS")))
        VBIND_ADD_MEMBERS(
            freq
        );
        VBIND_ADD_METHODS(
            start, waitForEnd, activate, deactivate, toggle, isActivated
        )
    }
    VBIND_CLASS_END
    #undef CURRENT_CLASS_BINDING

    VBIND_CLASS_DECLARE(ModelStatus)
    VBIND_ADD_ENUM(
        "ModelStatus",
        ("HIDE", ModelStatus::HIDE),
        ("SHOW", ModelStatus::SHOW),
        ("UNDEFINED", ModelStatus::UNDEFINED)
    )
    
    // MARK: ModelState3D
    #define CURRENT_CLASS_BINDING ModelState3D
    VBIND_CLASS_DECLARE(CURRENT_CLASS_BINDING)
    {
        VBIND_CREATE_CLASS
        VBIND_ADD_CONSTRUCTORS((), ())
        VBIND_ADD_MEMBERS(
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
        )
        VBIND_ADD_METHODS(
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
    }
    VBIND_CLASS_END
    #undef CURRENT_CLASS_BINDING
}

#include <Globals.hpp>

// MARK: Utils
void VulpineLuaBindings::Utils(sol::state &lua)
{
    VBIND_INIT_HEADER_CATEGORY("VULPINE UTILS")
    VBIND_ADD_FUNCTION_ALIAS(STR_CASE_STR, lua__STR_CASE_STR, ());
    VBIND_ADD_FUNCTION_ALIAS(replace, lua__replace, ());

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
                std::cout << TERMINAL_NOTIF << TERMINAL_UNDERLINE << "[" << "Sanctia-Release/" << file << ":" << line << "]\n" << TERMINAL_RESET;
                for(auto arg : args)
                {
                    std::cout << lua["tostring"](arg.get<sol::object>()).get<std::string>() << " ";
                }
                std::cout << std::endl;
            }
        );
    }

    VBIND_INIT_HEADER_CATEGORY("VULPINE GLOBALS")

    #define CURRENT_CLASS_BINDING Globals
    VBIND_CLASS_DECLARE(CURRENT_CLASS_BINDING)
    {
        VBIND_CREATE_CLASS
        // VBIND_ADD_CONSTRUCTORS((), ())
        VBIND_ADD_MEMBERS(
            appTime,
            mainThreadTime,
            simulationTime,
            cpuTime,
            gpuTime,
            fpsLimiter,
            enablePhysics,
            sceneChromaticAbbColor1,
            sceneChromaticAbbColor2,
            sceneChromaticAbbAngleAmplitude,
            sceneHsvShift,
            sceneVignette
        )
        VBIND_ADD_METHODS(
            screenResolution,
            mousePosition,
            windowWidth,
            windowHeight,
            windowHasFocus,
            windowSize,
            mouseLeftClick,
            mouseLeftClickDown,
            mouseRightClick,
            mouseRightClickDown,
            mouse4Click,
            mouse4ClickDown,
            mouse5Click,
            mouse5ClickDown,
            mouseScrollOffset,
            clearMouseScroll,
            renderScale,
            renderSize
        );
    }
    luaHeader << "globals = {}\n";
    lua["globals"] = &globals;
    // VBIND_CLASS_END
    #undef CURRENT_CLASS_BINDING
}