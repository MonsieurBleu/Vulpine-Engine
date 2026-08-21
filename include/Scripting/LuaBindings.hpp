#pragma once

#define SOL_LUAJIT          1
#define SOL_ALL_SAFETIES_ON 1
#define SOL_PRINT_ERRORS    1
#include <sol/sol.hpp>



/* WIP Lua annotation generator for better IDE support
 * Can kinda work but cannot get things like function arguments names which kinda defeats the point of IDE support
#include <cxxabi.h>
namespace LuaAnnotationGenerator {
std::string demangled(char const* tname)
{
    std::unique_ptr<char, void (*)(void*)>
        name { abi::__cxa_demangle(tname, 0, 0, nullptr), std::free };
    return { name.get() };
}

class LuaAnnotation {
public:
    virtual std::string toString() = 0;
};

// template <typename T>
// class LuaAnnotationEnum : LuaAnnotation {
//     std::string name;
//     std::vector<std::pair<std::string, T>> values;
// public:
//     LuaAnnotationEnum(const std::string& name) : name(name) {}
//     void addValue(const std::string& name, T value) {values.push_back({name, value});}
//     std::string toString() override {
//         std::string res = "---@enum " + name + "\n";
//         for(auto& v : values) {
//             res += "---@field " + v.first + " " + demangled(typeid(T).name()) 
//         }
//         return res;
//     }
// };

class LuaAnnotationFunction : LuaAnnotation {
    std::string name;
    std::vector<std::string> args; 
    std::string returnType;
public:
    LuaAnnotationFunction(const std::string& name) : name(name) {}
    void addArg(const std::string& type) {args.push_back(type);}
    void setReturnType(const std::string& type) {returnType = type;}
    std::string toString() override {
        std::string res;
        for(auto& a : args) {
            res += "---@param " + a.first + " " + a.second + "\n";
        }
        if(!returnType.empty())
            res += "---@return " + returnType + "\n";
        res += "function " + name + "(";
        for(size_t i = 0; i < args.size(); i++) {
            res += args[i].first;
            if(i < args.size() - 1)
                res += ", ";
        }
        res += ") end\n";
        return res;
    }

    std::string toFunctionTypeString(std::string className) {
        std::string res = "fun(self: " + className + ", ";
        for(size_t i = 0; i < args.size(); i++) {
            res += args[i].first + ": " + args[i].second;
            if(i < args.size() - 1)
                res += ", ";
        }
        res += ")";
        if(!returnType.empty())
            res += ": " + returnType;
        return res;
    }
};

class LuaAnnotationClass : LuaAnnotation {
    std::string name;
    std::vector<std::pair<std::string, std::string>> members; // name, type
    std::vector<LuaAnnotationFunction> methods;
public:
    LuaAnnotationClass(const std::string& name) : name(name) {}
    void addMember(const std::string& name, const std::string& type) {members.push_back({name, type});}
    void addMethod(const LuaAnnotationFunction& method) {methods.push_back(method);}
    std::string toString() override {
        std::string res = "---@class " + name + "\n";
        for(auto& m : members) {
            res += "---@field " + m.first + " " + m.second + "\n";
        }
        for(auto& meth : methods) {
            res += meth.toFunctionTypeString(name) + "\n";
        }
        return res;
    }
};

class LuaAnnotationGenerator {
    std::vector<std::shared_ptr<LuaAnnotation>> annotations;
public:
    void addAnnotation(const std::shared_ptr<LuaAnnotation>& annotation) {
        annotations.push_back(annotation);
    }
    std::string generate() {
        std::string res;
        for(auto& ann : annotations) {
            res += ann->toString() + "\n";
        }
        return res;
    }
};

} // namespace LuaAnnotationGenerator
*/

namespace VulpineLuaBindings
{
    void bindAll(sol::state& lua);

    void glm(sol::state& lua);

    void VulpineTypes(sol::state& lua);

    // static void globals(sol::state& lua);

    // static void controls(sol::state& lua);

    // static void states(sol::state& lua);

    void Entities(sol::state& lua);

    void Utils(sol::state& lua);
}


/* 
* DEPRECATED !!!
* DO NOT USE
*/

#define BIND_FUNCTION(f) lua[#f] = f;
#define BIND_FUNCTIONS(...) MAPGEN_FOR_EACH(BIND_FUNCTION, __VA_ARGS__)

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

#define CURRENT_CLASS_BINDING void
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
#define CREATE_CLASS_USERTYPE_ALIAS(class, alias, default, ...) sol::usertype<CURRENT_CLASS_BINDING> class_binding = lua.new_usertype<CURRENT_CLASS_BINDING>(alias, sol::call_constructor, sol::constructors<CURRENT_CLASS_BINDING default MAPGEN_FOR_EACH(CLASS_CONSTRUCTOR, __VA_ARGS__)>());
