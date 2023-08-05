#ifndef TEXTURES_HPP
#define TEXTURES_HPP

#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

using namespace glm;

class Texture2D
{
    private : 
        GLuint handle = 0;

        GLenum _internalFormat = GL_SRGB;
        GLenum _format = GL_RGB;
        GLenum _type = GL_UNSIGNED_BYTE;
        GLenum _filter = GL_LINEAR;
        GLenum _wrapMode = GL_REPEAT;

        const void* _pixelSource = nullptr;

        ivec2 _resolution;

        GLenum _attachement;

    public :   
        Texture2D& setResolution(ivec2 resolution);
        Texture2D& setInternalFormat(GLenum format);
        Texture2D& setFormat(GLenum format);
        Texture2D& setPixelType(GLenum type);
        Texture2D& setFilter(GLenum filter);
        Texture2D& setPixelSource(const void *source);
        Texture2D& setAttachement(GLenum attachement);
        Texture2D& setWrapMode(GLenum wrapMode);

        Texture2D& generate();      
        
        GLuint getHandle(){return handle;};
        GLenum getAttachement(){return _attachement;};
        ivec2 getResolution(){return _resolution;};
};

#endif