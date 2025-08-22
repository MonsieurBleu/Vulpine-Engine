#pragma once

#include <list>
#include <vector>
#include <memory>
#include <MappedEnum.hpp>

struct Mod
{
    GENERATE_ENUM_FAST_REVERSE(ImportanceCategory, 
        _0_BASE_GAME,
        _1_USER_MOD,
        _2_COMPATIBILITY_PATCH
    )

    std::string name;
    ImportanceCategory category;

    Mod(const std::string name, ImportanceCategory category);
};

typedef std::shared_ptr<Mod> ModRef;

struct AssetVersion
{
    ModRef version;
    std::string file;

    bool textless = false;
    std::string textlessPrefix = "";

    AssetVersion(const std::string &file);
};

struct AssetLoadInfos : public std::vector<AssetVersion>
{
    static void addToGlobalList(AssetVersion &asset);

    /*
        List of all assets by name
    */
    static inline  std::unordered_map<
        std::string, 
        std::unordered_map<std::string, AssetLoadInfos>> assetList;
};

class ModList : public std::list<ModRef>
{
    private : 
        static const AssetVersion& getBestVersionFromList(
            const AssetLoadInfos& versions,
            const std::list<ModRef> &list,
            const std::string &type, 
            const std::string &name
        );
    
    public :
        /*
            Informations needed to assert special rules for importance overwrite on a per-asset basis.
            Should be quite usefull for compatibility patch between mods.
        */
        struct SpecialAssetOverwrite : std::list<ModRef>
        {
            // std::string assetName;
        };

        typedef std::unordered_map<std::string, SpecialAssetOverwrite> SpecialTypeOverwrites;

        std::unordered_map<std::string, SpecialTypeOverwrites> overwrites;

        const AssetVersion& getCorrectVersionToUse(
            const AssetLoadInfos& versions,
            const std::string &type, 
            const std::string &name
        )   const; 
};

void loadAllModdedAssetsInfos(const char *filename);

/*
    list of all mods shorted by importance 

    (similar to 'load order' in other engine, but there isn't any load order here)
*/
inline ModList modImportanceList; 