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

        GLenum _attachement = 0;

        bool generated = false;

    public :   

        Texture2D();

        // TODO : add security when the texture is arleady generated
        Texture2D& setResolution(const ivec2 resolution);
        Texture2D& setInternalFormat(GLenum format);
        Texture2D& setFormat(GLenum format);
        Texture2D& setPixelType(GLenum type);
        Texture2D& setFilter(GLenum filter);
        Texture2D& setPixelSource(const void *source);
        Texture2D& setAttachement(GLenum attachement);
        Texture2D& setWrapMode(GLenum wrapMode);
        Texture2D& loadFromFile(const char* filename);
        Texture2D& loadFromFileHDR(const char* filename);
        Texture2D& loadFromFileKTX(const char* filename);

        Texture2D& generate();      

        Texture2D& bind(GLuint location);
        
        GLuint getHandle(){return handle;};
        GLenum getAttachement(){return _attachement;};
        ivec2 getResolution(){return _resolution;};
};

#endif