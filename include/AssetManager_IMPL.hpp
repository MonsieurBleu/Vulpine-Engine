#pragma once
/*
    Make sur to include the game's ECS version, the whole reason why this is inside a header
*/

#include <AssetManager.hpp>

#include <ModManager.hpp>
#include <Graphics/Animation.hpp>
#include <Graphics/Skeleton.hpp>
#include <Graphics/ObjectGroup.hpp>
#include <Scripting/ScriptInstance.hpp>

#include <Flags.hpp>

template<typename T> void loadAllInfos(std::string type, bool autoLoadData = false)
{
    auto assetList = AssetLoadInfos::assetList[type];

    Loader<T>::loadingInfos.clear();

    for(auto &assetVersions : assetList)
    {
        AssetVersion asset = modImportanceList.getCorrectVersionToUse(assetVersions.second, type, assetVersions.first);
        
        if(asset.textless)
            Loader<T>::addInfosTextless(asset.file.c_str(), asset.textlessPrefix);
        else
            Loader<T>::addInfos(asset.file.c_str());
    }

    if(autoLoadData)
    {
        for(auto &i : Loader<T>::loadingInfos)
        {
            Loader<T>::get(i.first);
        }            
    }
} 

void loadAllModdedAssetsInfos(const char *filename)
{
    for (auto f : std::filesystem::recursive_directory_iterator(filename))
    {
        if (f.is_directory()) continue;
        std::string path = f.path().string();
        AssetVersion version(path);
        AssetLoadInfos::addToGlobalList(version);
    }

    #define LOAD_ALL(T) loadAllInfos<T>(#T);
    #define LOAD_ALL_DATA(T) loadAllInfos<T>(#T, true);

    LOAD_ALL(ObjectGroup)
    LOAD_ALL(ObjectGroupRef)
    LOAD_ALL(MeshModel3D)
    LOAD_ALL(MeshMaterial)
    LOAD_ALL(MeshModel3D)
    LOAD_ALL(Texture2D)
    LOAD_ALL(AnimationRef)
    LOAD_ALL(MeshVao)
    LOAD_ALL(SkeletonRef)
    LOAD_ALL(EntityRef)
    LOAD_ALL(ScriptInstance)

    LOAD_ALL(ShaderFragPath)
    LOAD_ALL(ShaderVertPath)
    LOAD_ALL(ShaderGeomPath)
    LOAD_ALL(ShaderTescPath)
    LOAD_ALL(ShaderTesePath)
    LOAD_ALL(ShaderInclPath)

    LOAD_ALL_DATA(Flags)
    LOAD_ALL(FlagWrapper)
}