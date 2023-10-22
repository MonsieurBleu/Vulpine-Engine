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

#include <Helpers.hpp>


//https://antongerdelan.net/opengl/hellotriangle.html

std::mutex inputMutex;
InputBuffer inputs;

Globals globals;

App::App(GLFWwindow* window) : 
    window(window), 
    renderBuffer(globals.renderSizeAddr()),
    SSAO(renderBuffer),
    Bloom(renderBuffer)
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
        ShaderUniform(camera.getPositionAddr(),               5),
        ShaderUniform(camera.getDirectionAddr(),              6)
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
    renderBuffer.generate();
    SSAO.setup();
    Bloom.setup();
}

void App::mainInput(double deltatime)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        state = quit;
    
    float camspeed = 20.0;
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

    Scene scene;

    bool wireframe = false;
    bool vsync = true;
    glfwSwapInterval(1);

    ShaderProgram PostProcessing = ShaderProgram(
        "shader/post-process/final composing.frag", 
        "shader/post-process/basic.vert", 
        "", 
        globals.standartShaderUniform2D());
    
    PostProcessing.addUniform(ShaderUniform(Bloom.getIsEnableAddr(), 10));

    #ifdef INVERTED_Z
    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
    #endif

    /*    
    scene.add(redLight);
    scene.add(blueLight);

    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
    std::default_random_engine generator;
    for(int i = 0; i < 16; i++)
    for(int i = 0; i < MAX_LIGHT_COUNTER; i++)
    {
        scene.add(
            newPointLight(
                PointLight()
                .setColor(vec3(randomFloats(generator), randomFloats(generator), randomFloats(generator)))
                .setPosition(vec3(4, 2, 4) * vec3(0.5 - randomFloats(generator), randomFloats(generator), 0.5 - randomFloats(generator)))
                .setIntensity(1.75)
                .setRadius(5.0)
                )
        );
    }
    */

    MeshMaterial uvPhong(
            new ShaderProgram(
                "shader/foward rendering/uv/phong.frag", 
                "shader/foward rendering/uv/phong.vert", 
                "", 
                globals.standartShaderUniform3D() 
            ));

    MeshMaterial uvBasic(
            new ShaderProgram(
                "shader/foward rendering/uv/basic.frag", 
                "shader/foward rendering/uv/phong.vert", 
                "", 
                globals.standartShaderUniform3D() 
            ));

    ModelRef skybox = newModel(
        uvBasic, 
        readOBJ("ressources/test/skybox/skybox.obj", false),
        ModelState3D()
            .scaleScalar(1000.0)
            .setPosition(vec3(0.0, 0.0, 0.0)));

    Texture2D skyTexture = Texture2D()
            .loadFromFile("ressources/solar system/crab nebula.png")
            .setFormat(GL_RGBA)
            .setInternalFormat(GL_SRGB8_ALPHA8)
            .generate();

    // Texture2D skyTexture = Texture2D()
    //         .loadFromFileKTX("ressources/solar system/8k_mily_way.ktx");

    skybox->setMap(skyTexture, 0);
    skybox->invertFaces = true;
    skybox->depthWrite = false;
    scene.add(skybox);
    
    {
    // ModelRef jug = newModel(
    //     uvPhong, 
    //     readOBJ("ressources/test/jug.obj", false),
    //     ModelState3D()
    //         .scaleScalar(5.0)
    //         .setPosition(vec3(0.0, 0.0, 0.0)));
    
    // ModelRef barberShopChair = newModel(Mesh().setMaterial(uvPhong));
    // barberShopChair
    //     ->loadFromFolder("ressources/test/chair/")
    //     .state.setPosition(vec3(2.0, 0.0, 0.0));

    // ModelRef guitar = newModel(Mesh().setMaterial(uvPhong));
    // guitar
    //     ->loadFromFolder("ressources/test/guitar/")
    //     .state.setPosition(vec3(4.0, 0.0, 0.0));

    // ModelRef woman = newModel(
    //     uvPhong, 
    //     readOBJ("ressources/test/woman/woman.obj", false),
    //     ModelState3D()
    //         .scaleScalar(1.0)
    //         .setPosition(vec3(-2.0, 0.0, 0.0)));

    // ModelRef plane = newModel(
    //     uvPhong, 
    //     readOBJ("ressources/plane.obj", false),
    //     ModelState3D()
    //         .scaleScalar(0.25)
    //         .setPosition(vec3(0.0, 0.0, 0.0)));
    
    // plane->setMap(
    //     Texture2D()
    //         .loadFromFile("ressources/test/sphere/floor.jpg")
    //         .generate(),
    //         0);
    
    // scene.add(guitar);
    // scene.add(barberShopChair);
    }
    
    ScenePointLight sun = newPointLight(
        PointLight()
            .setColor(vec3(1.0, 1.0, 1.0))
            .setRadius(1000.0)
            .setIntensity(1.25)
            );
    scene.add(sun);
    // scene.add(std::make_shared<PointLightHelper>(sun));

    ModelRef sunModel(new MeshModel3D());
    sunModel->loadFromFolder("ressources/solar system/sun/");
    sunModel->setMaterial(uvPhong);
    scene.add(sunModel);

    ModelRef earth(new MeshModel3D());
    earth->loadFromFolder("ressources/solar system/earth/");
    earth->setMaterial(uvPhong);
    scene.add(earth);
    float ua = 150.f;
    earth->state.setPosition(vec3(ua, 0, 0));
    earth->state.scaleScalar(0.5);

    ModelRef moon(new MeshModel3D());
    moon->loadFromFolder("ressources/solar system/moon/");
    moon->setMaterial(uvPhong);
    scene.add(moon);
    moon->state.scaleScalar(earth->state.scale.x*0.25);

    ModelRef mars(new MeshModel3D());
    mars->loadFromFolder("ressources/solar system/mars/");
    mars->setMaterial(uvPhong);
    scene.add(mars);
    mars->state.scaleScalar(earth->state.scale.x*0.53);

    ModelRef venus(new MeshModel3D());
    venus->loadFromFolder("ressources/solar system/venus/");
    venus->setMaterial(uvPhong);
    scene.add(venus);
    venus->state.scaleScalar(earth->state.scale.x*0.86);

    ModelRef mercury(new MeshModel3D());
    mercury->loadFromFolder("ressources/solar system/mercury/");
    mercury->setMaterial(uvPhong);
    scene.add(mercury);
    mercury->state.scaleScalar(earth->state.scale.x*0.38);

    float simSpeed = 500.0;
    float simTime = 50.0;

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
                Bloom.getShader().reset();
                PostProcessing.reset();
                uvPhong->reset();
                skybox->getMaterial()->reset();
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

            case GLFW_KEY_1 :
                SSAO.toggle();
                break;
            
            case GLFW_KEY_2 :
                Bloom.toggle();
                break;

            case GLFW_KEY_F9 : 
                vsync = !vsync;
                glfwSwapInterval(vsync ? 1 : 0);
                break;
            
            case GLFW_KEY_P : 
                simSpeed *= 2.0;
                break;

            case GLFW_KEY_O : 
                simSpeed *= 0.5;
                break;

            default:
                break;
            }
        }

        mainInput(globals.appTime.getDelta());

        simTime += simSpeed * globals.appTime.getDelta();
        earth->state.setRotation(vec3(0, simTime, glm::radians(23.45)));
        earth->state.setPosition(vec3(ua)*vec3(cos(simTime/365.25), 0.0, sin(simTime/365.25)));

        // The moon always point the same face to the earth
        float moonOrbit = simTime/29.5;
        moon->state.setRotation(vec3(0, -moonOrbit, 0));
        moon->state.setPosition(earth->state.position + vec3(20)*vec3(cos(moonOrbit), 0, sin(moonOrbit)));

        float marsOrbit = simTime/687;
        mars->state.setRotation(vec3(0, simTime, 0));
        mars->state.setPosition(vec3(1.52*ua)*vec3(cos(marsOrbit), 0, sin(marsOrbit)));

        float venusOrbit = simTime/225;
        venus->state.setRotation(vec3(0, simTime/243, 0));
        venus->state.setPosition(vec3(0.7*ua)*vec3(cos(venusOrbit), 0, sin(venusOrbit)));

        float mercuryOrbit = simTime/88;
        venus->state.setRotation(vec3(0, simTime/59, 0));
        mercury->state.setPosition(vec3(0.39*ua)*vec3(cos(mercuryOrbit), 0, sin(mercuryOrbit)));  


        mainloopPreRenderRoutine();
        if(wireframe)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        renderBuffer.activate();
        // ligthBuffer.activate(0);
        skyTexture.bind(4);
        scene.draw();
        renderBuffer.deactivate();
        renderBuffer.bindTextures();

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        SSAO.render(camera);
        Bloom.render(camera);

        glViewport(0, 0, globals.windowWidth(), globals.windowHeight());
        PostProcessing.activate();
        globals.drawFullscreenQuad();
 
        mainloopEndRoutine();
    }
}