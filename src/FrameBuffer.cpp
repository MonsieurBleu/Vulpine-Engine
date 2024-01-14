#include <FrameBuffer.hpp>
#include <iostream>
#include <CompilingOptions.hpp>
#include <Utils.hpp>

Texture2D FrameBuffer::getTexture(int id)
{
    return textures[id%textures.size()];
}

FrameBuffer& FrameBuffer::addTexture(Texture2D texture)
{
    textures.push_back(texture.generate());
    return *this;
}

FrameBuffer& FrameBuffer::generate()
{
    generateHandle();
    bindTextures();
    return *this;
}

FrameBuffer& FrameBuffer::generateHandle()
{
    glGenFramebuffers(1, &handle);
    glBindFramebuffer(GL_FRAMEBUFFER, handle);
    return *this;
}

FrameBuffer& FrameBuffer::bindTextures()
{
    for(uint64 i = 0; i < textures.size(); i++)
    {
        textures[i].generate();
        if(textures[i].getAttachement())
        {
            glFramebufferTexture2D(
                GL_FRAMEBUFFER, 
                textures[i].getAttachement(), 
                GL_TEXTURE_2D, 
                textures[i].getHandle(), 
                0);
        }
    }

    const GLenum buffers[]{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, buffers);

    return *this;
}

void FrameBuffer::activate()
{
    if(textures.empty())
        return;

    glBindFramebuffer(GL_FRAMEBUFFER, handle);
    // glClearColor(0.2, 0.3, 0.3, 1.0);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    #ifdef INVERTED_Z
    glClearDepth(0.0f);
    #else
    glClearDepth(1.0f);
    #endif
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  
    glViewport(0, 0, textures[0].getResolution().x, textures[0].getResolution().y);
}

void FrameBuffer::deactivate()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::bindTexture(uint64 id, uint location)
{
    if(id >= textures.size())
    {
        std::cerr 
        << TERMINAL_WARNING << "FrameBuffer Error : can't bind out of bound texture at id " 
        << TERMINAL_INFO << id
        << TERMINAL_WARNING << " and location "
        << TERMINAL_INFO << location
        << TERMINAL_WARNING << ". The maximum id is " << textures.size()-1 << ".\n";
        return;
    }

    glActiveTexture(GL_TEXTURE0 + location);
    glBindTexture(GL_TEXTURE_2D, textures[id].getHandle());
}

RenderBuffer::RenderBuffer(const ivec2 *resolution) : resolution(resolution)
{}

void RenderBuffer::generate()
{
    (*this)
        .addTexture(
            Texture2D() // COLOR 
                .setResolution(*resolution)
                .setInternalFormat(GL_SRGB)
                .setFormat(GL_RGB)
                .setPixelType(GL_FLOAT)
                .setFilter(GL_LINEAR)
                .setWrapMode(GL_CLAMP_TO_EDGE)
                .setAttachement(GL_COLOR_ATTACHMENT0))
        .addTexture(
            Texture2D() // DEPTH
                .setResolution(*resolution)
                .setInternalFormat(GL_DEPTH_COMPONENT32F)
                .setFormat(GL_DEPTH_COMPONENT)
                .setPixelType(GL_FLOAT)
                .setFilter(GL_LINEAR)
                .setWrapMode(GL_CLAMP_TO_EDGE)
                .setAttachement(GL_DEPTH_ATTACHMENT))
        .addTexture(
            Texture2D() // NORMAL
                .setResolution(*resolution)
                .setInternalFormat(GL_RGB)
                .setFormat(GL_RGB)
                .setPixelType(GL_FLOAT)
                .setFilter(GL_NEAREST)
                .setWrapMode(GL_CLAMP_TO_EDGE)
                .setAttachement(GL_COLOR_ATTACHMENT1))
        .addTexture(
            Texture2D() // EMISSIVE
                .setResolution(*resolution)
                .setInternalFormat(GL_SRGB)
                .setFormat(GL_RGB)
                .setPixelType(GL_FLOAT)
                .setFilter(GL_LINEAR)
                .setWrapMode(GL_CLAMP_TO_EDGE)
                .setAttachement(GL_COLOR_ATTACHMENT2))
        .addTexture(
            Texture2D() // SRGB8 BUFFER
                .setResolution(*resolution)
                .setInternalFormat(GL_RGB)
                .setFormat(GL_RGB)
                .setPixelType(GL_FLOAT)
                .setFilter(GL_LINEAR)
                .setWrapMode(GL_CLAMP_TO_EDGE))
        .generate(); 
}

void RenderBuffer::bindTextures()
{
    bindTexture(RENDER_BUFFER_COLOR_TEXTURE_ID, 0);
    bindTexture(RENDER_BUFFER_DEPTH_TEXTURE_ID, 1);
    bindTexture(RENDER_BUFFER_NORMAL_TEXTURE_ID, 2);
    bindTexture(RENDER_BUFFER_EMISSIVE_TEXTURE_ID, 4);
}

void RenderBuffer::activate()
{
    // glEnable(GL_DEPTH_TEST);
    glEnable(GL_FRAMEBUFFER_SRGB);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW); 

    // #ifdef INVERTED_Z
    // glDepthFunc(GL_GREATER);
    // #else
    // glDepthFunc(GL_LESS);
    // #endif

    FrameBuffer::activate();
}

void RenderBuffer::deactivate()
{
    FrameBuffer::deactivate();
    glDisable(GL_FRAMEBUFFER_SRGB);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
}