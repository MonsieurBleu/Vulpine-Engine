#pragma once
#include <VulpineParser.hpp>
#include <unordered_map>
#include <cassert>

#include <Utils.hpp>
#include <string.h>

/*
    Template class for asset loading from text based format, with dupplication prevention policy.

    How to use :
        loading a VulpineTextBuffRef from disk
        adding info for the corresponding type represented

        if you want to preload certain assets : 
            find their reference by name using loadingInfos and then use loadFromInfos();
            or
            simply request them by using the get() function 
        
        if you want to clear a ressource from memory 
            simply erase it from the loaded assets
        
        if you want to load any asset dependencies 
            simply request the asset, all dependencies of any kind will be loaded if they are not arleady

    How not to use : 
        clear a loading infos but not the corresponding loaded asset
*/
template<typename T>
class Loader
{
    private : 

    public : 

        Loader(){};
        Loader(VulpineTextBuffRef b) : Loader<T>(addInfos(b))
        {};

        VulpineTextBuffRef buff;
        char* name = nullptr;
        char* values = nullptr;
        char* end = nullptr;

        static inline std::unordered_map<std::string, T> loadedAssets;
        static inline std::unordered_map<std::string, std::unique_ptr<Loader<T>>> loadingInfos;

        static T& get(const std::string &elem)
        {
            auto res = Loader<T>::loadedAssets.find(elem);

            if(res == Loader<T>::loadedAssets.end())
            {
                auto infos = Loader<T>::loadingInfos.find(elem);

                if(infos == Loader<T>::loadingInfos.end())
                {
                    FILE_ERROR_MESSAGE(elem, "No loader information was loaded for this element.")
                    return Loader<T>::loadedAssets.begin()->second;
                }

                return Loader<T>::loadingInfos[elem]->loadFromInfos();
            }

            return res->second;
        };

        static Loader<T>& addInfos(const char *filename)
        {
            return addInfos(VulpineTextBuffRef(new VulpineTextBuff(filename)));
        }

        static Loader<T>& addInfos(VulpineTextBuffRef buff)
        {
            bool infoMustBeLoaded = false;
            char *ptr = buff->read();

            if(*ptr == '|')
            {
                ptr = buff->read();
                infoMustBeLoaded = true;
            }

            auto check = loadingInfos.find(ptr);
            if(check != loadingInfos.end()) return *check->second;
        
            assert(!infoMustBeLoaded);

            Loader<T> e;
            e.buff = buff;
            e.values = buff->data + buff->getReadHead();
            e.name = ptr;
            return *(loadingInfos[e.name] = std::make_unique<Loader<T>> (e));
        };

        static Loader<T>& addInfosTextless(const char *filename, const std::string& prefix = "")
        {
            VulpineTextBuffRef autoText(new VulpineTextBuff());

            std::string id = getNameOnlyFromPath(filename) + " : " + prefix + " " + filename + " ; "; 

            autoText->alloc(id.size());
            strcpy(autoText->data, id.c_str());

            return addInfos(autoText);
        };
        
        T& loadFromInfos();
};
