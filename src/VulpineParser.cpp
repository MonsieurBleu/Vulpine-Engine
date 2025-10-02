#include <string.h>
#include <limits>

#include <VulpineParser.hpp>
#include <Utils.hpp>

VulpineTextBuff::VulpineTextBuff(const char *filename)
{
    loadFromFile(filename);
}

VulpineTextBuff::~VulpineTextBuff()
{
    if(data) delete [] data;
    if(originalData) delete [] originalData;
}

void VulpineTextBuff::loadFromFile(const char *filename)
{
    source = std::string(filename);
    std::ifstream file(filename, std::ios::in | std::ios::binary);

    if(!file.good())
    {
        FILE_ERROR_MESSAGE(filename, "Can't open file. The buffer will be empty.")
        return;
    }

    /* 
        Get the full file size from fstream.
            see : https://stackoverflow.com/a/37808094
        
        TODO : add this to an utility function
    */
    file.ignore( std::numeric_limits<std::streamsize>::max() );
    size = file.gcount();
    file.clear();
    file.seekg( 0, std::ios_base::beg );

    // data = new uft8[size+1];
    alloc(size);
    file.read((char *)data, size);
    file.close();
    data[size] = '\0';

    memcpy(originalData, data, size);
}

void VulpineTextBuff::alloc(int size)
{
    this->size = size;
    data = new uft8[size+1];
    data[size] = '\0';

    originalData = new uft8[size+1];
    originalData[size] = '\0';
}

void VulpineTextBuff::resetData()
{
    memcpy(data, originalData, size);
    eof = false;
    stringMode = false;
    commentsMode = false;
    clearHeadLoc = true;
    readhead = 0;
}

bool VulpineTextBuff::readBreakChar()
{
    if(readhead >= size)
    {
        eof = true;
        return false;
    } 

    clearHeadLoc = true;
    
    if(stringMode)
    {
        stringMode = data[readhead] != '"';
        if(!stringMode)
        {
            data[readhead] = '\0';
            readhead++;
            data[readhead] = '"';
            clearHeadLoc = false;
            readhead++;
        }

        return !stringMode;
    }

    if(commentsMode)
    {
        commentsMode = data[readhead] != '*';
        return true;
    }

    switch(data[readhead])
    {
        case '\0' : 
        case '\n' : 
        case '\t' : 
        case '\r' :
        case ' ' :
            return true;
        break;

        case '*' :
            commentsMode = true;
            return true;
        break;

        case '"' :
            stringMode = true;
            return true;
        break;

        default :
            return false;
        break;
    }
}

uft8* VulpineTextBuff::read()
{
    
    while(readBreakChar()) readhead++;

    if(eof) return new uft8('\0');
    
    uft8 *beg = data + readhead;

    while(!readBreakChar() && !eof)
        readhead++;

    if(clearHeadLoc)
        data[readhead] = '\0';

    return beg;
}

/* TODO : complete*/
void VulpineTextBuff::skipUntilNewElement()
{
    // int n = 0;
    // int b = 0;

    // while (b < n)
    // {
    //     switch (*read())
    //     {
    //         case ';': b++; break;
    //         case ':': n++; break;
    //         default: break;
    //     }
    // }
}



void VulpineTextBuff::setHead(uft8 *ptr)
{
    assert(ptr != nullptr);
    readhead = ptr - data;
}

void VulpineTextBuff::setHead(const uft8 *ptr)
{
    assert(ptr != nullptr);
    readhead = ptr - data;
}

template<>
int fromStr(const char *ptr)
{
    return atoi(ptr);
}

template<>
float fromStr(const char *ptr)
{
    return atof(ptr);
}

template<>
glm::vec3 fromStr(const char *ptr)
{
    // return glm::vec3(atof(ptr), atof(ptr+9), atof(ptr+18));
    glm::vec3 v;
    sscanf(ptr, "%f %f %f", &v.x, &v.y, &v.z);
    return v;
}

template<>
glm::quat fromStr(const char *ptr)
{
    // return glm::vec3(atof(ptr), atof(ptr+9), atof(ptr+18));
    glm::quat v;
    sscanf(ptr, "%f %f %f %f", &v.w, &v.x, &v.y, &v.z);
    return v;
}


void VulpineTextOutput::alloc(int _size)
{
    size = _size;
    data = new uft8[size];
    readhead = data;
}

void VulpineTextOutput::realloc(int _size)
{
    size_t tmpSize = size;
    uft8* tmpData = data;
    uft8* tmpHead = readhead;

    alloc(_size);

    memcpy(data, tmpData, tmpSize);
    delete [] tmpData;

    readhead += (tmpHead - tmpData);
}

VulpineTextOutput::VulpineTextOutput(int _size){alloc(_size);}
VulpineTextOutput::~VulpineTextOutput(){delete [] data;}

void VulpineTextOutput::Tabulate()
{
    tabLevel ++;
}

void VulpineTextOutput::Entry()
{
    *(readhead++) = '\n';
    for(int i = 0; i < tabLevel; i++)
        *(readhead++) = '\t';
    *(readhead++) = ':';
    *(readhead++) = ' ';
}

void VulpineTextOutput::Break()
{
    *(readhead++) = '\n';
    for(int i = 0; i < tabLevel; i++)
        *(readhead++) = '\t';
    *(readhead++) = ';';
    *(readhead) = '\0';
    tabLevel--;
}


void VulpineTextOutput::Reference()
{
    *(readhead++) = '|';
}

void VulpineTextOutput::NewLine()
{
    *(readhead++) = '\n';
}

int VulpineTextOutput::getTabLevel()const
{
    return tabLevel;
}

void VulpineTextOutput::write(const char *src, int wsize)
{
    if((data + size)-readhead < glm::max(256, wsize)) 
        realloc(size*2);

    memcpy(readhead, src, wsize);

    readhead += wsize;
}

void VulpineTextOutput::saveAs(const char *filename)
{
    std::ofstream file(filename, std::ios::out);

    file.write(data, readhead-data);

    file.close();
}

uft8       *& VulpineTextOutput::getReadHead(){return readhead;} 
const uft8 *& VulpineTextOutput::getReadHeadConst(){return readheadconst;}
uft8 *VulpineTextOutput::getData(){return data;}

#define PARSER_END_CHAR         '\0'
#define PARSER_SEPARATOR_CHAR   ' '

PARSER_WRITE(bool)
{
    int c = 0;
    buff[c++] = data ? 't' : 'f';
    buff[c] = PARSER_END_CHAR;
    buff += c;
}

PARSER_READ(bool)
{
    return *(buff++) == 't';
}

unsigned int ExtractDecimal(unsigned int &i, unsigned int dec)
{
    int res = i/dec;
    i %= dec;
    return res;
}

int ExtractDecimal(float &i, float dec)
{
    int res = i/dec;
    i = fmodf(i, dec);
    return res;
}

PARSER_WRITE(float)
{
    int c = 0;
    int e = 0;
    unsigned int precision = 1000000u;

    if(data < 0)
    {
        buff[c++] = '-';
        data = -data;
    }
    else
        buff[c++] = '+';

    if(data != 0.f && data >= 10.f)
        do{data *= 0.1f; e++;} while(data > 10.f);

    else if(data != 0.f && data < 1.f)
        do{data *= 10.f; e--;} while(data < 1.f);


    unsigned int datai = (double)(data) * precision;


    buff[c++] = '0' + ExtractDecimal(datai, precision);
    precision /= 10;
    buff[c++] = '.';

    for(; precision > 10; precision /= 10) 
        buff[c++] = '0' + ExtractDecimal(datai, precision);

    buff[c++] = '0' + datai/10u;
    buff[c++] = '0' + datai%10u;

    buff[c++] = 'e';
    buff[c++] = e >= 0 ? '+' : '-';
    uint64_t ue = e > 0 ? e : -e;
    buff[c++] = '0' + ue/10;
    buff[c++] = '0' + ue%10;

    buff[c] = PARSER_END_CHAR;
    buff += c;
}

PARSER_READ(float)
{
    #define Pcmpt10p(n) 1e##n##0f, 1e##n##1f, 1e##n##2f, 1e##n##3f, 1e##n##4f, 1e##n##5f, 1e##n##6f, 1e##n##7f, 1e##n##8f, 1e##n##9f 
    #define Pcmpt10n(n) 1e-##n##0f, 1e-##n##1f, 1e-##n##2f, 1e-##n##3f, 1e-##n##4f, 1e-##n##5f, 1e-##n##6f, 1e-##n##7f, 1e-##n##8f, 1e-##n##9f 

    static float powvaluesP[] = {Pcmpt10p(0), Pcmpt10p(1), Pcmpt10p(2)};
    static float powvaluesN[] = {Pcmpt10n(0), Pcmpt10n(1), Pcmpt10n(2)};

    int c = 0;
    bool invert = buff[c++] == '-';

    float data = (buff[c++]-'0');
    c++;

    float dec = 0.1;
    for(int i = 0; i < 6; dec *= 0.1, i++)
        data += (buff[c++]-'0')*dec;
    
    c++;
    bool s = buff[c++] == '+';
    int e = (buff[c++]-'0')*10;
    e += (buff[c++]-'0');

    if(invert) data = -data;

    buff += c;

    if(s)
        return data*powvaluesP[e];
    else
        return data*powvaluesN[e];
}

PARSER_READ(unsigned int)
{
    int c = 0;

    bool isdigit = true;
    while(isdigit)
    {
        switch (buff[c])
        {
            case '0' : case '1' : case '2' : case '3' : case '4' : 
            case '5' : case '6' : case '7' : case '8' : case '9' :
            c++;
            break;
            
            default: isdigit = false; break;
        }
    }

    int c2 = c;
    unsigned int val = 0;
    unsigned int e = 1;

    for(c--; c >= 0; c--)
    {
        val += (buff[c]-'0')*e;
        e *= 10;
    }

    buff += c2;

    return val;
}

PARSER_READ(int)
{
    if(buff[0] == '-')
        return -(int)FastTextParser::read<unsigned int>(++buff);
    else
        return (int)FastTextParser::read<unsigned int>(++buff);
}

PARSER_WRITE(unsigned int)
{
    static const unsigned int powerof10[9] = {1000000000u, 100000000u, 10000000u, 1000000u, 100000u, 10000u, 1000u, 100u, 10u};

    int c = 0;
    for(unsigned int e = 0; e < 9; e++)
    {
        unsigned int digit = data/powerof10[e];

        if(c || digit)
            buff[c++] = '0'+digit;

        data %= powerof10[e];
    }

    buff[c++] = '0'+data;

    buff[c] = PARSER_END_CHAR;
    buff += c;
}

PARSER_WRITE(int)
{
    if(data < 0)
    {
        buff[0] = '-';
        data = -data;
    }
    else
        buff[0] = '+';
    
    FastTextParser::write<unsigned int>(data, ++buff);
}

PARSER_EQUALS(int32_t, int16_t);
PARSER_EQUALS(int32_t, int8_t);
PARSER_EQUALS(uint32_t, uint16_t);
PARSER_EQUALS(uint32_t, uint8_t);

PARSER_READ(glm::vec2)
{
    float x = FastTextParser::read<float>(buff);
    float y = FastTextParser::read<float>(++buff);
    return glm::vec2(x, y);
}

PARSER_READ(glm::vec3)
{
    float x = FastTextParser::read<float>(buff);
    float y = FastTextParser::read<float>(++buff);
    float z = FastTextParser::read<float>(++buff);
    return glm::vec3(x, y, z);
}

PARSER_READ(glm::vec4)
{
    float x = FastTextParser::read<float>(buff);
    float y = FastTextParser::read<float>(++buff);
    float z = FastTextParser::read<float>(++buff);
    float w = FastTextParser::read<float>(++buff);
    return glm::vec4(x, y, z, w);
}

PARSER_READ(glm::quat)
{
    float w = FastTextParser::read<float>(buff);
    float x = FastTextParser::read<float>(++buff);
    float y = FastTextParser::read<float>(++buff);
    float z = FastTextParser::read<float>(++buff);
    return glm::quat(w, x, y, z);
}

PARSER_READ(glm::mat4)
{
    glm::mat4 m;
    float *f = (float *)&m;

    for(int i = 0; i < 16; i++, buff++)
        f[i] =  FastTextParser::read<float>(buff);

    return m;
}

PARSER_READ(glm::ivec2)
{
    int x = FastTextParser::read<int>(buff);
    int y = FastTextParser::read<int>(++buff);
    return glm::ivec2(x, y);
}

PARSER_READ(glm::ivec3)
{
    int x = FastTextParser::read<int>(buff);
    int y = FastTextParser::read<int>(++buff);
    int z = FastTextParser::read<int>(++buff);
    return glm::ivec3(x, y, z);
}

PARSER_READ(glm::ivec4)
{
    int x = FastTextParser::read<int>(buff);
    int y = FastTextParser::read<int>(++buff);
    int z = FastTextParser::read<int>(++buff);
    int w = FastTextParser::read<int>(++buff);
    return glm::ivec4(x, y, z, w);
}

PARSER_WRITE(glm::vec2)
{
    FastTextParser::write<float>(data.x, buff);
    *(buff++) = PARSER_SEPARATOR_CHAR;
    FastTextParser::write<float>(data.y, buff);
}

PARSER_WRITE(glm::vec3)
{
    FastTextParser::write<float>(data.x, buff);
    *(buff++) = PARSER_SEPARATOR_CHAR;
    FastTextParser::write<float>(data.y, buff);
    *(buff++) = PARSER_SEPARATOR_CHAR;
    FastTextParser::write<float>(data.z, buff);
}

PARSER_WRITE(glm::vec4)
{
    FastTextParser::write<float>(data.x, buff);
    *(buff++) = PARSER_SEPARATOR_CHAR;
    FastTextParser::write<float>(data.y, buff);
    *(buff++) = PARSER_SEPARATOR_CHAR;
    FastTextParser::write<float>(data.z, buff);
    *(buff++) = PARSER_SEPARATOR_CHAR;
    FastTextParser::write<float>(data.w, buff);
}

// template<> void FastTextParser::write<glm::qua<float, glm::packed_highp>>(glm::qua<float, glm::packed_highp> data, char *&buff)
PARSER_WRITE(glm::quat)
{
    FastTextParser::write<float>(data.w, buff);
    *(buff++) = PARSER_SEPARATOR_CHAR;
    FastTextParser::write<float>(data.x, buff);
    *(buff++) = PARSER_SEPARATOR_CHAR;
    FastTextParser::write<float>(data.y, buff);
    *(buff++) = PARSER_SEPARATOR_CHAR;
    FastTextParser::write<float>(data.z, buff);
}

PARSER_WRITE(glm::mat4)
{
    const float *f = (float *)&data;
    for(int i = 0; i < 16; i++)
    {
        FastTextParser::write<float>(f[i], buff);
        *(buff++) = PARSER_SEPARATOR_CHAR;
    }
}


PARSER_WRITE(glm::ivec2)
{
    FastTextParser::write<int>(data.x, buff);
    *(buff++) = PARSER_SEPARATOR_CHAR;
    FastTextParser::write<int>(data.y, buff);
}

PARSER_WRITE(glm::ivec3)
{
    FastTextParser::write<int>(data.x, buff);
    *(buff++) = PARSER_SEPARATOR_CHAR;
    FastTextParser::write<int>(data.y, buff);
    *(buff++) = PARSER_SEPARATOR_CHAR;
    FastTextParser::write<int>(data.z, buff);
}

PARSER_WRITE(glm::ivec4)
{
    FastTextParser::write<int>(data.x, buff);
    *(buff++) = PARSER_SEPARATOR_CHAR;
    FastTextParser::write<int>(data.y, buff);
    *(buff++) = PARSER_SEPARATOR_CHAR;
    FastTextParser::write<int>(data.z, buff);
    *(buff++) = PARSER_SEPARATOR_CHAR;
    FastTextParser::write<int>(data.w, buff);
}
