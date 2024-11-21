#ifndef RENDER_PASS_HPP
#define RENDER_PASS_HPP

#include <Timer.hpp>
#include <Graphics/Shader.hpp>
#include <Graphics/FrameBuffer.hpp>
#include <Graphics/Camera.hpp>

#define AO_RESULTS_BINDING_LOCATION 3

class RenderPass
{
    protected :
        FrameBuffer FBO;
        ShaderProgram shader;
        int isEnable = true;
        bool needClear = true;

    public : 
        RenderPass() {};

        virtual void setup() = 0;
        virtual void render(Camera &camera) = 0;
        virtual void bindResults(std::vector<ivec2> binds);
        
        FrameBuffer& getFBO();
        ShaderProgram& getShader();
        const int* getIsEnableAddr();
        void enable();
        void disable();
        void toggle();

        bool isPassEnable(){return isEnable;};
};

class SSAOPass : public RenderPass
{
    private :
        RenderBuffer& inputs;
        Texture2D ssaoNoiseTexture;
        std::vector<glm::vec3> ssaoKernel;
        ShaderUniform cameraProjMatrix;

    public :
        SSAOPass(RenderBuffer& inputsBuffer);
        void setup();
        void render(Camera &camera);
};

/*
    Read in the emmissive texture, write the result in the color texture
*/
class BloomPass : public RenderPass
{
    private : 
        FrameBuffer FBO2;
        RenderBuffer& inputs;
    
    public : 
        FrameBuffer& getFBO2(){return FBO2;};
        BloomPass(RenderBuffer& inputsBuffer);
        void setup();
        void render(Camera &camera);
};

class SkyboxPass : public RenderPass
{
    private : 
        FrameBuffer inputs;
        Texture2D skybox;
        std::string shaderFileName;

    public : 
        SkyboxPass(Texture2D &skybox, std::string shaderFileName);
        void setup();
        void render(Camera &camera);
};

#endif