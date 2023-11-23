#include <Textures.hpp>
#include <iostream>
#include <stb/stb_image.h>
#include <Utils.hpp>
#include <Timer.hpp>

#include <ktx.h>

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
    if(generated)
    {
        glActiveTexture(GL_TEXTURE0 + location);
        glBindTexture(GL_TEXTURE_2D, handle);
    }
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
    BenchTimer timer;
    timer.start();
    int n, fileStatus;
    fileStatus = stbi_info(filename, &_resolution.x, &_resolution.y, &n);

    if(!fileStatus)
    {
        std::cerr 
        << TERMINAL_ERROR << "Texture2D::loadFromFile : stb error, can't load image " 
        << TERMINAL_FILENAME << filename 
        << TERMINAL_ERROR << ". This file either don't exist or the format is not supported.\n"
        << TERMINAL_RESET; 
    }

    _pixelSource = stbi_load(filename, &_resolution.x, &_resolution.y, &n, 0);

    if(!_pixelSource)
    {
        std::cerr 
        << TERMINAL_ERROR << "Texture2D::loadFromFile : stb error, can load info but can't load pixels of image " 
        << TERMINAL_FILENAME << filename << "\n"
        << TERMINAL_RESET; 
    }

    timer.end();
    std::cout 
    << TERMINAL_OK << "Successfully loaded image "
    << TERMINAL_FILENAME << filename 
    << TERMINAL_OK << " in " 
    << TERMINAL_TIMER << timer.getElapsedTime() << " s\n"
    << TERMINAL_RESET;

    return *(this);
}

Texture2D& Texture2D::loadFromFileHDR(const char* filename)
{
    // BenchTimer timer;
    // timer.start();
    int n, fileStatus;
    fileStatus = stbi_info(filename, &_resolution.x, &_resolution.y, &n);

    if(!fileStatus)
    {
        std::cerr 
        << TERMINAL_ERROR << "Texture2D::loadFromFile : stb error, can't load image " 
        << TERMINAL_FILENAME << filename 
        << TERMINAL_ERROR << ". This file either don't exist or the format is not supported.\n"
        << TERMINAL_RESET; 
    }

    _pixelSource = stbi_loadf(filename, &_resolution.x, &_resolution.y, &n, 0);
    setPixelType(GL_FLOAT);

    if(!_pixelSource)
    {
        std::cerr 
        << TERMINAL_ERROR << "Texture2D::loadFromFile : stb error, can load info but can't load pixels of image " 
        << TERMINAL_FILENAME << filename << "\n"
        << TERMINAL_RESET; 
    }

    // timer.end();
    // std::cout 
    // << TERMINAL_OK << "Successfully loaded image "
    // << TERMINAL_FILENAME << filename 
    // << TERMINAL_OK << " in " 
    // << TERMINAL_TIMER << timer.getElapsedTime() << " s\n"
    // << TERMINAL_RESET;

    return *(this);
}

Texture2D& Texture2D::loadFromFileKTX(const char* filename)
{
    ktxTexture* kTexture;
    KTX_error_code result;
    // ktx_size_t offset;
    // ktx_uint32_t level, layer, faceSlice; 
    GLenum target, glerror;

    result = ktxTexture_CreateFromNamedFile(
        filename,
        KTX_TEXTURE_CREATE_NO_FLAGS,
        &kTexture);

    glGenTextures(1, &handle);
    result = ktxTexture_GLUpload(kTexture, &handle, &target, &glerror);
    
    if(result)
    {
        std::cout 
        << TERMINAL_ERROR << "Error loading file "
        << TERMINAL_FILENAME << filename 
        << TERMINAL_ERROR << ". Errore code : " << glerror
        << TERMINAL_RESET << "\n";
    }

    // don't work but technilcy need that in the future
    // ktxTexture_Destroy(handle);

    generated = true;

    return *(this);
}