#pragma once

#include <unordered_map>
#include <string>

#define GENERATE_ENUM(name, ...)                        \
    enum name                                           \
    {                                                   \
        __VA_ARGS__                                     \
    };                                                  \
    static std::unordered_map<std::string, name> name##Map = {MAPGEN_FOR_EACH(MAP_ARG, __VA_ARGS__)}; \

    #define GENERATE_ENUM_INLINE(name, ...)                        \
    enum name                                           \
    {                                                   \
        __VA_ARGS__                                     \
    };                                                  \
    static inline std::unordered_map<std::string, name> name##Map = {MAPGEN_FOR_EACH(MAP_ARG, __VA_ARGS__)}; \

#define GENERATE_ENUM_FAST_REVERSE(name, ...)      \
    enum name                                           \
    {                                                   \
        __VA_ARGS__                                     \
    };                                                  \
    static inline const std::unordered_map<std::string, name> name##Map = {MAPGEN_FOR_EACH(MAP_ARG, __VA_ARGS__)}; \
    static inline const std::string name##ReverseMap[] = {MAPGEN_FOR_EACH(REVERSE_MAP_ARG, __VA_ARGS__)};

#define GENERATE_ENUM_TYPE(name, type, ...)      \
    enum name : type                                    \
    {                                                   \
        __VA_ARGS__                                     \
    };                                                  \
    static inline const std::unordered_map<std::string, name> name##Map = {MAPGEN_FOR_EACH(MAP_ARG, __VA_ARGS__)}; \


#define PARENS ()

// #define MAPGEN_EXPAND(...) MAPGEN_EXPAND4(MAPGEN_EXPAND4(MAPGEN_EXPAND4(MAPGEN_EXPAND4(MAPGEN_EXPAND4(__VA_ARGS__)))))
// #define MAPGEN_EXPAND4(...) MAPGEN_EXPAND3(MAPGEN_EXPAND3(MAPGEN_EXPAND3(MAPGEN_EXPAND3(MAPGEN_EXPAND3(__VA_ARGS__)))))
// #define MAPGEN_EXPAND3(...) MAPGEN_EXPAND2(MAPGEN_EXPAND2(MAPGEN_EXPAND2(MAPGEN_EXPAND2(MAPGEN_EXPAND2(__VA_ARGS__)))))
// #define MAPGEN_EXPAND2(...) MAPGEN_EXPAND1(MAPGEN_EXPAND1(MAPGEN_EXPAND1(MAPGEN_EXPAND1(MAPGEN_EXPAND1(__VA_ARGS__)))))
// #define MAPGEN_EXPAND1(...) __VA_ARGS__

// #define MAPGEN_FOR_EACH(macro, ...) __VA_OPT__(MAPGEN_EXPAND(MAPGEN_FOR_EACH_HELPER(macro, __VA_ARGS__)))
// #define MAPGEN_FOR_EACH_HELPER(macro, a1, ...) macro(a1) __VA_OPT__(MAPGEN_FOR_EACH_AGAIN PARENS(macro, __VA_ARGS__))
// #define MAPGEN_FOR_EACH_AGAIN() MAPGEN_FOR_EACH_HELPER


#define MAPGEN_EXPAND_ONE_ARG(...) MAPGEN_EXPAND_ONE_ARG4(MAPGEN_EXPAND_ONE_ARG4(MAPGEN_EXPAND_ONE_ARG4(MAPGEN_EXPAND_ONE_ARG4(MAPGEN_EXPAND_ONE_ARG4(__VA_ARGS__)))))
#define MAPGEN_EXPAND_ONE_ARG4(...) MAPGEN_EXPAND_ONE_ARG3(MAPGEN_EXPAND_ONE_ARG3(MAPGEN_EXPAND_ONE_ARG3(MAPGEN_EXPAND_ONE_ARG3(MAPGEN_EXPAND_ONE_ARG3(__VA_ARGS__)))))
#define MAPGEN_EXPAND_ONE_ARG3(...) MAPGEN_EXPAND_ONE_ARG2(MAPGEN_EXPAND_ONE_ARG2(MAPGEN_EXPAND_ONE_ARG2(MAPGEN_EXPAND_ONE_ARG2(MAPGEN_EXPAND_ONE_ARG2(__VA_ARGS__)))))
#define MAPGEN_EXPAND_ONE_ARG2(...) MAPGEN_EXPAND_ONE_ARG1(MAPGEN_EXPAND_ONE_ARG1(MAPGEN_EXPAND_ONE_ARG1(MAPGEN_EXPAND_ONE_ARG1(MAPGEN_EXPAND_ONE_ARG1(__VA_ARGS__)))))
#define MAPGEN_EXPAND_ONE_ARG1(...) __VA_ARGS__

#define MAPGEN_FOR_EACH_ONE_ARG(macro, arg1, ...) __VA_OPT__(MAPGEN_EXPAND_ONE_ARG(MAPGEN_FOR_EACH_ONE_ARG_HELPER(macro, arg1, __VA_ARGS__)))
#define MAPGEN_FOR_EACH_ONE_ARG_HELPER(macro, arg1, a1, ...) macro(arg1, a1) __VA_OPT__(MAPGEN_FOR_EACH_ONE_ARG_AGAIN PARENS(macro, arg1, __VA_ARGS__))
#define MAPGEN_FOR_EACH_ONE_ARG_AGAIN() MAPGEN_FOR_EACH_ONE_ARG_HELPER

#define MAPGEN_EXPAND_TWO_ARG(...) MAPGEN_EXPAND_ONE_ARG4(MAPGEN_EXPAND_ONE_ARG4(MAPGEN_EXPAND_ONE_ARG4(MAPGEN_EXPAND_ONE_ARG4(MAPGEN_EXPAND_ONE_ARG4(__VA_ARGS__)))))
#define MAPGEN_EXPAND_TWO_ARG4(...) MAPGEN_EXPAND_ONE_ARG3(MAPGEN_EXPAND_ONE_ARG3(MAPGEN_EXPAND_ONE_ARG3(MAPGEN_EXPAND_ONE_ARG3(MAPGEN_EXPAND_ONE_ARG3(__VA_ARGS__)))))
#define MAPGEN_EXPAND_TWO_ARG3(...) MAPGEN_EXPAND_ONE_ARG2(MAPGEN_EXPAND_ONE_ARG2(MAPGEN_EXPAND_ONE_ARG2(MAPGEN_EXPAND_ONE_ARG2(MAPGEN_EXPAND_ONE_ARG2(__VA_ARGS__)))))
#define MAPGEN_EXPAND_TWO_ARG2(...) MAPGEN_EXPAND_ONE_ARG1(MAPGEN_EXPAND_ONE_ARG1(MAPGEN_EXPAND_ONE_ARG1(MAPGEN_EXPAND_ONE_ARG1(MAPGEN_EXPAND_ONE_ARG1(__VA_ARGS__)))))
#define MAPGEN_EXPAND_TWO_ARG1(...) __VA_ARGS__

#define MAPGEN_FOR_EACH_TWO_ARG(macro, arg1, arg2, ...) __VA_OPT__(MAPGEN_EXPAND_TWO_ARG(MAPGEN_FOR_EACH_TWO_ARG_HELPER(macro, arg1, arg2, __VA_ARGS__)))
#define MAPGEN_FOR_EACH_TWO_ARG_HELPER(macro, arg1, arg2, a1, ...) macro(arg1, arg2, a1) __VA_OPT__(MAPGEN_FOR_EACH_TWO_ARG_AGAIN PARENS(macro, arg1, arg2, __VA_ARGS__))
#define MAPGEN_FOR_EACH_TWO_ARG_AGAIN() MAPGEN_FOR_EACH_TWO_ARG_HELPER

#define MAP_ARG(x) \
    {#x, x},


#define REVERSE_MAP_ARG(x) #x,

// ------------------------
// Forced expansion helpers (depth 1..10)
// ------------------------
#define MAPGEN_EXPAND1(...) __VA_ARGS__
#define MAPGEN_EXPAND2(...) MAPGEN_EXPAND1(MAPGEN_EXPAND1(MAPGEN_EXPAND1(MAPGEN_EXPAND1(MAPGEN_EXPAND1(__VA_ARGS__)))))
#define MAPGEN_EXPAND3(...) MAPGEN_EXPAND2(MAPGEN_EXPAND2(MAPGEN_EXPAND2(MAPGEN_EXPAND2(MAPGEN_EXPAND2(__VA_ARGS__)))))
#define MAPGEN_EXPAND4(...) MAPGEN_EXPAND3(MAPGEN_EXPAND3(MAPGEN_EXPAND3(MAPGEN_EXPAND3(MAPGEN_EXPAND3(__VA_ARGS__)))))
#define MAPGEN_EXPAND5(...) MAPGEN_EXPAND4(MAPGEN_EXPAND4(MAPGEN_EXPAND4(MAPGEN_EXPAND4(MAPGEN_EXPAND4(__VA_ARGS__)))))
#define MAPGEN_EXPAND(...) MAPGEN_EXPAND5(__VA_ARGS__)

// ------------------------
// Recursive MAPGEN_FOR_EACH using deep expansion
// ------------------------
#define MAPGEN_FOR_EACH(macro, ...) __VA_OPT__(MAPGEN_EXPAND(MAPGEN_FOR_EACH_HELPER(macro, __VA_ARGS__)))

#define MAPGEN_FOR_EACH_HELPER(macro, a1, ...) \
    macro(a1) __VA_OPT__(MAPGEN_FOR_EACH_AGAIN PARENS(macro, __VA_ARGS__))

#define MAPGEN_FOR_EACH_AGAIN() MAPGEN_FOR_EACH_HELPER
