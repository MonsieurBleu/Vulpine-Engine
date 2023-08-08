#include <FrameBuffer.hpp>
#include <iostream>
#include <CompilingOptions.hpp>

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
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, 
            textures[i].getAttachement(), 
            GL_TEXTURE_2D, 
            textures[i].getHandle(), 
            0);
    }

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
        return;

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
                .setPixelType(GL_UNSIGNED_SHORT)
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
                .setFilter(GL_LINEAR)
                .setWrapMode(GL_CLAMP_TO_EDGE)
                .setAttachement(GL_COLOR_ATTACHMENT1))
        .generate();
}

void RenderBuffer::bindTextures()
{
    bindTexture(0, 0);
    bindTexture(1, 1);
    bindTexture(2, 2);
    bindTexture(3, 3);
    bindTexture(4, 4);
}

void RenderBuffer::activate()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FRAMEBUFFER_SRGB);
    #ifdef INVERTED_Z
    glDepthFunc(GL_GREATER);
    #else
    glDepthFunc(GL_LESS);
    #endif
    FrameBuffer::activate();
}

void RenderBuffer::deactivate()
{
    FrameBuffer::deactivate();
    glDisable(GL_FRAMEBUFFER_SRGB);
    glDisable(GL_DEPTH_TEST);
}