#ifndef FRAME_BUFFER_HPP
#define FRAME_BUFFER_HPP

#include <Textures.hpp>
#include <vector>
#include <memory>

using namespace glm;

class FrameBuffer
{
    protected : 
        std::vector<Texture2D> textures;

        GLuint handle = 0;

    public : 

        FrameBuffer& addTexture(Texture2D texture);

        FrameBuffer& generate();

        FrameBuffer& generateHandle();
        FrameBuffer& bindTextures();

        GLuint getHandle(){return handle;};

        void activate();
        void deactivate();

        void bindTexture(uint64 id, uint location);

        Texture2D& getTexture(int id);
        int getNBtextures() const{return textures.size();};

        FrameBuffer& resizeAll(ivec2 newres);
};

#define RENDER_BUFFER_COLOR_TEXTURE_ID 0
#define RENDER_BUFFER_DEPTH_TEXTURE_ID 1
#define RENDER_BUFFER_NORMAL_TEXTURE_ID 2
#define RENDER_BUFFER_EMISSIVE_TEXTURE_ID 3
#define RENDER_BUFFER_SRGB8_BUFF_ID 4

class RenderBuffer : public FrameBuffer
{    
    private : 
        const ivec2 *resolution;
        
    public : 
        RenderBuffer(const ivec2 *resolution);
        void bindTextures();
        void activate();
        void deactivate();
        void generate();

        
};

#endif