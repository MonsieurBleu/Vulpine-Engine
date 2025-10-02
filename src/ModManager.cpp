#include <ModManager.hpp>
#include <assert.h>
#include <Utils.hpp>
#include <string.h>
#include <AssetManager.hpp>
#include <AssetManagerUtils.hpp>

Mod::Mod(const std::string name, ImportanceCategory category) : name(name), category(category){}

const AssetVersion& ModList::getCorrectVersionToUse(
    const AssetLoadInfos& versions, 
    const std::string &type, 
    const std::string &name
)   const
{
    /*
        First, we check for any special rule 
    */
    auto typeOverwrite = overwrites.find(type);
    if(typeOverwrite != overwrites.end())
    {
        auto assetOverwrite = typeOverwrite->second.find(name);

        if(assetOverwrite != typeOverwrite->second.end())
        {
            return getBestVersionFromList(versions, assetOverwrite->second, type, name);
        }
    }

    /*
        If no overwrite, use the main list
    */
    return getBestVersionFromList(versions, *this, type, name);
}

const AssetVersion& ModList::getBestVersionFromList(
    const AssetLoadInfos& versions, 
    const std::list<ModRef> &list,
    const std::string &type, 
    const std::string &name
)
{
    // assert(!versions.empty());

    for(auto &mod : list)
    {
        for(auto &ver : versions)
        {
            if(mod == ver.version)
                return ver;
        }
    }

    /*
        TODO : test
    */
    WARNING_MESSAGE(
        "The asset '" << name 
        << "' of type '" << type << "' doens't have any version with explicit importance. "
        << "Either the mod-list is broken, or you have installed a mod with incorrect folder structure. "
        << "The version '" << versions.back().version->name << "' will be used by default." 
    )

    return versions.back();
}

AssetVersion::AssetVersion(const std::string &file) : file(file)
{
    std::string versionName;
    Mod::ImportanceCategory category = Mod::ImportanceCategory::_0_BASE_GAME;

    bool doCopy = false;
    bool checkForModFolder = false;
    bool checkForCategory = false;
    bool checkForModName = false;
    for(auto c : file)
    {
        if(c == '/' || c == '\\')
        {
            if(doCopy)
            {
                doCopy = false;
                versionName += ' ';
            }

            checkForModFolder = true;
            continue;
        }

        if(checkForModFolder && c == '[')
        {
            checkForCategory = true;
            continue;
        }

        checkForModFolder = false;

        if(checkForCategory)
        {
            if(c == ']')
            {
                checkForCategory = false;
                checkForModName = true;
                continue;
            }

            switch (c)
            {
                case '1' : category = Mod::ImportanceCategory::_1_USER_MOD; break;
                case '2' : category = Mod::ImportanceCategory::_2_COMPATIBILITY_PATCH; break;
                default: category = Mod::ImportanceCategory::_0_BASE_GAME; break;
            }

            continue;
        }

        if(doCopy)
            versionName += c;

        if(checkForModName && c == ' ')
        {
            doCopy = true;
            checkForModName = false;
        }
    }

    if(versionName.back() == ' ') versionName.pop_back();

    if(versionName.empty()) versionName = "Base_Game";

    for(auto i : modImportanceList)
    {
        if(i->name == versionName)
        {
            i->category = category;
            version = i;
            return;
        }
    }

    for(auto i = modImportanceList.begin(); i != modImportanceList.end(); i++)
    {
        if(category > (*i)->category)
        {
            auto newElem = std::make_shared<Mod>(versionName,category);
            version = *modImportanceList.insert(
                i, newElem
            );

            return;
        }
    }

    modImportanceList.push_back(std::make_shared<Mod>(
        versionName,
        category
        ));
    version = modImportanceList.back();
}

void AssetLoadInfos::addToGlobalList(AssetVersion &asset)
{
    std::string type;

    std::string ext = getFileExtension(asset.file);

    const std::pair<
        std::pair<const std::string, const std::string>,
        std::pair<bool, const std::string>
    > filetypes[] = 
    {
        {{"vGroup", "ObjectGroup"}, {false, ""}},
        {{"vGroupRef", "ObjectGroupRef"}, {false, ""}},
        {{"vModel", "MeshModel3D"}, {false, ""}},
        {{"vMaterial", "MeshMaterial"}, {false, ""}},
        {{"vMeshModel", "MeshModel3D"}, {false, ""}},
        {{"vTexture2D", "Texture2D"}, {false, ""}},

        {{"vEntity", "EntityRef"}, {false, ""}},

        {{"frag", "ShaderFragPath"}, {true, ""}},
        {{"vert", "ShaderVertPath"}, {true, ""}},
        {{"geom", "ShaderGeomPath"}, {true, ""}},
        {{"tesc", "ShaderTescPath"}, {true, ""}},
        {{"tese", "ShaderTesePath"}, {true, ""}},
        {{"glsl", "ShaderInclPath"}, {true, ""}},

        {{"vAnimation", "AnimationRef"}, {true, ""}},
        {{"vMesh", "MeshVao"}, {true, ""}},
        {{"obj", "MeshVao"}, {true, ""}},
        {{"vSkeleton", "SkeletonRef"}, {true, ""}},

        {{"ktx", "Texture2D"}, {true, "source"}},
        {{"ktx2", "Texture2D"}, {true, "source"}},
        {{"png", "Texture2D"}, {true, "source"}},
        {{"jpg", "Texture2D"}, {true, "source"}},
        {{"jpeg", "Texture2D"}, {true, "source"}},
        {{"tga", "Texture2D"}, {true, "source"}},
        {{"psd", "Texture2D"}, {true, "source"}},
        {{"bmp", "Texture2D"}, {true, "source"}},
        {{"gif", "Texture2D"}, {true, "source"}},
        {{"pic", "Texture2D"}, {true, "source"}},
        {{"ppm", "Texture2D"}, {true, "source"}},
        {{"pgm", "Texture2D"}, {true, "source"}},
        {{"exr", "Texture2D"}, {true, "source"}},

        {{"lua", "ScriptInstance"}, {true, ""}}
    };

    for(auto i : filetypes)
        if(!strcmp(ext.c_str(), i.first.first.c_str()))
        {
            type = i.first.second;

            asset.textless = i.second.first;
            asset.textlessPrefix = i.second.second;

            break;
        }

    if(type.empty()) return;

    assetList[type][getNameOnlyFromPath(asset.file.c_str())].push_back(asset);
}

DATA_WRITE_FUNC_INIT(ModList)

    for(auto &i : data)
    {
        out->Entry();
        out->write("\"", 1);
        out->write(i->name.c_str(), i->name.size());
        out->write("\"", 1);
    }

DATA_WRITE_END_FUNC


DATA_READ_FUNC_INIT(ModList)

    IF_MEMBER(ModList){}
    else if(true)
    {
        data.push_back(std::make_shared<Mod>(member, Mod::ImportanceCategory::_0_BASE_GAME));
    }

DATA_READ_END_FUNC