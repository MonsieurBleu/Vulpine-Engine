#ifndef VERTEX_BUFFER_HPP
#define VERTEX_BUFFER_HPP

#include <vector>
#include <string>
#include <memory>

#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

using namespace glm;

#define MAX_PER_VERTEX_SIZE 5

class VertexAttribute
{
    private :
        std::string name;

        uint8  perValuesSize = 0;
        uint8  perVertexSize = 3;
        uint64 vertexCount;
        uint64 bufferSize;

        GLuint handle = 0;
        GLuint location = 0;
        std::shared_ptr<void> buffer;

        GLenum type = GL_FLOAT;

    public :

        VertexAttribute();

        /*
            TODO : test if the src is correctly copied
        */
        int create(std::shared_ptr<void> _src, 
                   GLuint _location,
                   uint64 _vertexCount,
                   uint8  _perVertexSize,
                   GLenum _type,
                   bool   _copySource);

        void genBuffer();
        void sendAllToGPU();
        void bindVertexBuffer();


        GLuint getLocation() const {return location;};
};

class VertexAttributeGroup
{
    private : 

        std::vector<VertexAttribute> attributes;

        GLuint arrayHandle;

        bool handleCreated = false;

    public : 

        void add(std::vector<VertexAttribute> &newAttributes);
        void add(VertexAttribute newAttribute);

        void generate();

        void forEach(void (*func)(int, VertexAttribute&));
        void forEach(void (*func)(int, const VertexAttribute&)) const;
};

#endif