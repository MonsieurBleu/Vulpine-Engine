#include <Textures.hpp>
#include <iostream>

using namespace glm;

Texture2D::Texture2D(){}

Texture2D& Texture2D::setResolution(const ivec2 resolution)
{
    _resolution = resolution;
    return *this;
}

Texture2D& Texture2D::setInternalFormat(GLenum format)
{
    _internalFormat = format;
    return *this;
}

Texture2D& Texture2D::setFormat(GLenum format)
{
    _format = format;
    return *this;
}

Texture2D& Texture2D::setPixelType(GLenum type)
{
    _type = type;
    return *this;
}

Texture2D& Texture2D::setPixelSource(const void *source)
{
    _pixelSource = source;
    return *this;
}

Texture2D& Texture2D::setFilter(GLenum filter)
{
    _filter = filter;
    return *this;
}

Texture2D& Texture2D::setAttachement(GLenum attachement)
{
    _attachement = attachement;
    return *this;
}

Texture2D& Texture2D::setWrapMode(GLenum wrapMode)
{
    _wrapMode = wrapMode;
    return *this;
}

Texture2D& Texture2D::bind(GLuint location)
{
    glActiveTexture(GL_TEXTURE0 + location);
    glBindTexture(GL_TEXTURE_2D, handle);
    return *this;
}

Texture2D& Texture2D::generate()
{
    if(!generated)
    {
        glGenTextures(1, &handle);
        glBindTexture(GL_TEXTURE_2D, handle);

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            _internalFormat,
            _resolution.x,
            _resolution.y,
            0,
            _format,
            _type,
            _pixelSource
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _filter);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _wrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _wrapMode); 

        generated = true;
    }
    
    return *this;
}

Texture2D& Texture2D::loadFromFile(const char* filename)
{
    GLFWimage imageData;
    // undefined ?????
    // glfwReadImage(filename, &imageData, NULL); 

    https://www.glfw.org/GLFWUsersGuide277.pdf

    _pixelSource = imageData.pixels;

    /// suite ) https://www.youtube.com/watch?v=SMyj87VJRJM
}