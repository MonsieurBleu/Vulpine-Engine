#include <User/Options.hpp>
#include <Flags.hpp>
#include <Graphics/Shadinclude.hpp>

using namespace Option;

std::vector<Category> Option::categories;
bool Option::shaderRefreshTrigger = false;

void Preset::operator()()
{
    Flag *f = &Loader<Flag>::get(flagName);

    // apply(f);

    std::string shaderDefineName;

    std::string shaderDefineFlagName = flagName + "-SHADER_DEFINE_NAME";

    if(Loader<Flag>::loadingInfos.find(shaderDefineFlagName) != Loader<Flag>::loadingInfos.end())
    {
        shaderDefineName = Loader<Flag>::get(shaderDefineFlagName).flag->as_string();
    }

    if(!shaderDefineName.empty())
    {
        std::string old = shaderDefine;

        switch (f->flag->type)
        {
            case FlagData::Type::BOOL :
                if(f->flag->as_bool())
                    shaderDefine = "#define " + shaderDefineName  + '\n';
            break;

            default :
                shaderDefine = "#define " + f->flag->as_string() + '\n';
            break;
        }

        shaderRefreshTrigger = shaderRefreshTrigger or old != shaderDefine;
    }
}

void Category::apply()
{
    shaderRefreshTrigger = false;
    std::string shaderDefines;

    for(auto &p : presets)
    {
        p();
        shaderDefines+= p.shaderDefine;
    }

    if(shaderRefreshTrigger)
    {
        Shadinclude::optionShaderDefines = shaderDefines;

        for(auto i : Loader<MeshMaterial>::loadedAssets)
            i.second->reset(false);
    }
}

void Option::apply()
{
    shaderRefreshTrigger = false;
    std::string shaderDefines;

    for(auto &c : categories)
    {
        for(auto &p : c.presets)
        {
            p();
            shaderDefines += p.shaderDefine;
        }
    }

    if(shaderRefreshTrigger)
    {
        Shadinclude::optionShaderDefines = shaderDefines;

        // NOTIF_MESSAGE(shaderDefines)

        for(auto i : Loader<MeshMaterial>::loadedAssets)
            i.second->reset(false);
    }
}

void Option::init()
{
    for(auto &i : Loader<Flags>::loadedAssets)
    {
        if(i.first.starts_with("Options "))
        {
            categories.push_back(Category(i.first.data() + sizeof("Options")));

            // WARNING_MESSAGE("Creating Cateogry ", categories.back().name)

            Flags &f = Loader<Flags>::get(i.first);

            for(auto j : f.getAllFlagsMap())
            if(!j.first.ends_with("-SHADER_DEFINE_NAME"))
            {
                categories.back().presets.push_back(j.first.data());
            }
        }
    }
}
    