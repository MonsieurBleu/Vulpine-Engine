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

typedef std::shared_ptr<char []> GenericSharedBuffer;

class VertexAttribute
{
    private :
        uint8  perValuesSize = 0;
        uint8  perVertexSize = 3;
        uint64 vertexCount;
        uint64 bufferSize;

        GLuint handle = 0;
        GLuint location = 0;
        std::shared_ptr<char []> buffer;

        GLenum type = GL_FLOAT;

    public :

        /*
            TODO : test if the src is correctly copied
        */
        VertexAttribute(std::shared_ptr<char []> _src, 
                   GLuint _location,
                   uint64 _vertexCount,
                   uint8  _perVertexSize,
                   GLenum _type,
                   bool   _copySource);

        void genBuffer();
        void sendAllToGPU();
        void setFormat();

        GLuint getLocation() const {return location;};
        GLuint getHandle() const {return handle;};
        GLuint getVertexCount() const {return vertexCount;};
};

class VertexAttributeGroup
{
    private : 

        GLuint arrayHandle;

        bool handleCreated = false;

    public : 

        std::vector<VertexAttribute> attributes;

        VertexAttributeGroup(std::vector<VertexAttribute> newAttributes);
        VertexAttributeGroup(std::vector<VertexAttribute> &newAttributes);

        void add(std::vector<VertexAttribute> &newAttributes);
        void add(VertexAttribute newAttribute);

        void generate();

        void forEach(void (*func)(int, VertexAttribute&));
        void forEach(void (*func)(int, const VertexAttribute&)) const;

        GLuint getHandle() const {return arrayHandle;};
};

#endif