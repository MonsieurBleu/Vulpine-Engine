#ifndef FONTS_MSDF_HPP
#define FONTS_MSDF_HPP

#include <iostream>
#include <Mesh.hpp>
#include <memory>

typedef unsigned int UFT8;

/*
    All of this module is based on :
        https://github.com/Chlumsky/msdf-atlas-gen#atlas-types
*/

struct FontCharInfo
{
    UFT8 unicode = 0;
    float advance = 0.f;

    float planeLeft = 0.f;
    float planeBottom = 0.f;
    float planeRight = 0.f;
    float planeTop = 0.f;

    float atlasLeft = 0.f;
    float atlasBottom = 0.f;
    float atlasRight = 0.f;
    float atlasTop = 0.f;
};

class FontUFT8
{
    private : 
        FontCharInfo characters[256];
        Texture2D atlas;

    public :
        FontUFT8& readCSV(const std::string filename);
        FontUFT8& setAtlas(Texture2D newAtlas);
        FontCharInfo &getInfo(unsigned char uft8);
};

typedef std::shared_ptr<FontUFT8> FontRef;

class SingleStringBatch : public MeshModel3D
{
    private : 
        

    public : 
        FontRef font;
        std::u32string text;
        void batchText();
        void genVao(); // maybe useless
};


#endif