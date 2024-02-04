#pragma once

#include <unordered_map>
#include <string>

#define GENERATE_ENUM(name, ...)                        \
    enum name                                           \
    {                                                   \
        __VA_ARGS__                                     \
    };                                                  \
    static std::unordered_map<std::string, name> name##Map = { \
        FOR_EACH(MAP_ARG, __VA_ARGS__)}

#define PARENS ()

#define EXPAND(...) EXPAND5(EXPAND5(EXPAND5(EXPAND5(EXPAND5(__VA_ARGS__)))))
#define EXPAND5(...) EXPAND4(EXPAND4(EXPAND4(EXPAND4(EXPAND4(__VA_ARGS__)))))
#define EXPAND4(...) EXPAND3(EXPAND3(EXPAND3(EXPAND3(EXPAND3(__VA_ARGS__)))))
#define EXPAND3(...) EXPAND2(EXPAND2(EXPAND2(EXPAND2(EXPAND2(__VA_ARGS__)))))
#define EXPAND2(...) EXPAND1(EXPAND1(EXPAND1(EXPAND1(EXPAND1(__VA_ARGS__)))))
#define EXPAND1(...) __VA_ARGS__

#define FOR_EACH(macro, ...) \
    __VA_OPT__(EXPAND(FOR_EACH_HELPER(macro, __VA_ARGS__)))
#define FOR_EACH_HELPER(macro, a1, ...) \
    macro(a1)                           \
        __VA_OPT__(FOR_EACH_AGAIN PARENS(macro, __VA_ARGS__))
#define FOR_EACH_AGAIN() FOR_EACH_HELPER

#define MAP_ARG(x) \
    {#x, x},

