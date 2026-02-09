#pragma  once

#include <Scripting/LuaBindings.hpp>

/*
*  Hader string stream
*/
#ifdef __clang__
inline thread_local std::stringstream luaHeader;
inline thread_local std::stringstream luaHeaderAppendBuffer;
#elif defined(__GNUC__)
static inline thread_local std::stringstream luaHeader;
static inline thread_local std::stringstream luaHeaderAppendBuffer;
#endif

inline std::unordered_map<std::string_view, std::string_view> luaBindingTypesAlias;



/*
* FUNCTION TRAITS HELPER TEMPLATE
*/

template<typename T>
struct function_traits : function_traits<decltype(&T::operator())> {};
 
// Free function pointer
template<typename R, typename... Args>
struct function_traits<R(*)(Args...)> {
    using return_type = R;
    using args = std::tuple<Args...>;
};

// Member function pointer (this is what lambdas use)
template<typename C, typename R, typename... Args>
struct function_traits<R(C::*)(Args...) const> {
    using return_type = R;
    using args = std::tuple<Args...>;
};

// Member method
template<typename C, typename R, typename... Args>
struct function_traits<R(C::*)(Args...)> {
    using return_type = R;
    using args = std::tuple<Args...>;
};

/*
* DOCUMENTATION GENERATING FUNCTIONS
*/
std::string_view getLuaTypeName(const std::string_view &name);
std::vector<std::vector<std::string>> processArgsTraits(const std::vector<std::string_view> &args);
std::string getAutoParamName(int i, std::string type);
void writeConstructorDocumentation(
    const std::string_view &type, 
    const std::vector<std::vector<std::string>> &argsNames,
    const std::vector<std::vector<std::string>> &argsType
);
void writeFunctionDocumentation(
    const std::string_view &name, 
    const std::vector<std::vector<std::string>> &argsNames,
    const std::vector<std::vector<std::string>> &argsType
);
void writeFunctionDocumentation(
    bool singleLign,
    const std::string_view &name,
    const std::string_view &ret,
    const std::vector<std::string> &argsNames,
    const std::vector<std::string> &argsType
);

/*
* BIDNG AND DOCUMENTATION GENERATION MACCROS
*/

// HELPER AND BASIC STUFF

#define UNPARENT(...) __VA_ARGS__
#define UNPARENT_FIRST(first, second) first
#define UNPARENT_SECOND(first, second) second
#define EVAL(...)  EVAL1(EVAL1(EVAL1(__VA_ARGS__)))
#define EVAL1(...) EVAL2(EVAL2(EVAL2(__VA_ARGS__)))
#define EVAL2(...) EVAL3(EVAL3(EVAL3(__VA_ARGS__)))
#define EVAL3(...) __VA_ARGS__

#define __VBIND_TO_STR_EXPAND(x) #x
#define __VBIND_TO_STR(x) __VBIND_TO_STR_EXPAND(x)

#define VBIND_INIT_HEADER \
    luaHeader << "---@meta\n"; \
    luaHeader << "------------------------------------------------------------\n"; \
    luaHeader << "--- This header was automaticly written by Vulpine Engine.\n"; \
    luaHeader << "--- It contains every c++ to lua bindings created by the engine.\n"; \
    luaHeader << "--- This file is only used as a hint tool for IDEs, it nos referenced or used in the final application \n"; \
    luaHeader << "------------------------------------------------------------\n\n"; \

#define VBIND_INIT_HEADER_CATEGORY(category) \
    luaHeader << "\n\n\n------------------------------------------------------------\n"; \
    luaHeader << "--- " << category << " BINDINGS\n"; \
    luaHeader << "------------------------------------------------------------\n"; \

    

// FUNCTIONS 
#define __VBIND_FUNCTION_STATIC_CAST_OVERLOAD_IMPL(f, ret, ...) static_cast<UNPARENT ret (*) (__VA_ARGS__)>(& f)
#define __VBIND_FUNCTION_STATIC_CAST_OVERLOAD(f, ...) EVAL(__VBIND_FUNCTION_STATIC_CAST_OVERLOAD_IMPL(f, __VA_ARGS__))
#define __VBIND_FUNCTION_STATIC_CAST_OVERLOAD_EXPAND(f, args) , __VBIND_FUNCTION_STATIC_CAST_OVERLOAD(f, UNPARENT args)
#define __VBIND_FUNCTION_STATIC_CAST_OVERLOAD_EXPAND_FIRST(f, args) __VBIND_FUNCTION_STATIC_CAST_OVERLOAD(f, UNPARENT args)


#define __VBIND_ADD_FUNCTION_OVERLOADS_IMPL(alias, f, first, ...) \
    lua.set_function(alias, sol::overload( \
        __VBIND_FUNCTION_STATIC_CAST_OVERLOAD_EXPAND_FIRST(f, first) \
        MAPGEN_FOR_EACH_ONE_ARG(__VBIND_FUNCTION_STATIC_CAST_OVERLOAD_EXPAND, f, __VA_ARGS__)));

#define __VBIND_ADD_FUNCTION_OVERLOADS(alias, f, ...) EVAL(__VBIND_ADD_FUNCTION_OVERLOADS_IMPL(alias, f, __VA_ARGS__))

#define __VBIND_FUNCTION_ARGS_TO_STRING_IMPL(r, ...) \
    type_name<function_traits<UNPARENT r (*) (__VA_ARGS__)>::args>(), \
    type_name<function_traits<UNPARENT r (*) (__VA_ARGS__)>::return_type>(), 
#define __VBIND_FUNCTION_ARGS_TO_STRING(...) EVAL(__VBIND_FUNCTION_ARGS_TO_STRING_IMPL(__VA_ARGS__))
#define __VBIND_FUNCTION_ARGS_TO_STRING_EXPAND(args) __VBIND_FUNCTION_ARGS_TO_STRING(UNPARENT args)

#define VBIND_ADD_FUNCTION_OVERLOAD_ALIAS(alias, f, args, names) \
    __VBIND_ADD_FUNCTION_OVERLOADS(#alias, f, UNPARENT args) \
    writeFunctionDocumentation( #alias, \
        {MAPGEN_FOR_EACH(__VBIND_TO_STRING_VECTOR, UNPARENT names)}, \
        processArgsTraits({MAPGEN_FOR_EACH(__VBIND_FUNCTION_ARGS_TO_STRING_EXPAND, UNPARENT args)}) \
    )

#define VBIND_ADD_FUNCTION_OVERLOAD(f, args, names) VBIND_ADD_FUNCTION_OVERLOAD_ALIAS(f, f, args, names)

#define VBIND_ADD_FUNCTION_ALIAS(alias, f, names) \
    lua[#alias] = &f; \
    writeFunctionDocumentation(false, \
        #alias, \
        getLuaTypeName(type_name<function_traits<decltype(&f)>::return_type>()), \
        { UNPARENT names }, \
        processArgsTraits({{type_name<function_traits<decltype(&f)>::args>()}})[0] \
    );

#define VBIND_ADD_FUNCTION(f, names) VBIND_ADD_FUNCTION_ALIAS(f, f, names)

#define VBIND_ADD_FUNCTION_EXPAND(x) EVAL(VBIND_ADD_FUNCTION x)

#define VBIND_ADD_FUNCTIONS(...) MAPGEN_FOR_EACH(VBIND_ADD_FUNCTION_EXPAND, __VA_ARGS__)

// CLASS AND MEMBERS DECLARATION

#define ___VBIND_CURRENT_CLASS_ALIAS luaBindingTypesAlias[type_name<CURRENT_CLASS_BINDING>()]

#define VBIND_CLASS_DECLARE_ALIAS(__class__, __alias__) \
    luaBindingTypesAlias[type_name<__class__>()] = __VBIND_TO_STR(__alias__); \
    luaBindingTypesAlias[__VBIND_TO_STR(__alias__)] = __VBIND_TO_STR(__alias__);

#define VBIND_CLASS_DECLARE(__class__) VBIND_CLASS_DECLARE_ALIAS(__class__, __class__)

#define VBIND_CREATE_CLASS \
    sol::usertype<CURRENT_CLASS_BINDING> class_binding = lua.new_usertype<CURRENT_CLASS_BINDING>(___VBIND_CURRENT_CLASS_ALIAS); \
    luaHeader << "---@class " << ___VBIND_CURRENT_CLASS_ALIAS << "\n";

#define VBIND_ADD_MEMBER(x) \
    class_binding[#x] = & CURRENT_CLASS_BINDING::x; \
    luaHeader << "---@field " << #x << " " << getLuaTypeName(type_name<decltype(CURRENT_CLASS_BINDING::x)>()) << "\n";

#define VBIND_ADD_MEMBERS(...) \
    luaHeader << "--->>> Members\n"; \
    MAPGEN_FOR_EACH(VBIND_ADD_MEMBER, __VA_ARGS__)

#define VBIND_CLASS_END luaHeader << luaHeaderAppendBuffer.str(); luaHeaderAppendBuffer = std::stringstream();; luaHeader << "\n";



// CONSTRUCTORS

#define __VBIND_CONSTRUCTOR_STYLING(args) , CURRENT_CLASS_BINDING args

#define __VBIND_ADD_CONSTRUCTORS(...) \
    class_binding.set(sol::call_constructor, sol::constructors<CURRENT_CLASS_BINDING () MAPGEN_FOR_EACH(__VBIND_CONSTRUCTOR_STYLING, __VA_ARGS__)>()); 

#define __VBIND_TO_STRING_VECTOR(x) {UNPARENT x},

#define __VBIND_CONSTRUCTOR_ARGS_TO_STRING(x) type_name<function_traits<CURRENT_CLASS_BINDING (*) x>::args>(),

#define VBIND_ADD_CONSTRUCTORS(args, names)                                                     \
    __VBIND_ADD_CONSTRUCTORS(UNPARENT args)                                                     \
    writeConstructorDocumentation(__VBIND_TO_STR(CURRENT_CLASS_BINDING),                        \
        {MAPGEN_FOR_EACH(__VBIND_TO_STRING_VECTOR, UNPARENT names)},                            \
        processArgsTraits({MAPGEN_FOR_EACH(__VBIND_CONSTRUCTOR_ARGS_TO_STRING, UNPARENT args)}) \
    );



// OPERATORS
static std::unordered_map<std::string_view, std::string_view> luaOperatorNames = {{"+", "add"}, {"-", "sub"}, {"*", "mul"}, {"/", "div"}, {"[", "index"}, {"(", "call"}};

#define __VBIND_OPERATOR_DOC(op, opend, type)                                                                       \
    {                                                                                                               \
        auto __TMP_OP__ = __VBIND_OPERATOR_LAMBDA(op, opend, type);                                                 \
        if(std::string(#op) == "[")                                                                                 \
            luaHeader << "---@field [" << getLuaTypeName(__VBIND_TO_STR(type)) << "] "                              \
                      << getLuaTypeName(type_name<function_traits<decltype(__TMP_OP__)>::return_type>())<< "\n";    \
        else                                                                                                        \
            luaHeader << "---@operator " << luaOperatorNames[__VBIND_TO_STR(op)]                                    \
                      << "(" << getLuaTypeName(__VBIND_TO_STR(type)) << ") : "                                      \
                      << getLuaTypeName(type_name<function_traits<decltype(__TMP_OP__)>::return_type>()) << "\n";   \
    }

#define __VBIND_OPERATOR_LAMBDA(op, opend, type) [](const CURRENT_CLASS_BINDING &a, const type b){return a op b opend;}
#define __VBIND_OPERATOR_LAMBDA_COMMA(op, opend, type) , __VBIND_OPERATOR_LAMBDA(op, opend, type)
#define __VBIND_OPERATOR_LAMBDA_LIST(op, opend, first, ...) __VBIND_OPERATOR_LAMBDA(op, opend, first) MAPGEN_FOR_EACH_TWO_ARG(__VBIND_OPERATOR_LAMBDA_COMMA, op, opend, __VA_ARGS__)

#define __VBIND_OPERATOR_LAMBDA_2WAY(op, opend, type) [](CURRENT_CLASS_BINDING &a, const type b){return a op b opend;}, [](type a, const CURRENT_CLASS_BINDING &b){return a op b opend;}
#define __VBIND_OPERATOR_LAMBDA_COMMA_2WAY(op, opend, type) , __VBIND_OPERATOR_LAMBDA_2WAY(op, opend, type)
#define __VBIND_OPERATOR_LAMBDA_LIST_2WAY(op, opend, first, ...) __VBIND_OPERATOR_LAMBDA_2WAY(op, opend, first) MAPGEN_FOR_EACH_TWO_ARG(__VBIND_OPERATOR_LAMBDA_COMMA_2WAY, op, opend, __VA_ARGS__)


#define __VBIND_ADD_OPERATOR_2WAY(op, opend, solfunc, first, ...) \
    luaHeader << "--->>> Operator " << luaOperatorNames[__VBIND_TO_STR(op)] << "\n"; \
    __VBIND_OPERATOR_DOC(op, opend, first) \
    MAPGEN_FOR_EACH_TWO_ARG(__VBIND_OPERATOR_DOC, op, opend, __VA_ARGS__) \
    class_binding.set(solfunc, sol::overload( \
        __VBIND_OPERATOR_LAMBDA_LIST_2WAY(op, opend, first, __VA_ARGS__) \
    ));

#define __VBIND_ADD_OPERATOR(op, opend, solfunc, first, ...) \
    luaHeader << "--->>> Operator " << luaOperatorNames[__VBIND_TO_STR(op)] << "\n"; \
    __VBIND_OPERATOR_DOC(op, opend, first) \
    MAPGEN_FOR_EACH_TWO_ARG(__VBIND_OPERATOR_DOC, op, opend, __VA_ARGS__) \
    class_binding.set(solfunc, sol::overload( \
        __VBIND_OPERATOR_LAMBDA_LIST(op, opend, first, __VA_ARGS__) \
    ));

#define VBIND_ADD_OPERATOR_ADD(first, ...) __VBIND_ADD_OPERATOR_2WAY(+, ;, sol::meta_function::addition, first, __VA_ARGS__)
#define VBIND_ADD_OPERATOR_SUB(first, ...) __VBIND_ADD_OPERATOR_2WAY(-, ;, sol::meta_function::subtraction, first, __VA_ARGS__)
#define VBIND_ADD_OPERATOR_MUL(first, ...) __VBIND_ADD_OPERATOR_2WAY(*, ;, sol::meta_function::multiplication, first, __VA_ARGS__)
#define VBIND_ADD_OPERATOR_DIV(first, ...) __VBIND_ADD_OPERATOR_2WAY(/, ;, sol::meta_function::division, first, __VA_ARGS__)
#define VBIND_ADD_OPERATOR_INDEX(first, ...) __VBIND_ADD_OPERATOR([, ], sol::meta_function::index, first, __VA_ARGS__)
// #define VBIND_ADD_OPERATOR_CALL(first, ...) __VBIND_ADD_OPERATOR((, ), sol::meta_function::call, first, __VA_ARGS__)



// METHODS
#define VBIND_ADD_METHOD_ALIAS(alias, f, names) \
    class_binding[#alias] = &CURRENT_CLASS_BINDING::f; \
    writeFunctionDocumentation(true, \
        #alias, \
        getLuaTypeName(type_name<function_traits<decltype(&CURRENT_CLASS_BINDING::f)>::return_type>()), \
        { UNPARENT names }, \
        processArgsTraits({{type_name<function_traits<decltype(&CURRENT_CLASS_BINDING::f)>::args>()}})[0] \
    );

#define VBIND_ADD_METHOD(f, names) VBIND_ADD_METHOD_ALIAS(f, f, names)

#define VBIND_ADD_METHOD_AUTO_NAMES(f) VBIND_ADD_METHOD_ALIAS(f, f, ())

#define VBIND_ADD_METHODS(...) MAPGEN_FOR_EACH(VBIND_ADD_METHOD_AUTO_NAMES, __VA_ARGS__)


// ENUMS
#define __VBIND_ENUM_WRITE_MEMBER(name, value) luaHeader << "\t" << name << " = " << (int)value << ",\n";
#define __VBIND_ENUM_WRITE_MEMBER_EXPAND(x) EVAL(__VBIND_ENUM_WRITE_MEMBER x)

#define __VBIND_ENUM_ADD_MEMBER(x) , UNPARENT x

#define VBIND_ADD_ENUM(name, first, ...) \
    luaHeader << "---@enum " << name << "\n " << name << " = {\n"; \
    __VBIND_ENUM_WRITE_MEMBER first \
    MAPGEN_FOR_EACH(__VBIND_ENUM_WRITE_MEMBER_EXPAND, __VA_ARGS__) \
    luaHeader << "}\n"; \
    lua.new_enum(name, UNPARENT first MAPGEN_FOR_EACH(__VBIND_ENUM_ADD_MEMBER, __VA_ARGS__));

    
// GLOBALS

// #define VBIND_ADD_GLOBAL(value, alias)