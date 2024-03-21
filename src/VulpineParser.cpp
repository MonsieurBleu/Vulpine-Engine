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
}

void VulpineTextBuff::loadFromFile(const char *filename)
{
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

    data = new uft8[size+1];
    file.read((char *)data, size);
    file.close();
    data[size] = '\0';
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

    if(eof) return nullptr;
    
    uft8 *beg = data + readhead;

    while(!readBreakChar() && !eof)
        readhead++;

    if(clearHeadLoc)
        data[readhead] = '\0';

    return beg;
}

void VulpineTextBuff::setHead(uft8 *ptr)
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
