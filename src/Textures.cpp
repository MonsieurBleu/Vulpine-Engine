#include <Graphics/Textures.hpp>
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

Texture2D::~Texture2D()
{
    if(handle && handleRef.use_count() == 1)
    {
        glDeleteTextures(1, &handle);
    }
}

Texture2D& Texture2D::deleteHandle()
{
    glDeleteTextures(1, &handle);

    handle = 0;
    *handleRef = 0;

    generated = false;

    return *this;
}

Texture2D& Texture2D::generate(bool forceTexImageCall)
{
    if(!generated)
    {
        glGenTextures(1, &handle);
        glBindTexture(GL_TEXTURE_2D, handle);
        
        handleRef = std::make_shared<GLuint>(handle);

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

        bindlessHandleRef = std::make_shared<GLuint>(0);

        generated = true;
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, handle);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _filter);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _wrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _wrapMode);

        if(forceTexImageCall)
        {
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
        }
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
        FILE_ERROR_MESSAGE(filename, "STB error : this file either don't exist or the format is not supported!");
        return *(this);
    }

    _pixelSource = stbi_load(filename, &_resolution.x, &_resolution.y, &n, 0);

    if(!_pixelSource)
    {
        FILE_ERROR_MESSAGE(filename, "STB error : can load info but can't load pixels of the image!");
        return *(this);
    }

    return *(this);
}

Texture2D& Texture2D::loadFromFileHDR(const char* filename)
{
    stbi_hdr_to_ldr_gamma(1.0f);
    stbi_hdr_to_ldr_scale(1.0f);

    // BenchTimer timer;
    // timer.start();
    int n, fileStatus;
    fileStatus = stbi_info(filename, &_resolution.x, &_resolution.y, &n);

    if(!fileStatus)
    {
        FILE_ERROR_MESSAGE(filename, "STB error : this file either don't exist or the format is not supported!");
        return *(this);
    }

    _pixelSource = stbi_loadf(filename, &_resolution.x, &_resolution.y, &n, 0);
    setPixelType(GL_FLOAT);

    if(!_pixelSource)
    {
        FILE_ERROR_MESSAGE(filename, "STB error : can load info but can't load pixels of the image!");
        return *(this);
    }

    bindlessHandleRef = std::make_shared<GLuint>(0);

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

    if(result)
    {
        FILE_ERROR_MESSAGE(filename, "KTX error code : " << result);
        return *this;
    }

    ktxTexture2 *kTexture2 = (ktxTexture2 *)kTexture;
    if(ktxTexture_NeedsTranscoding(kTexture))
    {
        ktx_texture_transcode_fmt_e tf;
    
        // Using VkGetPhysicalDeviceFeatures or GL_COMPRESSED_TEXTURE_FORMATS or
        // extension queries, determine what compressed texture formats are
        // supported and pick a format. For example

        khr_df_model_e colorModel = ktxTexture2_GetColorModel_e(kTexture2);
        if (colorModel == KHR_DF_MODEL_UASTC) {
            tf = KTX_TTF_ASTC_4x4_RGBA;
        } else if (colorModel == KHR_DF_MODEL_ETC1S) {
            tf = KTX_TTF_ETC;
        } else {
            tf = KTX_TTF_ASTC_4x4_RGBA;
        };

        result = ktxTexture2_TranscodeBasis(kTexture2, tf, 0);
    }

    glGenTextures(1, &handle);
    result = ktxTexture_GLUpload(kTexture, &handle, &target, &glerror);

    
    if(result)
    {
        FILE_ERROR_MESSAGE(filename, "Can't upload file to GPU. KTX error code : " << result);
        return *this;
    }

    ktxTexture_Destroy(kTexture);

    generated = true;

    bindlessHandleRef = std::make_shared<GLuint>(0);
    
    return *(this);
}

Texture2D& Texture2D::loadFromFileKTX_IO(const char* filename)
{
    ktxTexture* kTexture;
    KTX_error_code result;
    // ktx_size_t offset;
    // ktx_uint32_t level, layer, faceSlice; 
    // GLenum target, glerror;

    result = ktxTexture_CreateFromNamedFile(
        filename,
        KTX_TEXTURE_CREATE_NO_FLAGS,
        &kTexture);

    if(result)
    {
        FILE_ERROR_MESSAGE(filename, "KTX error code : " << result);
        return *this;
    }

    ktxTexture2 *kTexture2 = (ktxTexture2 *)kTexture;
    if(ktxTexture_NeedsTranscoding(kTexture))
    {
        ktx_texture_transcode_fmt_e tf;
    
        // Using VkGetPhysicalDeviceFeatures or GL_COMPRESSED_TEXTURE_FORMATS or
        // extension queries, determine what compressed texture formats are
        // supported and pick a format. For example

        khr_df_model_e colorModel = ktxTexture2_GetColorModel_e(kTexture2);
        if (colorModel == KHR_DF_MODEL_UASTC) {
            tf = KTX_TTF_ASTC_4x4_RGBA;
        } else if (colorModel == KHR_DF_MODEL_ETC1S) {
            tf = KTX_TTF_ETC;
        } else {
            tf = KTX_TTF_ASTC_4x4_RGBA;
        };

        result = ktxTexture2_TranscodeBasis(kTexture2, tf, 0);
    }

    // glGenTextures(1, &handle);
    // result = ktxTexture_GLUpload(kTexture, &handle, &target, &glerror);

    
    // if(result)
    // {
    //     FILE_ERROR_MESSAGE(filename, "Can't upload file to GPU. KTX error code : " << result);
    //     return *this;
    // }

    // ktxTexture_Destroy(kTexture);

    // generated = true;

    // bindlessHandleRef = std::make_shared<GLuint>(0);
    
    return *(this);
}


GLuint64 Texture2D::getBindlessHandle()
{
    if(!*bindlessHandleRef)
    {
        *bindlessHandleRef = glGetTextureHandleARB(handle);
        glMakeTextureHandleResidentARB(*bindlessHandleRef);
    }
    return *bindlessHandleRef;
}