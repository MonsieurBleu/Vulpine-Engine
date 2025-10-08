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

    void Entities(sol::state& lua);
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


#include "MappedEnum.hpp"

#define CURRENT_CLASS_BINDING
#define METHOD_BINDING(method) class_binding[#method] = & CURRENT_CLASS_BINDING::method;
#define METHOD_BINDING_TEMPLATED_SINGLE(method, classType) class_binding[#method"_"#classType] = & CURRENT_CLASS_BINDING::method<classType>;
#define METHOD_BINDING_TEMPLATED(method, ...) MAPGEN_FOR_EACH_ONE_ARG(METHOD_BINDING_TEMPLATED_SINGLE, method, __VA_ARGS__)
#define ADD_MEMBER_BINDINGS(...) MAPGEN_FOR_EACH(METHOD_BINDING, __VA_ARGS__)
#define ADD_REFERENCE(thing) &thing
#define ADD_OVERLOADED_METHOD(name, ...) class_binding[#name] = sol::overload(MAPGEN_FOR_EACH(ADD_REFERENCE, __VA_ARGS__));
#define ENUM_BINDING(enumType) for(auto &i : enumType##Map) {sol::object obj = lua[#enumType][i.first]; auto dir = static_cast<enumType>(obj.as<typename std::underlying_type<enumType>::type>()); }

#define TO_STR(macro) #macro
#define CLASS_CONSTRUCTOR(args) , CURRENT_CLASS_BINDING args
#define CREATE_CLASS_USERTYPE(class, default, ...) sol::usertype<CURRENT_CLASS_BINDING> class_binding = lua.new_usertype<CURRENT_CLASS_BINDING>(#class, sol::call_constructor, sol::constructors<CURRENT_CLASS_BINDING default MAPGEN_FOR_EACH(CLASS_CONSTRUCTOR, __VA_ARGS__)>());
