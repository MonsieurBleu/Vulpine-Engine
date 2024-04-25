#include <RenderPass.hpp>
#include <random>
#include <Globals.hpp>

FrameBuffer& RenderPass::getFBO(){return FBO;};
ShaderProgram& RenderPass::getShader(){return shader;};

void RenderPass::enable(){isEnable = true;};
void RenderPass::disable()
{
    isEnable = false;
    needClear = true;
};
void RenderPass::toggle()
{
    needClear = isEnable;
    isEnable = !isEnable;
};

const int* RenderPass::getIsEnableAddr()
{
    return &isEnable;
}

void RenderPass::bindResults(std::vector<ivec2> binds)
{
    for(auto i = binds.begin(); i != binds.end(); i++)
        FBO.bindTexture(i->x, i-> y);
}

SSAOPass::SSAOPass(RenderBuffer& inputsBuffer) : inputs(inputsBuffer)
{};

void SSAOPass::setup()
{
    ////// SSAO 
    // generate sample kernel
    // ----------------------
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
    std::default_random_engine generator;
    for (unsigned int i = 0; i < 64; ++i)
    {
        glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        float scale = float(i) / 64.0f;

        // scale samples s.t. they're more aligned to center of kernel
        float a = 0.1f;
        float b = 1.0f;
        float f = (scale * scale);
        scale = a + f * (b - a);
        sample *= scale;
        ssaoKernel.push_back(sample);
    }

    // generate noise texture
    // ----------------------
    std::vector<glm::vec3> ssaoNoise;
    for (unsigned int i = 0; i < 16; i++)
    {
        glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
        ssaoNoise.push_back(noise);
    }

    ssaoNoiseTexture = Texture2D()
        .setResolution(ivec2(4, 4))
        .setInternalFormat(GL_RGBA16F)
        .setFormat(GL_RGB)
        .setPixelType(GL_FLOAT)
        .setFilter(GL_NEAREST)
        .setPixelSource(&ssaoNoise[0])
        .setWrapMode(GL_REPEAT)
        .generate();

    FBO = FrameBuffer()
        .addTexture(
            Texture2D()
                .setResolution(globals.renderSize())
                // .setInternalFormat(GL_R32F)
                // .setFormat(GL_RED)
                .setInternalFormat(GL_SRGB8_ALPHA8)
                .setFormat(GL_RGBA)
                .setPixelType(GL_FLOAT)
                .setAttachement(GL_COLOR_ATTACHMENT0))
        .generate();

    shader = ShaderProgram(
        "shader/post-process/SSAO.frag", 
        "shader/post-process/basic.vert", 
        "")
            // .addUniform(ShaderUniform(camera.getProjectionMatrixAddr(), 9))
            .addUniform(ShaderUniform(globals.renderSizeAddr(), 0))
            .addUniform(ShaderUniform((vec3*)&ssaoKernel[0], 16).setCount(64));

    cameraProjMatrix = ShaderUniform((mat4*)nullptr, 9);
};

void SSAOPass::render(Camera &camera)
{
    if(isEnable)
    {
        cameraProjMatrix.setData(camera.getProjectionMatrixAddr());

        FBO.activate();
        shader.activate();
        cameraProjMatrix.activate();
        ShaderUniform(inverse(camera.getViewMatrix()), 3).activate();
        ShaderUniform(camera.getViewMatrix(), 4).activate();
        ShaderUniform(normalize(camera.getDirection()), 6).activate();
        ssaoNoiseTexture.bind(3);
        globals.drawFullscreenQuad();
        FBO.deactivate();

        // bindResults({vec2(0, AO_RESULTS_BINDING_LOCATION)});
    }
    else
    // if(needClear)
    {
        FBO.activate();
        glClear(0);
        FBO.deactivate();
    }
    bindResults({vec2(0, AO_RESULTS_BINDING_LOCATION)});
}

BloomPass::BloomPass(RenderBuffer& inputsBuffer) : inputs(inputsBuffer)
{};

void BloomPass::setup()
{
    shader = ShaderProgram(
        "shader/post-process/Bloom.frag", 
        "shader/post-process/basic.vert", 
        "",
        globals.standartShaderUniform2D());
    
    FBO
        .addTexture(
            inputs.getTexture(RENDER_BUFFER_SRGB8_BUFF_ID)
                .setAttachement(GL_COLOR_ATTACHMENT0))
        .generate();

    FBO2
        .addTexture(
            inputs.getTexture(RENDER_BUFFER_EMISSIVE_TEXTURE_ID)
                .setAttachement(GL_COLOR_ATTACHMENT0))
        .generate();
}

void BloomPass::render(Camera &camera)
{
    if(isEnable)
    {
        shader.activate(); 

        FBO.activate();
        glUniform1i(3, 1);
        inputs.bindTexture(RENDER_BUFFER_EMISSIVE_TEXTURE_ID, 4);
        globals.drawFullscreenQuad();
        FBO.deactivate();

        FBO2.activate();
        glUniform1i(3, 2);
        inputs.bindTexture(RENDER_BUFFER_SRGB8_BUFF_ID, 4);
        globals.drawFullscreenQuad();
        FBO2.deactivate();   

        inputs.bindTexture(RENDER_BUFFER_EMISSIVE_TEXTURE_ID, 4);

        shader.deactivate();
    }
}

SkyboxPass::SkyboxPass(Texture2D &skybox, std::string shaderFileName) : shaderFileName(shaderFileName)
{
    this->skybox = skybox; 
    this->skybox.setAttachement(GL_COLOR_ATTACHMENT0);
}

void SkyboxPass::setup()
{
    shader = ShaderProgram(
        "shader/skybox/" + shaderFileName, 
        "shader/post-process/basic.vert", 
        "",
        globals.standartShaderUniform2D());

    // FBO = FrameBuffer().addTexture(skybox.setAttachement(0)).generate();
    FBO = FrameBuffer().addTexture(skybox).generate();
}

void SkyboxPass::render(Camera &camera)
{
    if(isEnable)
    {
        // std::cout << FBO.getTexture(0).getHandle() << " " << FBO.getTexture(0).getAttachement() << "\n";
        // std::cout << shader.getProgram() << "\n";
        FBO.activate();
        shader.activate();
        globals.drawFullscreenQuad();
        shader.deactivate();
        FBO.deactivate();
    }
}
