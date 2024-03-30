#ifndef FONTS_MSDF_HPP
#define FONTS_MSDF_HPP

#include <iostream>
#include <Mesh.hpp>
#include <memory>

typedef unsigned int UFT8;

#define CHARSTYLE_BOLD 0b1
#define CHARSTYLE_ITALIC 0b10

#define STYLE_ID_BOLD 0x0000002a0000002aU 

/*
    All of this module is based on :
        https://github.com/Chlumsky/msdf-atlas-gen#atlas-types
*/

#include <codecvt>
#include <iomanip>
extern std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> UFTconvert;

typedef std::basic_ostringstream<char32_t> UFT32Stream;

UFT32Stream& operator<<(UFT32Stream& os, const float f);
UFT32Stream& operator<<(UFT32Stream& os, const int i);
UFT32Stream& operator<<(UFT32Stream& os, const std::string& str);

bool u32strtof(std::u32string &str, float &f);
int u32strtoi(std::u32string &str, int base = 10);
bool u32chartoi(char32_t c, int &i);
bool u32strtocolorHTML(std::u32string &str, vec3 &v);
std::u32string rgbtou32str(vec3 v);

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
        FontCharInfo &getInfo(int uft8);
        
        Texture2D getAtlas();
};

typedef std::shared_ptr<FontUFT8> FontRef;

enum StringAlignement
{
    TO_LEFT, CENTERED
};

class SingleStringBatch : public MeshModel3D
{
    private : 
        FontRef font;
        vec2 textSize;
        int currentTab = 0;

    public : 
        const float charSize = 0.025;
        void setFont(FontRef newFont);
        std::u32string text;

        StringAlignement align = TO_LEFT;

        /*
            TODO : add inteligent buffer overwrite (the whole thing is not re-allocated if the place is arleady here)
        */
        void batchText();
        vec2 getSize();

        void setCurrentTab(int id);
};

typedef std::shared_ptr<SingleStringBatch> SingleStringBatchRef;

#endif