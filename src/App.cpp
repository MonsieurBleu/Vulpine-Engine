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

#include <Light.hpp>

#include <CompilingOptions.hpp>


//https://antongerdelan.net/opengl/hellotriangle.html

std::mutex inputMutex;
InputBuffer inputs;

Globals globals;

App::App(GLFWwindow* window) : 
    window(window), 
    renderBuffer(globals.renderSizeAddr()),
    SSAO(renderBuffer)
{
    timestart = GetTimeMs();

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
    globals._renderSize = ivec2(globals._windowSize.x*globals._renderScale, globals._windowSize.y*globals._renderScale);

    globals._standartShaderUniform2D =
    {
        ShaderUniform(globals.windowSizeAddr(), 0),
        ShaderUniform(globals.appTime.getElapsedTimeAddr(),   1),

    };

    globals._standartShaderUniform3D =
    {
        ShaderUniform(globals.windowSizeAddr(),               0),
        ShaderUniform(globals.appTime.getElapsedTimeAddr(),   1),
        ShaderUniform(camera.getProjectionViewMatrixAddr(),   2),
        ShaderUniform(camera.getViewMatrixAddr(),             3),
        ShaderUniform(camera.getProjectionMatrixAddr(),       4),
        ShaderUniform(camera.getPositionAddr(),               5)
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

    SSAO.setup();
    renderBuffer.generate();
}

void App::mainInput(double deltatime)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        state = quit;
    
    float camspeed = 0.3;
    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camspeed *= 5.0;

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
    // camera.init(radians(50.0f), globals.windowWidth(), globals.windowHeight(), 1000.0f, 0.1f);

    glEnable(GL_DEPTH_TEST);

    auto myfile = std::fstream("saves/cameraState.bin", std::ios::in | std::ios::binary);
    if(myfile)
    {
        CameraState buff;
        myfile.read((char*)&buff, sizeof(CameraState));
        myfile.close();
        camera.setState(buff);
    }

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
                "shader/foward rendering/basic.frag", 
                "shader/foward rendering/basic.vert", 
                "", 
                globals.standartShaderUniform3D())), 

        MeshMaterial(
            new ShaderProgram(
                "shader/foward rendering/gouraud.frag", 
                "shader/foward rendering/gouraud.vert", 
                "", 
                globals.standartShaderUniform3D())), 

        MeshMaterial(
            new ShaderProgram(
                "shader/foward rendering/phong.frag", 
                "shader/foward rendering/phong.vert", 
                "", 
                globals.standartShaderUniform3D())), 

        MeshMaterial(
            new ShaderProgram(
                "shader/foward rendering/toon.frag", 
                "shader/foward rendering/toon.vert", 
                "", 
                globals.standartShaderUniform3D())),            
    };

    uint64 materialId = 0;

    Scene scene;

    ModelRef model = newModel(
        materials[materialId], 
        TemplateMeshes[TemplateId],
        ModelState3D()
            .scaleScalar(1.0));

    ModelRef model2 = newModel(
        materials[materialId], 
        TemplateMeshesFlatShade[TemplateId],
        ModelState3D()
            .scaleScalar(1.0)
            .setPosition(vec3(2.0, 0.0, 0.0)));

    ModelRef room = newModel(
        materials[materialId], 
        readOBJ("ressources/room.obj"),
        ModelState3D()
            .scaleScalar(1.0)
            .setPosition(vec3(0.0, -0.2, 0.0)));


    scene.add(room, false);
    scene.add(model, false);
    scene.add(model2, false);

    bool wireframe = false;

    ShaderProgram PostProcessing(
        "shader/post-process/final composing.frag", 
        "shader/post-process/basic.vert", 
        "", 
        globals.standartShaderUniform2D());

    const GLenum buffers[]{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
    glDrawBuffers(4, buffers);

    #ifdef INVERTED_Z
    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
    #endif

    ScenePointLight redLight = newPointLight(
        PointLight()
        .setColor(vec3(1, 0, 0))
        .setPosition(vec3(1, 0.5, 0.0))
        .setIntensity(1)
        .setRadius(1.0));

    ScenePointLight blueLight = newPointLight(
        PointLight()
        .setColor(vec3(0, 0.5, 1.0))
        .setPosition(vec3(1, 0.5, -1))
        .setIntensity(2)
        .setRadius(3.0));

    scene.add(
        newDirectionLight(
            DirectionLight()
                .setColor(vec3(1.0))
                .setDirection(normalize(vec3(-0.5, 1.0, 0.25)))
                .setIntensity(0.5)
        )
    );
    // scene.add(redLight);
    // scene.add(blueLight);

    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
    std::default_random_engine generator;
    // for(int i = 0; i < 16; i++)
    for(int i = 0; i < MAX_LIGHT_COUNTER; i++)
    {
        scene.add(
            newPointLight(
                PointLight()
                .setColor(vec3(randomFloats(generator), randomFloats(generator), randomFloats(generator)))
                .setPosition(vec3(4, 2, 4) * vec3(0.5 - randomFloats(generator), randomFloats(generator), 0.5 - randomFloats(generator)))
                .setIntensity(1)
                .setRadius(1.0)
                )
        );
    }

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
            case GLFW_KEY_F3 :
                std::cout 
                << globals.appTime << "\n";
                break;

            case GLFW_KEY_F5:
                system("cls");
                SSAO.getShader().reset();
                PostProcessing.reset();
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
        blueLight->setPosition(vec3(100, 50 + 50*(1.0 + cos(globals.appTime.getElapsedTime())), -100));

        renderBuffer.activate();
        // ligthBuffer.activate(0);
        scene.draw();
        renderBuffer.deactivate();
        renderBuffer.bindTextures();

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        SSAO.render(camera);

        glViewport(0, 0, globals.windowWidth(), globals.windowHeight());
        PostProcessing.activate();
        globals.drawFullscreenQuad();

        mainloopEndRoutine();
    }
}