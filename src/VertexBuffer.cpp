#include <VertexBuffer.hpp>

using namespace glm;

int VertexAttribute::create(std::shared_ptr<void> _src, 
                           GLuint _location,
                           uint64 _vertexCount, 
                           uint8  _perVertexSize = 3, 
                           GLenum _type = GL_FLOAT,
                           bool   _copySource = false)
{
    if(perVertexSize == 0 || perVertexSize > MAX_PER_VERTEX_SIZE)
        return -1;

    location = _location;
    vertexCount = _vertexCount;
    perVertexSize = _perVertexSize;
    type = _type;
    bufferSize = perValuesSize*perVertexSize*vertexCount;

    switch (type)
    {
    case GL_BYTE :
    case GL_UNSIGNED_BYTE :
        perValuesSize = 1;
        break;

    case GL_HALF_FLOAT :
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

    if(_copySource)
        buffer = std::make_shared<void>(_src);
    else
        buffer = _src;

    return 0;
}

void VertexAttribute::genBuffer()
{
    glGenBuffers(1, &handle);
}

void VertexAttribute::sendAllToGPU()
{
    glBindBuffer(GL_ARRAY_BUFFER, handle);
    glBufferData(GL_ARRAY_BUFFER, bufferSize, buffer.get(), GL_STATIC_DRAW);
}

void VertexAttribute::bindVertexBuffer()
{
    glBindVertexBuffer(location, handle, 0, perValuesSize*perVertexSize);
    glVertexAttribFormat(location, perValuesSize, type, GL_FALSE, 0);
}


void VertexAttributeGroup::forEach(void (*func)(int, VertexAttribute&))
{
    for(int i = 0; i < attributes.size(); i++)
    {
        func(i, attributes[i]);
    }
}

void VertexAttributeGroup::forEach(void (*func)(int, const VertexAttribute&)) const
{
    for(int i = 0; i < attributes.size(); i++)
    {
        func(i, attributes[i]);
    }
}

void VertexAttributeGroup::add(std::vector<VertexAttribute> &newAttributes)
{
    for(auto i = newAttributes.begin(); i < newAttributes.end(); i++)
    {
        attributes.push_back(*i);
    }
}

void VertexAttributeGroup::add(VertexAttribute newAttribute)
{
    attributes.push_back(newAttribute);
}

void VertexAttributeGroup::generate()
{
    forEach([](int i, VertexAttribute& attribute)
    {
        attribute.genBuffer();
        attribute.sendAllToGPU();
    });

    glGenVertexArrays(1, &arrayHandle);
    glBindVertexArray(arrayHandle);

    forEach([](int i, VertexAttribute& attribute)
    {
        glEnableVertexAttribArray(i);
        attribute.bindVertexBuffer();
        glVertexAttribBinding(i, attribute.getLocation());
    }); 
};