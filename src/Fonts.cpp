#include <Fonts.hpp>
#include <Utils.hpp>

UFT32Stream &operator<<(UFT32Stream &os, const float f)
{
    std::stringstream s;
    s << std::setprecision(8) << f;
    os << UFTconvert.from_bytes(s.str());
    return os;
}

UFT32Stream &operator<<(UFT32Stream &os, const std::string &str)
{
    os << UFTconvert.from_bytes(str);
    return os;
}

UFT32Stream &operator<<(UFT32Stream &os, const int i)
{
    os << UFTconvert.from_bytes(std::to_string(i));
    return os;
}

UFT32Stream &operator<<(UFT32Stream &os, const short i)
{
    os << UFTconvert.from_bytes(std::to_string(i));
    return os;
}

bool u32strtof(std::u32string &str, float &f)
{
    errno = 0;
    char *endptr = NULL;
    std::string s = UFTconvert.to_bytes(str);

    if (!strcmp("nan", s.c_str()))
        return false;

    f = strtof(s.c_str(), &endptr);
    return !(errno || s.c_str() == endptr);
}

UFT32Stream& operator<<(UFT32Stream& os, const safeBoolOverload b)
{
    if(b.val) 
        os << U"True";
    else 
        os << U"False";
    return os;
}

bool u32chartoi(char32_t c, int &i)
{
    switch (c)
    {
    case U'0':
        i = 0;
        break;
    case U'1':
        i = 1;
        break;
    case U'2':
        i = 2;
        break;
    case U'3':
        i = 3;
        break;
    case U'4':
        i = 4;
        break;
    case U'5':
        i = 5;
        break;
    case U'6':
        i = 6;
        break;
    case U'7':
        i = 7;
        break;
    case U'8':
        i = 8;
        break;
    case U'9':
        i = 9;
        break;
    case U'A':
    case U'a':
        i = 10;
        break;
    case U'B':
    case U'b':
        i = 11;
        break;
    case U'C':
    case U'c':
        i = 12;
        break;
    case U'D':
    case U'd':
        i = 13;
        break;
    case U'E':
    case U'e':
        i = 14;
        break;
    case U'F':
    case U'f':
        i = 15;
        break;

    default:
        return false;
        break;
    }

    return true;
}

int u32strtoi(std::u32string &str, int base)
{
    errno = 0;
    char *endptr = NULL;
    std::string s = UFTconvert.to_bytes(str);
    int i = strtol(s.c_str(), &endptr, base);
    return errno || s.c_str() == endptr ? INT_MAX : i;
}

bool u32strtocolorHTML(std::u32string &str, vec3 &v)
{
    int i = 0;
    if (str[0] == U'#')
        i++;

    int size = str.size();

    if (size - i != 6)
        return false;

    ivec3 rgb;
    int tmp;
    bool validInput = true;

    for (int j = 0; j < 3; j++)
    {
        validInput &= u32chartoi(str[i + j * 2], tmp);
        validInput &= u32chartoi(str[i + j * 2 + 1], rgb[j]);
        rgb[j] += tmp << 4;
        std::cout << rgb[j] << "\t";
    }
    std::cout << "\n";

    if (!validInput)
        return false;

    for (int j = 0; j < 3; j++)
        v[j] = (float)rgb[j] / 255.f;

    return true;
}

std::u32string rgbtou32str(vec3 v)
{
    std::stringstream s;

    if (v.r < 0.f || v.r > 1.f || v.g < 0.f || v.g > 1.f || v.b < 0.f || v.b > 1.f)
        s << "Invalid";
    else
    {
        ivec3 rgb = round(v * vec3(255.0));
        s << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << rgb.r;
        s << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << rgb.g;
        s << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << rgb.b;
    }

    std::u32string ret = UFTconvert.from_bytes(s.str());

    return ret;
}

FontUFT8 &FontUFT8::readCSV(const std::string filename)
{
    FILE *csv = fopen(filename.c_str(), "r");

    fseek(csv, 0, SEEK_END);
    const uint64 fsize = ftell(csv);
    fseek(csv, 0, SEEK_SET);

    if (csv == nullptr || fsize == UINT64_MAX)
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

    while (reader < data + fsize && reader != (char *)0x1)
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
               &info.atlasTop);

        info.planeBottom -= 1.0;
        info.planeTop -= 1.0;

        if (info.unicode < 256)
            characters[info.unicode] = info;

        // std::cout << reader << " " << info.unicode << "\n";
        reader = strchr(reader, '\n') + 1;
    }

    delete[] data;
    return *this;
}

FontUFT8 &FontUFT8::setAtlas(Texture2D newAtlas)
{
    atlas = newAtlas;
    return *this;
}

FontCharInfo &FontUFT8::getInfo(int uft8)
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
    if (!size)
        return;

    depthWrite = false;
    state.frustumCulled = false;

    GenericSharedBuffer positions(new char[sizeof(vec3) * 6 * size]);
    GenericSharedBuffer uvs(new char[sizeof(vec2) * 6 * size]);
    GenericSharedBuffer style(new char[sizeof(uint) * 6 * size]);
    vec3 *p = (vec3 *)positions.get();
    vec2 *u = (vec2 *)uvs.get();
    uint *s = (uint *)style.get();

    vec2 b = vec2(0);
    vec3 c = vec3(b, 0.f);
    uint currentStyle = 0;

    textSize = vec2(0);

    const float tabulationSize = 16.f * charSize;
    const float smallTabulationSize = 4.f * charSize;

    for (size_t i = 0; i < size; i++)
    {
    
        switch (*(uint64*)(&text[i]))
        {
            case STYLE_ID_BOLD : 
                currentStyle ^= CHARSTYLE_BOLD; 
                i++;
                continue;
                break;

            default:break;
        }

        switch (text[i])
        {
            case U'\n':
                c.x = b.x;
                c.y -= charSize;
                continue;
                break;

            case U'\t':
                c.x = (c.x - mod(c.x, tabulationSize)) + tabulationSize;
                continue;
                break;

            case U'\f':
                c.x = (c.x - mod(c.x, smallTabulationSize)) + smallTabulationSize;
                continue;
                break;
            
            case U'*':
                currentStyle ^= CHARSTYLE_ITALIC;
                continue;
                break;

            default:break;
        }

        size_t id = usedChar * 6;

        if (text[i] >= 256)
            continue;

        const FontCharInfo &info = font->getInfo(text[i]);

        if (text[i] != U' ')
        {
            p[id] = c + vec3(info.planeLeft, info.planeTop, 0) * vec3(charSize, charSize, 0);
            p[id + 1] = c + vec3(info.planeRight, info.planeTop, 0) * vec3(charSize, charSize, 0);
            p[id + 2] = c + vec3(info.planeLeft, info.planeBottom, 0) * vec3(charSize, charSize, 0);

            p[id + 3] = c + vec3(info.planeRight, info.planeBottom, 0) * vec3(charSize, charSize, 0);
            p[id + 4] = p[id + 2];
            p[id + 5] = p[id + 1];

            u[id] = vec2(info.atlasLeft, info.atlasTop);
            u[id + 1] = vec2(info.atlasRight, info.atlasTop);
            u[id + 2] = vec2(info.atlasLeft, info.atlasBottom);

            u[id + 3] = vec2(info.atlasRight, info.atlasBottom);
            u[id + 4] = u[id + 2];
            u[id + 5] = u[id + 1];

            s[id] = s[id+1] = s[id+2] = s[id+3] = s[id+4] = s[id+5] = currentStyle;

            c.x += charSize * info.advance;
            textSize = max(textSize, abs(vec2(c.x, info.planeBottom * charSize)));

            usedChar++;
        }
        else
            c.x += charSize * info.advance;
    }

    textSize = vec2(textSize.x, -c.y + textSize.y);

    if(align == CENTERED)
    {
        for(int i = 0; i < usedChar*6; i++)
        {
            p[i] -= vec3(c.x*0.5, 0, 0);
        }
    }

    if (!vao.get() || !vao->getHandle())
    {
        setVao(
            MeshVao(
                new VertexAttributeGroup({
                    VertexAttribute(positions, 0, usedChar * 6, 3, GL_FLOAT, false),
                    VertexAttribute(uvs, 1, usedChar * 6, 2, GL_FLOAT, false),
                    VertexAttribute(style, 2, usedChar * 6, 1, GL_UNSIGNED_INT, false)
        })));

        vao->generate();
    }
    else
    {
        vao->attributes[0].updateData(positions, usedChar * 6);
        vao->attributes[1].updateData(uvs, usedChar * 6);
        vao->attributes[2].updateData(style, usedChar * 6);
    }
}

void SingleStringBatch::setFont(FontRef newFont)
{
    font = newFont;
    removeMap(0);
    setMap(font->getAtlas(), 0);
}

vec2 SingleStringBatch::getSize()
{
    return textSize;
}
