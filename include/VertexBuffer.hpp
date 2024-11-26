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
    friend std::ostream& operator<<(std::ostream&, const VertexAttribute&);

    private :
        uint8  perValuesSize = 0;
        uint8  perVertexSize = 3;
        uint64 vertexCount;
        uint64 bufferSize;

        GLuint handle = 0;
        std::shared_ptr<GLuint> handleRef;
        GLuint location = 0;
        GenericSharedBuffer buffer;

        GLenum type = GL_FLOAT;

    public :

        /*
            TODO : test if the src is correctly copied
        */
        VertexAttribute(GenericSharedBuffer _src, 
                   GLuint _location,
                   uint64 _vertexCount,
                   uint8  _perVertexSize,
                   GLenum _type,
                   bool   _copySource);

        ~VertexAttribute();

        void updateData(GenericSharedBuffer _src, uint64 _vertexCount);

        void genBuffer();
        void sendAllToGPU();
        void setFormat();
        char* getBufferAddr();

        GLuint getLocation() const {return location;};
        GLuint getHandle() const {return handle;};
        GLuint getVertexCount() const {return vertexCount;};

        GLenum getType(){return type;};
        uint8 getPerVertexSize(){return perVertexSize;};
        uint8 getPerValuesSize(){return perValuesSize;};
};

std::ostream& operator<<(std::ostream& os, const VertexAttribute &l);

class VertexAttributeGroup
{
    private : 

        GLuint arrayHandle;
        std::shared_ptr<GLuint> handleRef;

        bool handleCreated = false;

        bool generated = false;

        vec3 AABBmin = vec3(-1e12f);
        vec3 AABBmax = vec3(1e12f);

    public : 

        ~VertexAttributeGroup();

        VertexAttributeGroup& genAABB();
        vec3 getAABBMin();
        vec3 getAABBMax();
        void setAABB(vec3 min, vec3 max){AABBmax = max; AABBmin = min;};

        std::vector<VertexAttribute> attributes;

        // VertexAttributeGroup(const std::vector<VertexAttribute> &newAttributes);
        VertexAttributeGroup(std::vector<VertexAttribute> newAttributes);

        void add(std::vector<VertexAttribute> &newAttributes);
        void add(VertexAttribute newAttribute);

        VertexAttributeGroup& generate();

        void forEach(void (*func)(int, VertexAttribute&));
        void forEach(void (*func)(int, const VertexAttribute&)) const;

        GLuint getHandle() const {return arrayHandle;};
};

#endif