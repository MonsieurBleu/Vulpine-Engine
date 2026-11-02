#pragma once

#include <string>
#include <functional>

struct Flag;

namespace Option 
{
    struct Preset
    {
        Preset(const char * flagName) : flagName(flagName){};
        const std::string flagName;
        std::string shaderDefine;
        void operator()();
    };

    struct Category
    {
        Category(const char * name) : name(name){};
        const std::string name;
        std::vector<Preset> presets;
        void apply();
    };

    extern std::vector<Category> categories;
    extern bool shaderRefreshTrigger;

    void apply();
    void init();
};