#ifndef CUBE_MAP_HPP
#define CUBE_MAP_HPP

#include <Textures.hpp>
#include <string>

class CubeMap
{
    private : 
        GLuint handle;
        Texture2D faces[6];
        bool generated = false;

    public : 
        /*
            We consider the folder with 6 images : 
                nx.png, ny.png, nz.png, px.png, py.png, pz.png
            all loaded using the stb library
        */
        CubeMap& loadAndGenerate(const std::string& folderPath);
        CubeMap& bind();
};  

#endif