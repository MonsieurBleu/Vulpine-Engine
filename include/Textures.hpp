#pragma once

#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <memory>
#include <glm/glm.hpp>

using namespace glm;

class Texture2D
{
    private : 
        GLuint handle = 0;
        std::shared_ptr<GLuint> bindlessHandleRef;
        std::shared_ptr<GLuint> handleRef;

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
        ~Texture2D();

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
        Texture2D& loadFromFileKTX_IO(const char* filename);

        /** test
         * 
         * @param forceTexImageCall 
         *      If the image is arleady generated, set to true to force call 
         *      glTexImage2D to update its format, internal format, type and 
         *      resolution. Default is false. Warning : If you call it on a
         *      Texture that uses a pixel source, it will be unsafe and 
         *      likely loose it's data.
        */
        Texture2D& generate(bool forceTexImageCall = false);    
        
        Texture2D& deleteHandle();      

        Texture2D& bind(GLuint location);
        
        GLuint getHandle(){return handle;};
        GLuint64 getBindlessHandle();
        GLenum getAttachement(){return _attachement;};
        ivec2 getResolution(){return _resolution;};
        const ivec2* getResolutionAddr() const {return &_resolution;};
};
