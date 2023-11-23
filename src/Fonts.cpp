#include <Fonts.hpp>
#include <Utils.hpp>

FontUFT8& FontUFT8::readCSV(const std::string filename)
{
    FILE *csv = fopen(filename.c_str(), "r");

    fseek(csv, 0, SEEK_END);
    const uint64 fsize = ftell(csv);
    fseek(csv, 0, SEEK_SET);

    if(csv == nullptr|| fsize == UINT64_MAX)
    {
        std::cout 
        << TERMINAL_ERROR << "Error loading file : "
        << TERMINAL_FILENAME << filename
        << TERMINAL_ERROR << "\n";
        perror("\tFontUFT8::readCSV");
        std::cout << "\n";
        return *this;
    }

    char *data = new char[fsize]{'\0'};
    fread(data, fsize, 1, csv);
    fclose(csv);
    char *reader = data;

    while (reader < data+fsize && reader != (char*)0x1)
    {
        FontCharInfo info;

        sscanf(reader, "%u,%f,%f,%f,%f,%f,%f,%f,%f,%f\n", 
            &info.unicode,
            &info.advance,
            &info.planeLeft,
            &info.planeBottom,
            &info.planeRight,
            &info.planeTop,
            &info.atlasLeft,
            &info.atlasBottom,
            &info.atlasRight,
            &info.atlasTop
        );

        if(info.unicode < 256)
            characters[info.unicode] = info;
    
        // std::cout << reader << " " << info.unicode << "\n";
        reader = strchr(reader, '\n')+1;

    }
    
    delete data;
    return *this;
}

FontUFT8& FontUFT8::setAtlas(Texture2D newAtlas)
{
    atlas = newAtlas;
    return *this;
}

FontCharInfo& FontUFT8::getInfo(int uft8)
{
    return characters[uft8];
}

Texture2D FontUFT8::getAtlas()
{
    return atlas;
}

void SingleStringBatch::batchText()
{
    size_t size = text.size();
    int usedChar = 0;
    if(!size) return;
    depthWrite = false;
    invertFaces = true;

    GenericSharedBuffer positions(new char[sizeof(vec3)*6*size]);
    GenericSharedBuffer uvs(new char[sizeof(vec2)*6*size]);
    vec3 *p = (vec3 *)positions.get();
    vec2 *u = (vec2 *)uvs.get();

    vec2 b = vec2(0);
    vec3 c = vec3(b, 0.f);
    const float charSize = 0.05;

    for(size_t i = 0; i < size; i++)
    {
        if(text[i] == '\n')
        {
            c.x = b.x;
            c.y -= charSize;
            continue;
        }

        size_t id = usedChar*6;
        if(text[i] >= 256) continue;
        const FontCharInfo &info = font->getInfo(text[i]);

        if(text[i] != U' ')
        {
            p[id] = c + vec3(info.planeLeft, info.planeTop, 0)*vec3(charSize, charSize, 0);
            p[id+1] = c + vec3(info.planeRight, info.planeTop, 0)*vec3(charSize, charSize, 0);
            p[id+2] = c + vec3(info.planeLeft, info.planeBottom, 0)*vec3(charSize, charSize, 0);

            p[id+3] = c + vec3(info.planeRight, info.planeBottom, 0)*vec3(charSize, charSize, 0);
            p[id+4] = p[id+2];
            p[id+5] = p[id+1];

            u[id] = vec2(info.atlasLeft, info.atlasTop);
            u[id+1] = vec2(info.atlasRight, info.atlasTop);
            u[id+2] = vec2(info.atlasLeft, info.atlasBottom);

            u[id+3] = vec2(info.atlasRight, info.atlasBottom);
            u[id+4] = u[id+2];
            u[id+5] = u[id+1];

            usedChar ++;
        }

        c.x += charSize*info.advance*1.f;
    }

    if(!vao.get() || !vao->getHandle())
    {
        setVao(
            MeshVao(
                new VertexAttributeGroup({
                    VertexAttribute(positions, 0, usedChar*6, 3, GL_FLOAT, false),
                    VertexAttribute(uvs, 1, usedChar*6, 2, GL_FLOAT, false)
                })
            )    
        );

        vao->generate();
    }
    else
    {
        vao->attributes[0].updateData(positions, usedChar*6);
        vao->attributes[1].updateData(uvs, usedChar*6);
    }
}

void SingleStringBatch::setFont(FontRef newFont)
{
    font = newFont;
    removeMap(0);
    setMap(font->getAtlas(), 0);
}

void SingleStringBatch::genVao()
{


};