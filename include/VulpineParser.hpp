#pragma once 
#include <memory>
#include <fstream>
#include <glm/glm.hpp>

typedef char uft8;

/*
    Simple buffer to load a full uft8 file and parse Vulpine text based formats
*/
class VulpineTextBuff
{
    private :

        size_t size = 0;
        size_t readhead = 0;

        bool eof = false;
        bool stringMode = false;
        bool commentsMode = false;

        bool clearHeadLoc = true;

        bool readBreakChar();

        std::string source;

    public :
        VulpineTextBuff(const char *filename);
        VulpineTextBuff(){};
        ~VulpineTextBuff();

        void loadFromFile(const char *filename);
        uft8 *data = nullptr;

        size_t getSize(){return size;};
        size_t getReadHead(){return readhead;};
        const std::string getSource(){return source;}; 

        void skipUntilNewElement();

        void setHead(uft8 *ptr);
        void setHead(const uft8 *ptr);

        void alloc(int size);

        uft8* read();
};

typedef std::shared_ptr<VulpineTextBuff> VulpineTextBuffRef;

template<typename T>
T fromStr(const char * ptr);

template<>
int fromStr(const char *ptr);

template<>
float fromStr(const char *ptr);

template<>
glm::vec3 fromStr(const char *ptr);

class VulpineTextOutput
{
    private : 
        
        uft8 *data = nullptr;
        uft8 *readhead = nullptr;
        const uft8 *readheadconst = nullptr;
        int tabLevel = 0;
        size_t size = 1024;

    public : 

        VulpineTextOutput(int _size = 1024);
        ~VulpineTextOutput();

        void Tabulate();

        void Entry();
        void Break();

        void Reference();
        void NewLine();

        void alloc(int _size = 1024);
        void realloc(int _size);

        void write(const char *src, int wsize);

        uft8 *&getReadHead(); 
        const uft8 *&getReadHeadConst(); 

        uft8 *getData(); 

        void saveAs(const char *filename);
};

typedef std::shared_ptr<VulpineTextOutput> VulpineTextOutputRef;


class FastTextParser
{
    public : 

    template<typename T>
    static T read(const char *&buff);

    template<typename T>
    static void write(T data, char *&buff);
};

#define PARSER_READ(T) template<> T FastTextParser::read<T>(const char *&buff)
#define PARSER_WRITE(T) template<> void FastTextParser::write<T>(T data, char *&buff)
#define PARSER_DEFINE(T) PARSER_WRITE(T); PARSER_READ(T);

PARSER_DEFINE(float);
PARSER_DEFINE(unsigned int);
PARSER_DEFINE(int);
PARSER_DEFINE(glm::vec2);
PARSER_DEFINE(glm::vec3);
PARSER_DEFINE(glm::vec4);
PARSER_DEFINE(glm::ivec2);
PARSER_DEFINE(glm::ivec3);
PARSER_DEFINE(glm::ivec4);

PARSER_DEFINE(bool);