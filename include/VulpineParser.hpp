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

    public :
        VulpineTextBuff(const char *filename);
        ~VulpineTextBuff();

        void loadFromFile(const char *filename);
        uft8 *data = nullptr;

        size_t getSize(){return size;};
        size_t getReadHead(){return readhead;};

        void setHead(uft8 *ptr);

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