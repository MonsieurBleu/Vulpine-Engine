#include <CubeMap.hpp>
#include <stb/stb_image.h>
#include <Utils.hpp>

CubeMap& CubeMap::loadAndGenerate(const std::string& folderPath)
{
    if(generated) return *this;

    static std::string facesNames[6] = 
    {
        // "nx.png", "ny.png", "nz.png",
        // "px.png", "py.png", "pz.png",
        "px.png", "nx.png", 
        "py.png", "ny.png", 
        "pz.png", "nz.png"
    };

    for(int i = 0; i < 6; i++)
    {
        int height, width, n;
        void* data = stbi_load((folderPath+facesNames[i]).c_str(), &width, &height, &n, 3);

        if(data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );

            free(data);
        }
        else
        {
            std::cerr 
            << TERMINAL_ERROR << "CubeMap::load : stb error, can't load image " 
            << TERMINAL_FILENAME << folderPath+facesNames[i]
            << TERMINAL_ERROR << ". This file either don't exist or the format is not supported.\n"
            << TERMINAL_RESET; 
        }

    }
    
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    generated = true;

    return *this;
}

CubeMap& CubeMap::bind()
{
    glActiveTexture(GL_TEXTURE_BINDING_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, handle);
    return *this;
}

// CubeMap& CubeMap::generate()
// {
//     if(!generated)
//     {
//         glGenTextures(1, &handle);
//         glBindTexture(GL_TEXTURE_CUBE_MAP, handle);

//         for(int i = 0; i < 6; i++)
//         {

//         }

//         generated = true;
//     }
//     return *this;   
// }
