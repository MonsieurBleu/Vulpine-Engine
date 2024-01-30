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

#include <GameObject.hpp>

#include <Fonts.hpp>
#include <FastUI.hpp>

#include <CubeMap.hpp>

#include <sstream>
#include <iomanip>
#include <codecvt>

#include <stb/stb_image.h>

#include <Audio.hpp>

#include <Controller.hpp>

//https://antongerdelan.net/opengl/hellotriangle.html

std::mutex inputMutex;
std::mutex physicsMutex;
InputBuffer inputs;

Globals globals;

std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> UFTconvert;

void App::init()
{
    return;
}

void App::setIcon(const std::string &filename)
{
    GLFWimage image[1]; 
    image[0].pixels = stbi_load(filename.c_str(), &image[0].width, &image[0].height, 0, 4);
    glfwSetWindowIcon(window, 1, image); 
    stbi_image_free(image[0].pixels);
}

bool App::userInput(GLFWKeyInfo input){return false;};

bool App::baseInput(GLFWKeyInfo input)
{
    bool used = false;

    if(globals._currentController)
        used = globals._currentController->inputs(input);

    if(used) return true;

    if(input.action == GLFW_RELEASE)
    switch (input.key)
    {
        case GLFW_MOUSE_BUTTON_LEFT:
            globals._mouseLeftClickDown = false;
            used = true;
            break;
        
        default: break;
    }

    if(input.action == GLFW_PRESS)
    switch (input.key)
    {
        case GLFW_MOUSE_BUTTON_LEFT :
            globals._mouseLeftClick = true;
            globals._mouseLeftClickDown = true;
            used = true;
            break;
        
        case GLFW_KEY_V :
            if(input.mods&GLFW_MOD_CONTROL)
            {
                if(globals.isTextInputsActive())
                {
                    globals.textInputString += UFTconvert.from_bytes(glfwGetClipboardString(window));
                    used = true;
                }
            }
            break;
        
        case GLFW_KEY_ENTER :
            if(globals.isTextInputsActive())
            {
                globals.textInputString.push_back(U'\n');
                used = true;
            }
            break;

        case GLFW_KEY_DELETE :
        case GLFW_KEY_BACKSPACE :
            if(globals.isTextInputsActive() && !globals.textInputString.empty())
            {
                globals.textInputString.pop_back();
                used = true;
            }
            break;
        
        default: break;
    }

    return used;
}

void App::setController(Controller *c)
{
    globals._currentController = c;
    glfwSetCursorPosCallback(window,[](GLFWwindow* window, double dx, double dy)
    {
        if(globals._currentController)
            globals._currentController->mouseEvent(vec2(dx, dy), window);
    });
}

App::App(GLFWwindow* window) : 
    window(window), 
    renderBuffer(globals.renderSizeAddr()),
    SSAO(renderBuffer),
    Bloom(renderBuffer)
{
    if(!alCall(alDistanceModel, AL_INVERSE_DISTANCE_CLAMPED))
    {
        std::cerr << "ERROR: Could not set Distance Model to AL_INVERSE_DISTANCE_CLAMPED" << std::endl;
    }

    timestart = GetTimeMs();

    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        giveCallbackToApp(GLFWKeyInfo{window, key, scancode, action, mods});
    });

    // GLFWwindow* window, int button, int action, int mods

    glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods){
        giveCallbackToApp(GLFWKeyInfo{window, button, button, action, mods});
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
        ShaderUniform(camera.getDirectionAddr(),              6),
        ShaderUniform(&ambientLight,                         15),
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

    screenBuffer2D
        .addTexture(
            Texture2D().
                setResolution(globals.windowSize())
                .setInternalFormat(GL_SRGB8_ALPHA8)
                .setFormat(GL_RGBA)
                .setPixelType(GL_UNSIGNED_BYTE)
                .setFilter(GL_LINEAR)
                .setWrapMode(GL_CLAMP_TO_EDGE)
                .setAttachement(GL_COLOR_ATTACHMENT0))
        .addTexture(
            Texture2D() 
                .setResolution(globals.windowSize())
                .setInternalFormat(GL_DEPTH_COMPONENT)
                .setFormat(GL_DEPTH_COMPONENT)
                .setPixelType(GL_UNSIGNED_BYTE)
                .setFilter(GL_LINEAR)
                .setWrapMode(GL_CLAMP_TO_EDGE)
                .setAttachement(GL_DEPTH_ATTACHMENT))
        .generate();

    globals.currentCamera = &camera;

    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    
    // glfwSetCursorPosCallback(window,[](GLFWwindow* window, double dx, double dy)
    // {
    //     static bool lastCameraFollow = !globals.currentCamera->getMouseFollow();
    //     bool cameraFollow = globals.currentCamera->getMouseFollow();

    //     if(!lastCameraFollow && cameraFollow)
    //         glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    //     else if(lastCameraFollow && !cameraFollow)
    //         glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    //     lastCameraFollow = cameraFollow;

    //     if(globals.currentCamera->getMouseFollow())
    //     {
    //         /* TODO : replace this by a global function call, so that the camera 
    //                   controller can be easly changed.
    //         */
    //         {
    //             vec2 center(globals.windowWidth()*0.5, globals.windowHeight()*0.5);
    //             vec2 sensibility(50.0);
    //             vec2 dir = sensibility * (vec2(dx, dy)-center)/center;

    //             float yaw = radians(-dir.x);
    //             float pitch = radians(-dir.y);

    //             vec3 up = vec3(0,1,0);
    //             vec3 front = mat3(rotate(mat4(1), yaw, up)) * globals.currentCamera->getDirection();
    //             front = mat3(rotate(mat4(1), pitch, cross(front, up))) * front;
    //             front = normalize(front);

    //             front.y = clamp(front.y, -0.9f, 0.9f);
    //             globals.currentCamera->setDirection(front);

    //             glfwSetCursorPos(window, center.x, center.y);
    //         }
    //     }
    // });
    
    glfwSetCharCallback(window, [](GLFWwindow* window, unsigned int codepoint)
    {
        // globals.textInputString << (char32_t)codepoint;
        // globals.textInputString.str().push_back(codepoint);
        globals.textInputString.push_back(codepoint);
    });
    
    glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset)
    {
        globals._scrollOffset = vec2(xoffset, yoffset);
        GLFWKeyInfo i;
        i.key = 0;
        inputs.add(i);
    });

    /// CENTER WINDOW
    glfwSetWindowPos(
        window, 
        (globals.screenResolution().x - globals.windowWidth())/2, 
        (globals.screenResolution().y - globals.windowHeight())/2);

    #ifdef INVERTED_Z
    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
    #endif

    glLineWidth(3.0);

    globals.basicMaterial = MeshMaterial(
        new ShaderProgram(
            "shader/foward/basic.frag",
            "shader/foward/basic.vert",
            "",
            globals.standartShaderUniform3D()));
}

void App::mainInput(double deltatime)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        state = quit;
    
    float camspeed = 5.0;
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

    camera.move(velocity, deltatime);
}

void App::mainloopStartRoutine()
{
    globals.fpsLimiter.start();
    globals.appTime.start();
    globals.simulationTime.start();
    globals.cpuTime.start();

    globals._mouseLeftClick = false;

    glfwPollEvents();
    globals._windowHasFocus = glfwGetWindowAttrib(window, GLFW_FOCUSED);

    double mpx, mpy;
    glfwGetCursorPos(window, &mpx, &mpy);
    globals._mousePosition = vec2(mpx, mpy);
} 

void App::mainloopPreRenderRoutine()
{
    globals.currentCamera->updateProjectionViewMatrix();

    vec3 camPos = globals.currentCamera->getPosition();   
    vec3 camFront = globals.currentCamera->getDirection();
    vec3 camDir[2] = 
    {
        globals.currentCamera->getDirection(),
        cross(camFront, cross(camFront, vec3(0, 1, 0)))
    };

    //set current listener position
    alListener3f(AL_POSITION, -camPos.x, camPos.y, -camPos.z);

    //set current listener orientation
    alListenerfv(AL_ORIENTATION, (float*)camDir);

    if(globals._currentController)
        globals._currentController->update();
}

void App::mainloopEndRoutine()
{
    globals.cpuTime.end();

    globals.gpuTime.start();
    glfwSwapBuffers(window);
    globals.gpuTime.end();

    globals.fpsLimiter.waitForEnd();
    globals.appTime.end();
    globals.simulationTime.end();
}

void App::mainloop()
{   
    /// SETTING UP THE CAMERA 
    camera.init(radians(70.0f), globals.windowWidth(), globals.windowHeight(), 0.1f, 1E4f);
    // camera.init(radians(50.0f), 1920*0.05, 1080*0.05, 0.1f, 1000.0f);

    glEnable(GL_DEPTH_TEST);

    auto myfile = std::fstream("saves/cameraState.bin", std::ios::in | std::ios::binary);
    if(myfile)
    {
        CameraState buff;
        myfile.read((char*)&buff, sizeof(CameraState));
        myfile.close();
        camera.setState(buff);
    }

    bool wireframe = false;
    bool vsync = false;
    glfwSwapInterval(0);

    finalProcessingStage = ShaderProgram(
        "shader/post-process/final composing.frag", 
        "shader/post-process/basic.vert", 
        "", 
        globals.standartShaderUniform2D());
    
    finalProcessingStage.addUniform(ShaderUniform(Bloom.getIsEnableAddr(), 10));

    /* 
    ScenePointLight redLight = newPointLight(
        PointLight()
        .setColor(vec3(1, 0, 0))
        .setPosition(vec3(1, 0.5, 0.0))
        .setIntensity(0.75)
        .setRadius(15.0));

    ScenePointLight blueLight = newPointLight(
        PointLight()
        .setColor(vec3(0, 0.5, 1.0))
        .setPosition(vec3(-1, 0.5, 0.0))
        .setIntensity(1.0)
        .setRadius(10.0));

      
    scene.add(redLight);
    scene.add(blueLight);
    

    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
    std::default_random_engine generator;
    for(int i = 0; i < 32; i++)
    // for(int i = 0; i < MAX_LIGHT_COUNTER; i++)
    {
        scene.add(
            newPointLight(
                PointLight()
                .setColor(vec3(randomFloats(generator), randomFloats(generator), randomFloats(generator)))
                .setPosition(vec3(10, 2, 15) * vec3(randomFloats(generator), randomFloats(generator), randomFloats(generator)))
                .setIntensity(0.5)
                .setRadius(10.0)
                )
        );
    }
    */

    MeshMaterial uvPBR(
            new ShaderProgram(
                "shader/foward rendering/uv/CTBRDF.frag", 
                "shader/foward rendering/uv/phong.vert", 
                "", 
                globals.standartShaderUniform3D() 
            ));

    MeshMaterial uvPBRStencil(
            new ShaderProgram(
                "shader/foward rendering/uv/CTBRDF.frag", 
                "shader/foward rendering/uv/phong.vert", 
                "", 
                globals.standartShaderUniform3D() 
            ));

    MeshMaterial uvPhong(
            new ShaderProgram(
                // "shader/foward rendering/uv/phong.frag", 
                "shader/foward rendering/uv/basic.frag", 
                "shader/foward rendering/uv/phong.vert", 
                "", 
                globals.standartShaderUniform3D() 
            ));
    
    ShaderProgram tmpShaderProg;

    MeshMaterial skyboxMaterial(
            new ShaderProgram(
                "shader/foward rendering/uv/Skybox.frag", 
                "shader/foward rendering/uv/phong.vert", 
                "", 
                globals.standartShaderUniform3D() 
            ));

    MeshMaterial uvDepthOnly(
            new ShaderProgram(
                "shader/depthOnly.frag", 
                "shader/foward rendering/uv/phong.vert", 
                ""
                // , 
                // globals.standartShaderUniform3D() 
            ));

    MeshMaterial uvDepthOnlyStencil(
            new ShaderProgram(
                "shader/depthOnlyStencil.frag", 
                "shader/foward rendering/uv/phong.vert", 
                ""
                // , 
                // globals.standartShaderUniform3D() 
            ));

    scene.depthOnlyMaterial = uvDepthOnly;
    uvPBRStencil.depthOnly = uvDepthOnlyStencil;


    ModelRef skybox(new MeshModel3D);
    skybox->setVao(readOBJ("ressources/material demo/skybox/skybox.obj", false));
    skybox->state.scaleScalar(1E6);

    #ifdef CUBEMAP_SKYBOX
        CubeMap skyboxCubeMap;
        skyboxCubeMap.loadAndGenerate("ressources/material demo/cubemap/");

        skybox->setMaterial(MeshMaterial((
            new ShaderProgram(
                "shader/foward rendering/uv/cubeMap.frag", 
                "shader/foward rendering/uv/phong.vert", 
                "", 
                globals.standartShaderUniform3D() 
            ))
        ));

    #else 
        Texture2D skyTexture = Texture2D();
        skybox->setMaterial(skyboxMaterial);

        #ifdef GENERATED_SKYBOX
            skyTexture
                .setResolution(globals.screenResolution())
                .setInternalFormat(GL_SRGB)
                .setFormat(GL_RGB)
                .setPixelType(GL_FLOAT)
                .setFilter(GL_LINEAR)
                .setWrapMode(GL_CLAMP_TO_EDGE)
                .generate();

            SkyboxPass skyboxPass(skyTexture, "nightPixelArt.frag");
            // SkyboxPass skyboxPass(skyTexture, "basic.frag");
            skyboxPass.setup();
        #else
            // skyTexture.loadFromFileKTX("ressources/material demo/skybox/skybox.ktx");
            // skyTexture.loadFromFile("ressources/test/skybox/table_mountain_1_4k.hdr").generate();
            // skyTexture.loadFromFileHDR("ressources/test/skybox/table_mountain_1_4k.hdr").generate();
            // skyTexture.loadFromFile("ressources/test/skybox/puresky2.png").generate();
            // skyTexture.loadFromFile("ressources/test/skybox/shudu_lake_4k.png").generate();
            skyTexture.loadFromFileKTX("ressources/material demo/skybox/8k-puresky.ktx");
            // skyTexture.loadFromFile("ressources/material demo/skybox/8k-cloud-puresky.png");
        #endif

        skybox->setMap(skyTexture, 0);
    #endif
    skybox->invertFaces = true;
    skybox->depthWrite = true;
    skybox->state.frustumCulled = false;
    scene.add(skybox);
    
    SceneDirectionalLight sun = newDirectionLight(
        DirectionLight()
            .setColor(vec3(143, 107, 71)/vec3(255))
            .setDirection(normalize(vec3(-0.454528, -0.707103, 0.541673)))
            .setIntensity(1.0)
            );
    sun->cameraResolution = vec2(2048);
    // sun->cameraResolution = vec2(8192);
    sun->shadowCameraSize = vec2(90, 90);
    sun->activateShadows();

    scene.add(sun);

    SceneDirectionalLight lumiereAppoint = newDirectionLight(
        DirectionLight()
            .setColor(vec3(107, 71, 175)/vec3(255))
            .setIntensity(0.5)
            );

    scene.add(lumiereAppoint);

    ObjectGroupRef materialTesters = newObjectGroup();
    std::vector<MeshVao> mtGeometry = 
    {
        readOBJ("ressources/material demo/sphere.obj"),
        readOBJ("ressources/material demo/d20.obj"),
        readOBJ("ressources/material demo/cube.obj"),
    };

    glLineWidth(3.0);

    #ifdef MATERIAL_TEST
        #define TEST_KTX

        #ifndef TEST_KTX
        std::vector<std::string> mtTextureName
        {
            "ressources/material demo/png/0",
            "ressources/material demo/png/1",
            "ressources/material demo/png/2",
            "ressources/material demo/png/3",
            "ressources/material demo/png/4",
            "ressources/material demo/png/5",
            "ressources/material demo/png/6",
            "ressources/material demo/png/7",
            "ressources/material demo/png/8",
            "ressources/material demo/png/9",
            "ressources/material demo/png/10",
            "ressources/material demo/png/11",
            "ressources/material demo/ktx/12"
        };
        #else
        std::vector<std::string> mtTextureName
        {
            "ressources/material demo/ktx/0",
            "ressources/material demo/ktx/",
            "ressources/material demo/ktx/whiteSandstones",
            "ressources/material demo/ktx/1",
            "ressources/material demo/ktx/2",
            "ressources/material demo/ktx/Scuffed_Gold",
            "ressources/material demo/ktx/StreakyMetal",
            "ressources/material demo/ktx/Aluminum",
            "ressources/material demo/ktx/WornShinyMetal",
            // "ressources/material demo/ktx/QuiltedFabric"
            // "ressources/material demo/ktx/AntiqueGrate",
            
            // "ressources/material demo/ktx/3",
            // "ressources/material demo/ktx/4",
            // "ressources/material demo/ktx/5",
            // "ressources/material demo/ktx/6",
            // "ressources/material demo/ktx/7",
            // "ressources/material demo/ktx/8",
            // "ressources/material demo/ktx/9",
            // "ressources/material demo/ktx/10",
            // "ressources/material demo/ktx/11",
            // "ressources/material demo/ktx/12"
        };
        #endif

        // banger site for textures : https://ambientcg.com/list?type=Material,Atlas,Decal

        BenchTimer mtTimer;
        mtTimer.start();
        for(size_t t = 0; t < mtTextureName.size(); t++)
        {
            Texture2D color;
            Texture2D material;

            #ifndef TEST_KTX
            std::string namec = mtTextureName[t] + "CE.png";
            color.loadFromFile(namec.c_str())
                .setFormat(GL_RGBA)
                .setInternalFormat(GL_SRGB8_ALPHA8)
                .generate();

            std::string namem = mtTextureName[t] + "NRM.png";
            material.loadFromFile(namem.c_str())
                .setFormat(GL_RGBA)
                .setInternalFormat(GL_RGB)
                .generate();
            #else
            std::string namec = mtTextureName[t] + "CE.ktx";
            color.loadFromFileKTX(namec.c_str());
            std::string namem = mtTextureName[t] + "NRM.ktx";
            material.loadFromFileKTX(namem.c_str());
            #endif

            for(size_t g = 0; g < mtGeometry.size(); g++)
            {
                std::shared_ptr<DirectionalLightHelper> helper = std::make_shared<DirectionalLightHelper>(sun);
                helper->state.setPosition(vec3(2.5*g, 0.0, 2.5*t));
                helper->state.scaleScalar(2.0);
                // materialTesters->add(helper);

                ModelRef model = newModel();
                model->setMaterial(uvPBR);
                model->setVao(mtGeometry[g]);

                model->state.setPosition(vec3(2.5*g, 0.0, 2.5*t));
                model->setMap(color, 0);
                model->setMap(material, 1);

                materialTesters->add(model);
            }
        }
        mtTimer.end();
        std::cout 
        << TERMINAL_OK << "Loaded all model images in "
        << TERMINAL_TIMER << mtTimer.getElapsedTime()
        << TERMINAL_OK << " s\n" << TERMINAL_RESET;

        materialTesters->state.setPosition(vec3(0, 0, 5));
        materialTesters->update(true);
        scene.add(materialTesters);

        ModelRef ground = newModel(uvPBR, readOBJ("ressources/material demo/plane.obj"));
        ground->setMap(Texture2D().loadFromFileKTX("ressources/material demo/ktx/QuiltedFabricCE.ktx"), 0);
        ground->setMap(Texture2D().loadFromFileKTX("ressources/material demo/ktx/QuiltedFabricNRM.ktx"), 1);
        ground->state.scaleScalar(45.0).setPosition(vec3(0, -2, 0));
        scene.add(ground);

        ModelRef cofeeCup = newModel(uvPBR, readOBJ("ressources/material demo/cofeeCup/model.obj"));
        cofeeCup->setMap(Texture2D().loadFromFileKTX("ressources/material demo/cofeeCup/CE.ktx"), 0);
        cofeeCup->setMap(Texture2D().loadFromFileKTX("ressources/material demo/cofeeCup/NRM.ktx"), 1);
        cofeeCup->state.scaleScalar(2.0).setPosition(vec3(0, -1, -15)).setRotation(vec3(0, radians(90.f), 0));
        scene.add(cofeeCup);

        ModelRef barberChair = newModel(uvPBR, readOBJ("ressources/material demo/barberChair/model.obj"));
        barberChair->setMap(Texture2D().loadFromFileKTX("ressources/material demo/barberChair/CE.ktx"), 0);
        barberChair->setMap(Texture2D().loadFromFileKTX("ressources/material demo/barberChair/NRM.ktx"), 1);
        barberChair->state.scaleScalar(4.0).setPosition(vec3(0, -1, -10)).setRotation(vec3(0, radians(90.f), 0));
        scene.add(barberChair);

        ModelRef revolver = newModel(uvPBR, readOBJ("ressources/material demo/revolver/model.obj"));
        revolver->setMap(Texture2D().loadFromFileKTX("ressources/material demo/revolver/CE.ktx"), 0);
        revolver->setMap(Texture2D().loadFromFileKTX("ressources/material demo/revolver/NRM.ktx"), 1);
        revolver->state.scaleScalar(15.0).setPosition(vec3(0, 0, -20)).setRotation(vec3(0, 0, 0));
        scene.add(revolver);

        ModelRef sniper = newModel(uvPBR, readOBJ("ressources/material demo/sniper/model.obj"));
        sniper->setMap(Texture2D().loadFromFileKTX("ressources/material demo/sniper/CE.ktx"), 0);
        sniper->setMap(Texture2D().loadFromFileKTX("ressources/material demo/sniper/NRM.ktx"), 1);
        sniper->state.scaleScalar(0.2).setPosition(vec3(0, 1, -30)).setRotation(vec3(0, 0, 0));
        scene.add(sniper);

        ModelRef keyboard = newModel(uvPBR, readOBJ("ressources/material demo/keyboard/model.obj"));
        keyboard->setMap(Texture2D().loadFromFileKTX("ressources/material demo/keyboard/CE.ktx"), 0);
        keyboard->setMap(Texture2D().loadFromFileKTX("ressources/material demo/keyboard/NRM.ktx"), 1);
        keyboard->state.scaleScalar(0.1).setPosition(vec3(0, 1, -5)).setRotation(vec3(0, 0, 0));
        scene.add(keyboard);

        ModelRef leaves = newModel(uvPBRStencil, readOBJ("ressources/material demo/tree/leaves/model.obj"));
        leaves->setMap(Texture2D().loadFromFileKTX("ressources/material demo/tree/leaves/CE.ktx"), 0);
        leaves->setMap(Texture2D().loadFromFileKTX("ressources/material demo/tree/leaves/NRM.ktx"), 1);
        leaves->state.scaleScalar(1.0).setPosition(vec3(0, 0, 0)).setRotation(vec3(0, 0, 0));
        leaves->noBackFaceCulling = true;
        scene.add(leaves);

    #else
    #endif

    FontRef font(new FontUFT8);
    font->readCSV("ressources/fonts/Roboto/out.csv");
    font->setAtlas(Texture2D().loadFromFileKTX("ressources/fonts/Roboto/out.ktx"));

    MeshMaterial defaultFontMaterial(
        new ShaderProgram(
            "shader/2D/sprite.frag",
            "shader/2D/sprite.vert",
            "",
            globals.standartShaderUniform3D()
        ));

    std::shared_ptr<SingleStringBatch> ssb(new SingleStringBatch);
    ssb->setFont(font);
    ssb->setMaterial(defaultFontMaterial);

    MeshMaterial defaultSUIMaterial(
        new ShaderProgram(
            "shader/2D/fastui.frag",
            "shader/2D/fastui.vert",
            "",
            globals.standartShaderUniform3D()
        ));

    SimpleUiTileBatchRef uiBatch(new SimpleUiTileBatch);
    
    uiBatch->setMaterial(defaultSUIMaterial);
    uiBatch->state.position.z = 0.0;
    uiBatch->state.forceUpdate();

    FastUI_context ui(uiBatch, font, scene2D, defaultFontMaterial);

    // float test2 = 0.5;
    // int test3 = 512; 
    FastUI_valueMenu menu(ui, {
        // {FastUI_menuTitle(ui, U"Infos"), FastUI_valueTab(ui, {
        //     FastUI_value(globals.appTime.getElapsedTimeAddr(), U"App Time\t", U" s"),
        //     FastUI_value(globals.simulationTime.getElapsedTimeAddr(), U"Simulation Time\t", U" s"),
        // })}, 
        // {FastUI_menuTitle(ui, U"Titre 2"), FastUI_valueTab(ui, {
        //     FastUI_value(&test2, U"value4\t", U" ms "),
        //     FastUI_value(&test2, U"value4\t", U" ° ", FUI_floatAngle),
        //     FastUI_value(&test3, U"value5\t", U" units "),
            // FastUI_value(&test2, U"value5 : ", U" s "),
            // FastUI_value(&test2, U"value6 : ", U" m/s "),
        // })}  

    });

    menu->state.setPosition(vec3(-0.9, 0.5, 0)).scaleScalar(0.95); //0.65

    sun->setMenu(menu, U"Sun");
    lumiereAppoint->setMenu(menu, U"Lum.Appoint");

    globals.appTime.setMenuConst(menu);
    // globals.simulationTime.setMenu(menu);
    globals.cpuTime.setMenu(menu);
    globals.gpuTime.setMenu(menu);
    globals.fpsLimiter.setMenu(menu);

    globals.fpsLimiter.activate();
    globals.fpsLimiter.freq = 144.f;

    menu.batch();
    scene2D.updateAllObjects();
    uiBatch->batch();
    
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 


    // AudioFile test;
    // test.loadOGG("ressources/sounds/Clouded Thoughts by Wakapedia.ogg");

    // AudioSource source;
    // source
    //     .setBuffer(test.getHandle())
    //     .play();

    while(state != quit)
    {
        mainloopStartRoutine();

        camera.updateMouseFollow(window);

        GLFWKeyInfo input;
        while(inputs.pull(input))
        {
            if(input.action == GLFW_RELEASE)
            switch (input.key)
            {
            case GLFW_MOUSE_BUTTON_LEFT:
                globals._mouseLeftClickDown = false;
                break;
            
            default:
                break;
            }

            if(input.action == GLFW_PRESS)
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
                finalProcessingStage.reset();
                uvPBR->reset();
                skybox->getMaterial()->reset();
                ssb->getMaterial()->reset();
                defaultSUIMaterial->reset();

                #ifdef GENERATED_SKYBOX
                    skyboxPass.getShader().reset();
                #endif

                break;
            
            case GLFW_KEY_F2:
                camera.toggleMouseFollow();
                if(camera.getMouseFollow())
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                else 
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

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
            
            case GLFW_KEY_3 : 
                tmpShaderProg = *(uvPBR.get());
                *(uvPBR.get()) = *(uvPhong.get());
                *(uvPhong.get()) = tmpShaderProg;
                break;

            case GLFW_KEY_F9 : 
                vsync = !vsync;
                glfwSwapInterval(vsync ? 1 : 0);
                break;
            
            case GLFW_KEY_TAB :
                globals.simulationTime.toggle();
                break;

            case GLFW_MOUSE_BUTTON_LEFT :
                globals._mouseLeftClick = true;
                globals._mouseLeftClickDown = true;
                break;
            
            case GLFW_KEY_V :
                if(input.mods&GLFW_MOD_CONTROL)
                {
                    if(globals.isTextInputsActive())
                        globals.textInputString += UFTconvert.from_bytes(glfwGetClipboardString(window));

                    // std::cout << glfwGetClipboardString(window);
                }
                break;
            
            case GLFW_KEY_ENTER :
                if(globals.isTextInputsActive())
                    globals.textInputString.push_back(U'\n');
                break;

            case GLFW_KEY_DELETE :
            case GLFW_KEY_BACKSPACE :
                if(globals.isTextInputsActive() && !globals.textInputString.empty())
                    globals.textInputString.pop_back();
                break;

            }
        }

        mainInput(globals.appTime.getDelta());

        mainloopPreRenderRoutine();

        #ifdef GENERATED_SKYBOX
            skyboxPass.render(camera);
        #endif

        if(wireframe)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        menu.trackCursor();
        menu.updateText();

        glEnable(GL_BLEND); 
        scene2D.updateAllObjects();
        uiBatch->batch();
        glEnable(GL_FRAMEBUFFER_SRGB);
        screenBuffer2D.activate();
        uiBatch->draw();
        scene2D.draw(); // GL error GL_INVALID_OPERATION in (null): (ID: 173538523)
        screenBuffer2D.deactivate();
        glDisable(GL_FRAMEBUFFER_SRGB);
        glDisable(GL_BLEND);

        scene.updateAllObjects();

        #ifdef INVERTED_Z
        glDepthFunc(GL_GREATER);
        #else
        glDepthFunc(GL_LESS);
        #endif
        
        scene.generateShadowMaps(); // GL error GL_INVALID_OPERATION in (null): (ID: 173538523)

        renderBuffer.activate();
        scene.depthOnlyDraw(camera, true);
        glDepthFunc(GL_EQUAL);

        #ifdef CUBEMAP_SKYBOX
            skyboxCubeMap.bind();
        #else
            skyTexture.bind(4);
        #endif
        scene.genLightBuffer();
        scene.draw(); // GL error GL_INVALID_OPERATION in (null): (ID: 173538523)
        renderBuffer.deactivate();

        renderBuffer.bindTextures();
        
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        SSAO.render(camera); // GL error GL_INVALID_OPERATION in (null): (ID: 173538523)
        Bloom.render(camera);

        glViewport(0, 0, globals.windowWidth(), globals.windowHeight());
        // sun->shadowMap.bindTexture(0, 6);
        skyTexture.bind(6);
        screenBuffer2D.bindTexture(0, 7);
        finalProcessingStage.activate();
        globals.drawFullscreenQuad();
 
        sun->shadowCamera.setPosition(camera.getPosition());

        mainloopEndRoutine();
    }

    // alCall(alDeleteSources, 1, &source.getHandle());
    // alCall(alDeleteBuffers, 1, (ALuint*)&test);
}