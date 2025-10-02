#include "Graphics/Camera.hpp"
#include <Graphics/Textures.hpp>
#include <GL/glu.h>
#include <cstring>
#include <glm/ext/vector_int2.hpp>
#include <iostream>
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>
#include <Utils.hpp>
#include <Timer.hpp>

#include <ktx.h>

#define TINYEXR_IMPLEMENTATION
#define TINYEXR_USE_MINIZ 0
#define TINYEXR_USE_STB_ZLIB 1
#define TINYEXR_USE_THREAD 1
#include <tinyexr.h>

using namespace glm;

Texture2D::Texture2D(){}

bool Texture2D::operator==(Texture2D &a)
{
    return 
        loadedChannels == a.loadedChannels
        && handle == a.handle
        && _internalFormat == a._internalFormat
        && _format == a._format
        && _type == a._type
        && _filter == a._filter
        && _wrapMode == a._wrapMode
        && _resolution == a._resolution;
}

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
        // if(_pixelSource) freeSource();
        
        glDeleteTextures(1, &handle);
    }
}

void Texture2D::freeSource()
{
    // if(_type == GL_FLOAT)
    if(_pixelSource)
    {
        delete((float *)_pixelSource); 
        _pixelSource = nullptr;
    }
};

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


        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _filter);

        if(_filter == GL_LINEAR_MIPMAP_LINEAR)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        else 
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _filter);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _wrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _wrapMode);

        if(!_pixelSource)
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
        else
        {
            gluBuild2DMipmaps(
                GL_TEXTURE_2D, 
                _internalFormat, 
                _resolution.x, 
                _resolution.y, 
                _format,
                _type,
                _pixelSource
            );
        }

        bindlessHandleRef = std::make_shared<GLuint>(0);

        generated = true;
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, handle);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _filter);

        if(_filter == GL_LINEAR_MIPMAP_LINEAR)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        else 
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _filter);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _wrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _wrapMode);

        if(forceTexImageCall)
        {
            if(!_pixelSource)
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
            else
            {
                gluBuild2DMipmaps(
                    GL_TEXTURE_2D, 
                    _internalFormat, 
                    _resolution.x, 
                    _resolution.y, 
                    _format,
                    _type,
                    _pixelSource
                );
            }
        }
    }

    // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -0.5f);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 16.f);
    
    return *this;
}

Texture2D& Texture2D::loadFromFile(const char* filename)
{
    BenchTimer timer;
    timer.start();
    int fileStatus;
    fileStatus = stbi_info(filename, &_resolution.x, &_resolution.y, &loadedChannels);

    if(!fileStatus)
    {
        FILE_ERROR_MESSAGE(filename, "STB error : this file either don't exist or the format is not supported!");
        return *(this);
    }

    _pixelSource = stbi_load(filename, &_resolution.x, &_resolution.y, &loadedChannels, loadedChannels);

    switch (loadedChannels)
    {
        case 1 : setInternalFormat(GL_R8).setFormat(GL_R); break;
        case 2 : setInternalFormat(GL_RG8).setFormat(GL_RG); break;
        case 3 : setInternalFormat(GL_RGB8).setFormat(GL_RGB); break;
        case 4 : setInternalFormat(GL_RGBA8).setFormat(GL_RGBA); break;
        default: break;
    }

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

Texture2D& Texture2D::loadFromFileEXR(const char* filename)
{
    const char* err = nullptr; 
    int fileStatus = 0;


    // fileStatus = LoadEXR((float **)&_pixelSource, &_resolution.x, &_resolution.y, filename, &err);
    // // fileStatus = LoadEXRWithLayer((float **)&_pixelSource, &_resolution.x, &_resolution.y, filename, "Z", &err);

    // if(fileStatus != TINYEXR_SUCCESS)
    // {
    //     FILE_ERROR_MESSAGE(filename, err);
    //     FreeEXRErrorMessage(err); 
    //     return *(this);
    // }
    


    // 1. Read EXR version.
    EXRVersion exr_version;

    fileStatus = ParseEXRVersionFromFile(&exr_version, filename);
    if (fileStatus)
    {
        FILE_ERROR_MESSAGE(filename, "Invalid EXR file");
        return *(this);
    }

    if (exr_version.multipart) {
        // must be multipart flag is false.
        return *(this);
    }

    // 2. Read EXR header
    EXRHeader exr_header;
    InitEXRHeader(&exr_header);

    fileStatus = ParseEXRHeaderFromFile(&exr_header, &exr_version, filename, &err);
    if (fileStatus)
    {
        FILE_ERROR_MESSAGE(filename, err);
        FreeEXRErrorMessage(err); // free's buffer for an error message
        return *(this);
    }

    // // Read HALF channel as FLOAT.
    for (int i = 0; i < exr_header.num_channels; i++) {
      if (exr_header.pixel_types[i] == TINYEXR_PIXELTYPE_HALF) {
        exr_header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;
      }
    }

    EXRImage exr_image;
    InitEXRImage(&exr_image);
    
    fileStatus = LoadEXRImageFromFile(&exr_image, &exr_header, filename, &err);
    if (fileStatus)
    {
        FILE_ERROR_MESSAGE(filename, err);
        FreeEXRHeader(&exr_header);
        FreeEXRErrorMessage(err); // free's buffer for an error message
        return *(this);
    }

    // _pixelSource = exr_image.images[0];
    _pixelSource = new float[exr_image.width*exr_image.height];
    memcpy((void *)_pixelSource, (void *)exr_image.images[0], sizeof(float)*exr_image.width*exr_image.height);

    _resolution.x = exr_image.width;
    _resolution.y = exr_image.height;

    // NOTIF_MESSAGE(exr_image.num_channels << " " << exr_image.num_tiles)
    
    // 3. Access image data
    // `exr_image.images` will be filled when EXR is scanline format.
    // `exr_image.tiled` will be filled when EXR is tiled format.

    // 4. Free image data



    FreeEXRImage(&exr_image);
    FreeEXRHeader(&exr_header);

    setPixelType(GL_FLOAT);
    setInternalFormat(GL_DEPTH_COMPONENT32F);
    setFormat(GL_DEPTH_COMPONENT);
    setWrapMode(GL_CLAMP);
    // setFilter(GL_LINEAR);
    setFilter(GL_LINEAR_MIPMAP_LINEAR);
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
        // ktx_texture_transcode_fmt_e tf;
        ktx_transcode_fmt_e tf;

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

    setFilter(GL_LINEAR_MIPMAP_LINEAR);
    
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
        // ktx_texture_transcode_fmt_e tf;

        ktx_transcode_fmt_e tf;
    
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

ivec2 Texture2D::getResolution() const {return _resolution;};
GLenum Texture2D::getInternalFormat() const {return _internalFormat;};
GLenum Texture2D::getFormat() const {return _format;};
GLenum Texture2D::getPixelType() const {return _type;};
GLenum Texture2D::getFilter() const {return _filter;};
GLenum Texture2D::getAttachement() const {return _attachement;};
GLenum Texture2D::getWrapMode() const {return _wrapMode;};
const void * Texture2D::getPixelSource() const {return _pixelSource;};