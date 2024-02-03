#include <iostream>
#include <VertexBuffer.hpp>
#include <glm/gtx/string_cast.hpp>
#include <string.h>

using namespace glm;

std::ostream &operator<<(std::ostream &os, const VertexAttribute &l)
{
    switch (l.type)
    {
    case GL_FLOAT:
        switch (l.perVertexSize)
        {
        case 4:
        {
            vec4 *d = (vec4 *)l.buffer.get();
            for (uint64 i = 0; i < l.vertexCount; i++)
                os << to_string(d[i]) << "\n";
        }
        break;

        case 3:
        {
            vec3 *d = (vec3 *)l.buffer.get();
            for (uint64 i = 0; i < l.vertexCount; i++)
                os << to_string(d[i]) << "\n";
        }
        break;

        case 2:
        {
            vec2 *d = (vec2 *)l.buffer.get();
            for (uint64 i = 0; i < l.vertexCount; i++)
                os << to_string(d[i]) << "\n";
        }
        break;

        case 1:
        {
            vec1 *d = (vec1 *)l.buffer.get();
            for (uint64 i = 0; i < l.vertexCount; i++)
                os << to_string(d[i]) << "\n";
        }
        break;

        default:
            break;
        }
        break;

    default:
        break;
    }

    return os;
}

VertexAttribute::VertexAttribute(GenericSharedBuffer _src,
                                 GLuint _location,
                                 uint64 _vertexCount,
                                 uint8 _perVertexSize = 3,
                                 GLenum _type = GL_FLOAT,
                                 bool _copySource = false)
{
    if (perVertexSize == 0 || perVertexSize > MAX_PER_VERTEX_SIZE)
        return;

    location = _location;
    vertexCount = _vertexCount;
    perVertexSize = _perVertexSize;
    type = _type;

    switch (type)
    {
    case GL_BYTE:
    case GL_UNSIGNED_BYTE:
        perValuesSize = 1;
        break;

    case GL_HALF_FLOAT:
    case GL_SHORT:
    case GL_UNSIGNED_SHORT:
        perValuesSize = 2;
        break;

    case GL_FLOAT:
    case GL_INT:
    case GL_UNSIGNED_INT:
    case GL_FIXED:
        perValuesSize = 4;
        break;

    case GL_DOUBLE:
        perValuesSize = 8;
        break;

    default:
        perValuesSize = 0;
        break;
    }

    bufferSize = perValuesSize * perVertexSize * vertexCount;

    if (_copySource)
    {
        buffer = std::shared_ptr<char[]>(new char[bufferSize]);
        memcpy((void *)buffer.get(), (void *)_src.get(), bufferSize);
    }
    else
        buffer = _src;
}

void VertexAttribute::updateData(GenericSharedBuffer _src, uint64 _vertexCount)
{
    buffer = _src;
    vertexCount = _vertexCount;
    bufferSize = perValuesSize * perVertexSize * vertexCount;
    sendAllToGPU();
}

void VertexAttribute::genBuffer()
{
    if(handle) return;
    glGenBuffers(1, &handle);
    handleRef = std::make_shared<GLuint>(handle);
}

VertexAttribute::~VertexAttribute()
{
    if(handle && handleRef.use_count() == 1)
    {
        glDeleteBuffers(1, &handle);
    }
}

void VertexAttribute::sendAllToGPU()
{
    glBindBuffer(GL_ARRAY_BUFFER, handle);
    glBufferData(GL_ARRAY_BUFFER, bufferSize, buffer.get(), GL_STATIC_DRAW);
}

void VertexAttribute::setFormat()
{
    glBindVertexBuffer(location, handle, 0, perValuesSize * perVertexSize);
    
    switch (type)
    {
    case GL_FLOAT :
        glVertexAttribFormat(location, perVertexSize, type, GL_FALSE, 0);
        break;
    
    case GL_UNSIGNED_INT :
    case GL_INT :
        glVertexAttribIFormat(location, perVertexSize, type, 0);
        break;

    default:
        break;
    }

    
}

char *VertexAttribute::getBufferAddr() { return buffer.get(); };

VertexAttributeGroup::VertexAttributeGroup(std::vector<VertexAttribute> &newAttributes)
{
    add(newAttributes);
}

VertexAttributeGroup::VertexAttributeGroup(std::vector<VertexAttribute> newAttributes)
{
    add(newAttributes);
}

void VertexAttributeGroup::forEach(void (*func)(int, VertexAttribute &))
{
    for (uint64 i = 0; i < attributes.size(); i++)
    {
        func(i, attributes[i]);
    }
}

void VertexAttributeGroup::forEach(void (*func)(int, const VertexAttribute &)) const
{
    for (uint64 i = 0; i != attributes.size(); i++)
    {
        func(i, attributes[i]);
    }
}

void VertexAttributeGroup::add(std::vector<VertexAttribute> &newAttributes)
{
    for (auto i = newAttributes.begin(); i != newAttributes.end(); i++)
    {
        attributes.push_back(*i);
    }
}

void VertexAttributeGroup::add(VertexAttribute newAttribute)
{
    attributes.push_back(newAttribute);
}

VertexAttributeGroup &VertexAttributeGroup::generate()
{
    if (generated)
        return *this;

    forEach([](int i, VertexAttribute &attribute){
        attribute.genBuffer();
        attribute.sendAllToGPU();
    });

    glGenVertexArrays(1, &arrayHandle);
    glBindVertexArray(arrayHandle);

    handleRef = std::make_shared<GLuint>(arrayHandle);

    forEach([](int i, VertexAttribute &attribute){
        glEnableVertexAttribArray(i);
        attribute.setFormat();
        glVertexAttribBinding(i, attribute.getLocation());
    });

    genAABB();

    generated = true;

    return *this;
};

VertexAttributeGroup::~VertexAttributeGroup()
{
    if(arrayHandle && handleRef.use_count() == 1)
    {
        glDeleteVertexArrays(1, &arrayHandle);
    }
}

VertexAttributeGroup &VertexAttributeGroup::genAABB()
{
    if (!attributes.size())
        return *this;

    vec3 *b = (vec3 *)attributes[0].getBufferAddr();
    int size = attributes[0].getVertexCount();

    AABBmax = vec3(-1E12);
    AABBmin = vec3(1E12);

    for (int i = 0; i < size; i++)
    {
        AABBmax = max(AABBmax, b[i]);
        AABBmin = min(AABBmin, b[i]);
    }

    return *this;
}

vec3 VertexAttributeGroup::getAABBMin()
{
    return AABBmin;
}

vec3 VertexAttributeGroup::getAABBMax()
{
    return AABBmax;
}
