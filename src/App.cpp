#include <Globals.hpp>

#include <Utils.hpp>
#include <string.h> // for memset

#include <fstream>

#include <Timer.hpp>
#include <Sprites.hpp>

#include <Uniforms.hpp>

#include <Mesh.hpp>

#include <Scene.hpp>

#include <FrameBuffer.hpp>

#include <random>

//https://antongerdelan.net/opengl/hellotriangle.html

std::mutex inputMutex;
InputBuffer inputs;

App::App(GLFWwindow* window) : window(window)
{
    timestart = Get_time_ms();

    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        giveCallbackToApp(GLFWKeyInfo{window, key, scancode, action, mods});
    });

    /*
        TODO : 
            Test if the videoMode automaticlly update
    */
    globals._videoMode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    glfwGetWindowSize(window, &globals._windowSize.x, &globals._windowSize.y);

    globals._standartShaderUniform2D =
    {
        ShaderUniform(globals.windowSizeAddr(), 0),
        ShaderUniform(globals.appTime.getElapsedTimeAddr(),   1),

    };

    globals._standartShaderUniform3D =
    {
        ShaderUniform(globals.windowSizeAddr(), 0),
        ShaderUniform(globals.appTime.getElapsedTimeAddr(),   1),
        ShaderUniform(camera.getProjectionViewMatrixAddr(),   2),
        ShaderUniform(camera.getPositionAddr(),               3),
        ShaderUniform(camera.getViewMatrixAddr(),             8),
        ShaderUniform(camera.getProjectionMatrixAddr(),       9)
    };

    globals._fullscreenQuad.setVao(
        MeshVao(new VertexAttributeGroup(
            {
                VertexAttribute(
                    GenericSharedBuffer(
                        (char *)new float[12]{-1.0f,  1.0f, 1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f, 1.0f, -1.0f, -1.0f}
                        ),
                    0, 
                    6, 
                    2, 
                    GL_FLOAT, 
                    false
                )
            }
        )));
    
    globals._fullscreenQuad.getVao()->generate();
}

void App::mainInput(double deltatime)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        state = quit;
    
    float camspeed = 15.0;
    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camspeed *= 10.0;

    vec3 velocity(0.0);

    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        velocity.x += camspeed;
    }
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        velocity.x -= camspeed;
    }
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        velocity.z -= camspeed;
    }
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        velocity.z += camspeed;
    }
    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        velocity.y += camspeed;
    }
    if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        velocity.y -= camspeed;
    }

    camera.move(velocity, deltatime);

    // if(glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS)
    // {
    //     camera.add_FOV(-0.1);
    // }

    // if(glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS)
    // {
    //     camera.add_FOV(0.1);
    // }

    // if(glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS)
    // {
    //     // glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data); //to update partially
    // }
}

void App::mainloopStartRoutine()
{
    globals.appTime.start();
}

void App::mainloopPreRenderRoutine()
{
    camera.updateProjectionViewMatrix();
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  
}

void App::mainloopEndRoutine()
{
    glfwSwapBuffers(window);
    globals.appTime.end();
}

void App::mainloop()
{   
    /// CENTER WINDOW
    glfwSetWindowPos(
        window, 
        (globals.screenResolution().x - globals.windowWidth())/2, 
        (globals.screenResolution().y - globals.windowHeight())/2);

    /// SETTING UP THE CAMERA 
    camera.init(radians(50.0f), globals.windowWidth(), globals.windowHeight(), 0.1f, 1000.0f);

    glEnable(GL_DEPTH_TEST);

    auto myfile = std::fstream("saves/cameraState.bin", std::ios::in | std::ios::binary);
    if(myfile)
    {
        CameraState buff;
        myfile.read((char*)&buff, sizeof(CameraState));
        myfile.close();
        camera.setState(buff);
    }

    MeshMaterial meshMaterial(
        new ShaderProgram(
            "shader/meshTest.frag", 
            "shader/meshTest.vert", 
            "", 
            globals.standartShaderUniform3D()
            ));


    std::vector<MeshVao> TemplateMeshes =
    {
        readOBJ("ressources/sphere.obj"),
        readOBJ("ressources/hardsurface.obj"),
        readOBJ("ressources/voronoi.obj"),
        readOBJ("ressources/cube.obj"),
        readOBJ("ressources/fox.obj"),
        readOBJ("ressources/nerd.obj"),
        readOBJ("ressources/sword.obj"),
        readOBJ("ressources/airboat.obj"),
        readOBJ("ressources/cowgirl.obj"),
        readOBJ("ressources/wraith.obj"),
        readOBJ("ressources/female.obj"),
    };

    uint64 TemplateId = 0;

    std::vector<MeshVao> TemplateMeshesFlatShade =
    {
        readOBJ("ressources/sphere_flatshade.obj"),
        readOBJ("ressources/hardsurface_flatshade.obj"),
        readOBJ("ressources/voronoi_flatshade.obj"),
        readOBJ("ressources/cube_flatshade.obj"),
        readOBJ("ressources/fox_flatshade.obj"),
        readOBJ("ressources/nerd_flatshade.obj"),
        readOBJ("ressources/sword_flatshade.obj"),
        readOBJ("ressources/airboat_flatshade.obj"),
        readOBJ("ressources/cowgirl_flatshade.obj"),
        readOBJ("ressources/wraith_flatshade.obj"),
        readOBJ("ressources/female_flatshade.obj"),
    };

    std::vector<MeshMaterial> materials = 
    {
        MeshMaterial(
            new ShaderProgram(
                "shader/meshTest.frag", 
                "shader/meshTest.vert", 
                "", 
                globals.standartShaderUniform3D())), 

        MeshMaterial(
            new ShaderProgram(
                "shader/gouraud.frag", 
                "shader/gouraud.vert", 
                "", 
                globals.standartShaderUniform3D())), 

        MeshMaterial(
            new ShaderProgram(
                "shader/phong.frag", 
                "shader/phong.vert", 
                "", 
                globals.standartShaderUniform3D())), 

        MeshMaterial(
            new ShaderProgram(
                "shader/toon.frag", 
                "shader/toon.vert", 
                "", 
                globals.standartShaderUniform3D())),            
    };

    uint64 materialId = 0;

    Scene scene;

    std::shared_ptr<MeshModel3D> model = std::make_shared<MeshModel3D>(
        materials[materialId], 
        TemplateMeshes[TemplateId],
        ModelState3D().scaleScalar(100.0));

    std::shared_ptr<MeshModel3D> model2 = std::make_shared<MeshModel3D>(
        materials[materialId], 
        TemplateMeshesFlatShade[TemplateId],
        ModelState3D().scaleScalar(100.0).setPosition(vec3(200.0, 0.0, 0.0)));

    std::shared_ptr<MeshModel3D> room = std::make_shared<MeshModel3D>(
        materials[materialId], 
        readOBJ("ressources/room.obj"),
        ModelState3D().scaleScalar(100.0).setPosition(vec3(0.0, -20.0, 0.0)));


    scene.add(room, false);
    scene.add(model, false);
    scene.add(model2, false);

    bool wireframe = false;

    ShaderProgram PostProcessing("shader/frame.frag", "shader/2dBase.vert", "", globals.standartShaderUniform2D());

    RenderBuffer screen3D(globals.windowSize());

    const GLenum buffers[]{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
    glDrawBuffers(4, buffers);

    ////// SSAO 
    // generate sample kernel
    // ----------------------
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
    std::default_random_engine generator;
    std::vector<glm::vec3> ssaoKernel;
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

    // unsigned int noiseTexture; 
    // glGenTextures(1, &noiseTexture);
    // glBindTexture(GL_TEXTURE_2D, noiseTexture);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 


    Texture2D ssaoNoiseTexture = Texture2D()
        .setResolution(ivec2(4, 4))
        .setInternalFormat(GL_RGBA16F)
        .setFormat(GL_RGB)
        .setPixelType(GL_FLOAT)
        .setFilter(GL_NEAREST)
        .setPixelSource(&ssaoNoise[0])
        .setWrapMode(GL_REPEAT)
        .generate();

    FrameBuffer ssaoFBO = FrameBuffer()
        .addTexture(
            Texture2D()
                .setResolution(globals.windowSize())
                .setInternalFormat(GL_RGB)
                .setFormat(GL_RGB)
                .setPixelType(GL_FLOAT)
                .setAttachement(GL_COLOR_ATTACHMENT0))
        .generate();
    
    auto ssaoUniforms = globals.standartShaderUniform2D();
    ssaoUniforms.push_back(ShaderUniform(camera.getProjectionMatrixAddr(), 9));

    ShaderProgram SSAO(
            "shader/SSAO.frag", 
            "shader/2dBase.vert", 
            "", 
            ssaoUniforms
            );
    //////////////////////////////////////

    while(state != quit)
    {
        mainloopStartRoutine();
        
        glfwPollEvents();

        camera.updateMouseFollow(window);

        GLFWKeyInfo input;
        while(inputs.pull(input))
        {
            if(input.action != GLFW_PRESS)
                continue;

            switch (input.key)
            {
            case GLFW_KEY_F5:
                system("cls");
                SSAO.reset();
                PostProcessing.reset();
                meshMaterial->reset();
                for(uint64 i = 0; i < materials.size(); i++)
                    materials[i]->reset();
                break;
            
            case GLFW_KEY_F2:
                camera.toggleMouseFollow();
                break;
            
            case GLFW_KEY_F8:
            {
                auto myfile = std::fstream("saves/cameraState.bin", std::ios::out | std::ios::binary);
                myfile.write((char*)&camera.getState(), sizeof(CameraState));
                myfile.close();
            }
                break;
            
            case GLFW_KEY_F1 :
                wireframe = !wireframe;
                break;

            case GLFW_KEY_LEFT:
                TemplateId = TemplateId == 0 ? TemplateMeshes.size()-1 : TemplateId-1;
                model->setVao(TemplateMeshes[TemplateId]);
                model2->setVao(TemplateMeshesFlatShade[TemplateId]);
                break;
            
            case GLFW_KEY_RIGHT:                
                TemplateId = TemplateId == TemplateMeshes.size()-1 ? 0 : TemplateId+1;
                model->setVao(TemplateMeshes[TemplateId]);
                model2->setVao(TemplateMeshesFlatShade[TemplateId]);
                break;
            
            case GLFW_KEY_UP :
                materialId = materialId == materials.size()-1 ? 0 : materialId+1;
                model->setMaterial(materials[materialId]);
                model2->setMaterial(materials[materialId]);
                room->setMaterial(materials[materialId]);
                break;

            case GLFW_KEY_DOWN :
                materialId = materialId == 0 ? materials.size()-1 : materialId-1;
                model->setMaterial(materials[materialId]);
                model2->setMaterial(materials[materialId]);
                room->setMaterial(materials[materialId]);
                break;

            default:
                break;
            }
        }

        mainInput(globals.appTime.getDelta());

        mainloopPreRenderRoutine();
        if(wireframe)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        model->state.setRotation(vec3(0.0, globals.appTime.getElapsedTime()*0.5, 0.0));
        model2->state.setRotation(vec3(0.0, globals.appTime.getElapsedTime()*0.5, 0.0));

        screen3D.activate();
        scene.draw();
        screen3D.deactivate();

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


        screen3D.bindTextures();

        ssaoFBO.activate();
        SSAO.activate();
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, ssaoNoiseTexture.getHandle());
        glUniform3fv(16, 64, (float*)&ssaoKernel[0]);
        globals.drawFullscreenQuad();
        ssaoFBO.deactivate();

        glViewport(0, 0, globals.windowWidth(), globals.windowHeight());
        ssaoFBO.bindTexture(0, 6);
        PostProcessing.activate();
        globals.drawFullscreenQuad();

        mainloopEndRoutine();
    }
}