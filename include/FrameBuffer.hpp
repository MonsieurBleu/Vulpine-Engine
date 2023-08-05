#ifndef FRAME_BUFFER_HPP
#define FRAME_BUFFER_HPP

#include <Textures.hpp>
#include <vector>
#include <memory>

using namespace glm;

class FrameBuffer
{
    private : 
        std::vector<Texture2D> textures;

        GLuint handle;

    public : 

        FrameBuffer& addTexture(Texture2D texture);

        FrameBuffer& generate();

        FrameBuffer& generateHandle();
        FrameBuffer& bindTextures();

        GLuint getHandle(){return handle;};

        void activate();
        void deactivate();

        void bindTexture(uint64 id, uint location);
};

class RenderBuffer : public FrameBuffer
{    
    public : 
        RenderBuffer(ivec2 resolution);
        void bindTextures();
        void activate();
        void deactivate();
};

#endif