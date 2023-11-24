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

#include <Fonts.hpp>

#ifdef DEMO_MAGE_BATTLE
    #include <demos/Mage_Battle/Team.hpp>
#endif

// #include <sstream>
// #include <iomanip>
// #include <codecvt>

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

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    
    glfwSetCursorPosCallback(window,[](GLFWwindow* window, double dx, double dy)
    {
        if(globals.currentCamera->getMouseFollow())
        {
            vec2 center(globals.windowWidth()*0.5, globals.windowHeight()*0.5);
            vec2 sensibility(50.0);
            vec2 dir = sensibility * (vec2(dx, dy)-center)/center;

            float yaw = radians(-dir.x);
            float pitch = radians(-dir.y);

            vec3 up = vec3(0,1,0);
            vec3 front = mat3(rotate(mat4(1), yaw, up)) * globals.currentCamera->getDirection();
            front = mat3(rotate(mat4(1), pitch, cross(front, up))) * front;
            front = normalize(front);

            front.y = clamp(front.y, -0.9f, 0.9f);
            globals.currentCamera->setDirection(front);

            glfwSetCursorPos(window, center.x, center.y);
        }
    });
}

void App::mainInput(double deltatime)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        state = quit;
    
    float camspeed = 2.0;
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
    globals.unpausedTime.start();
}

void App::mainloopPreRenderRoutine()
{
    camera.updateProjectionViewMatrix();
}

void App::mainloopEndRoutine()
{
    glfwSwapBuffers(window);
    globals.appTime.end();
    globals.unpausedTime.end();
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

    Scene scene;
    Scene scene2D;

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

    MeshMaterial uvDepthOnly(
            new ShaderProgram(
                "shader/depthOnly.frag", 
                "shader/foward rendering/uv/phong.vert", 
                "", 
                globals.standartShaderUniform3D() 
            ));
    
    scene.depthOnlyMaterial = uvDepthOnly;

    ModelRef skybox = newModel(
        uvBasic, 
        readOBJ("ressources/test/skybox/skybox.obj", false),
        ModelState3D()
            .scaleScalar(10000000.0)
            .setPosition(vec3(0.0, 0.0, 0.0)));

    Texture2D skyTexture = Texture2D();
    
    // #define GENERATED_SKYBOX

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
        skyTexture.loadFromFile("ressources/test/skybox/puresky2.png").generate();
    #endif

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
    
    SceneDirectionalLight sun = newDirectionLight(
        DirectionLight()
            .setColor(vec3(143, 107, 71)/vec3(255))
            .setDirection(normalize(vec3(-1.0, -1.0, 0.0)))
            .setIntensity(1.0)
            );
    // sun->cameraResolution = vec2(2048);
    sun->cameraResolution = vec2(8192);
    sun->shadowCameraSize = vec2(90, 90);
    // sun->activateShadows();
    scene.add(sun);
    


    ObjectGroupRef materialTesters = newObjectGroup();
    std::vector<MeshVao> mtGeometry = 
    {
        readOBJ("ressources/material demo/sphere.obj"),
        readOBJ("ressources/material demo/d20.obj"),
        readOBJ("ressources/material demo/cube.obj"),
    };

    glLineWidth(15.0);

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
            "ressources/material demo/ktx/1",
            "ressources/material demo/ktx/2",
            "ressources/material demo/ktx/3",
            "ressources/material demo/ktx/4",
            "ressources/material demo/ktx/5",
            "ressources/material demo/ktx/6",
            "ressources/material demo/ktx/7",
            "ressources/material demo/ktx/8",
            "ressources/material demo/ktx/9",
            "ressources/material demo/ktx/10",
            "ressources/material demo/ktx/11",
            "ressources/material demo/ktx/12"
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
                .setInternalFormat(GL_SRGB8_ALPHA8)
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
                materialTesters->add(helper);

                ModelRef model = newModel();
                model->setMaterial(uvPhong);
                model->setVao(mtGeometry[g]);

                model->state.setPosition(vec3(2.5*g, 0.0, 2.5*t));
                model->setMap(color, 0);
                model->setMap(material, 1);

                materialTesters->add(model);
                scene.add(model);
            }
        }
        mtTimer.end();
        std::cout 
        << TERMINAL_OK << "Loaded all model images in "
        << TERMINAL_TIMER << mtTimer.getElapsedTime()
        << TERMINAL_OK << " s\n" << TERMINAL_RESET;

        materialTesters->update(true);
        scene.add(materialTesters);
    #else
    
    #ifdef DEMO_MAGE_BATTLE
        ModelRef ground = newModel(uvPhong, mtGeometry[2]);
        ground->setMap(Texture2D().loadFromFileKTX("ressources/material demo/ktx/2CE.ktx"), 0);
        ground->setMap(Texture2D().loadFromFileKTX("ressources/material demo/ktx/2NRM.ktx"), 1);
        ground->state.setScale(vec3(ARENA_RADIUS*2.0, 0.5, ARENA_RADIUS*2.0)).setPosition(vec3(0, -0.5, 0));
        scene.add(ground);
        
 
        // SceneTubeLight test = newTubetLight();
        // test->setColor(vec3(0, 0.5, 1.0))
        //     .setIntensity(1.0)
        //     .setRadius(3.0)
        //     .setPos(vec3(-2, 0, -2), vec3(2, 0, 2));
        // scene.add(test);
        // scene.add(std::make_shared<TubeLightHelper>(test));


        MeshMaterial MageMaterial(
                new ShaderProgram(
                    "shader/demos/Mage_Battle/Mage.frag", 
                    "shader/demos/Mage_Battle/Mage.vert", 
                    "", 
                    globals.standartShaderUniform3D() 
                ),
                new ShaderProgram(
                    "shader/demos/Mage_Battle/MageDepthOnly.frag", 
                    "shader/demos/Mage_Battle/Mage.vert", 
                    ""
                    , globals.standartShaderUniform3D() 
                )     
                );

        ModelRef MageTestModelAttack = newModel(MageMaterial, mtGeometry[1]);
        MageTestModelAttack->setMap(Texture2D().loadFromFileKTX("ressources/material demo/ktx/0CE.ktx"), 0)
            .setMap(Texture2D().loadFromFileKTX("ressources/material demo/ktx/0NRM.ktx"), 1);
        MageTestModelAttack->state.scaleScalar(0.5);

        ModelRef MageTestModelHeal = newModel(MageMaterial, mtGeometry[0]);
        MageTestModelHeal->setMap(Texture2D().loadFromFileKTX("ressources/material demo/ktx/0CE.ktx"), 0)
            .setMap(Texture2D().loadFromFileKTX("ressources/material demo/ktx/0NRM.ktx"), 1);
        MageTestModelHeal->state.scaleScalar(0.35);

        ModelRef MageTestModelTank = newModel(MageMaterial, mtGeometry[2]);
        MageTestModelTank->setMap(Texture2D().loadFromFileKTX("ressources/material demo/ktx/0CE.ktx"), 0)
            .setMap(Texture2D().loadFromFileKTX("ressources/material demo/ktx/0NRM.ktx"), 1);
        MageTestModelTank->state.scaleScalar(0.5);

        // MageRef MageTest = SpawnNewMage(MageTestModel, vec3(0), vec3(0), DEBUG);
        // scene.add(MageTest->getModel());

        Team::healModel = MageTestModelHeal;
        Team::attackModel = MageTestModelAttack;
        Team::tankModel = MageTestModelTank; 

        int unitsNB = 10;
        int healNB = unitsNB*0.2f;
        int attackNB = unitsNB*0.7f;
        int tankNB = unitsNB*0.1f;

        Team red;
        red.SpawnUnits(scene, healNB, attackNB, tankNB, vec3(-ARENA_RADIUS*0.5, 0, ARENA_RADIUS*0.5), ARENA_RADIUS*0.4, vec3(0xCE, 0x20, 0x29)/vec3(255.f));

        Team blue;
        blue.SpawnUnits(scene, healNB, attackNB, tankNB, vec3(ARENA_RADIUS*0.5, 0, -ARENA_RADIUS*0.5), ARENA_RADIUS*0.4, vec3(0x28, 0x32, 0xC2)/vec3(255.f));

        Team yellow;
        yellow.SpawnUnits(scene, healNB, attackNB, tankNB, vec3(ARENA_RADIUS*0.5, 0, ARENA_RADIUS*0.5), ARENA_RADIUS*0.4, vec3(0xFD, 0xD0, 0x17)/vec3(255.f));

        Team green;
        green.SpawnUnits(scene, healNB, attackNB, tankNB, vec3(-ARENA_RADIUS*0.5, 0, -ARENA_RADIUS*0.5), ARENA_RADIUS*0.4, vec3(0x3C, 0xB0, 0x43)/vec3(255.f));

        Team magenta;
        // magenta.SpawnUnits(scene, healNB, attackNB, tankNB, vec3(0, 0, 0), ARENA_RADIUS*0.3, vec3(0xE9, 0x2C, 0x91)/vec3(255.f));


        glLineWidth(3.0);
        globals.unpausedTime.pause();
    #endif
    
    #endif

    FontRef font(new FontUFT8);
    font->readCSV("ressources/fonts/MorkDungeon/out.csv");
    font->setAtlas(Texture2D().loadFromFileKTX("ressources/fonts/MorkDungeon/out.ktx"));

    MeshMaterial defaultFontMaterial(
        new ShaderProgram(
            "shader/2D/sprite.frag",
            "shader/2D/sprite.vert",
            "",
            globals.standartShaderUniform3D()
        ));

    std::shared_ptr<SingleStringBatch> ssb(new SingleStringBatch);
    ssb->setFont(font);;
    ssb->setMaterial(defaultFontMaterial);

    ssb->state.setPosition(vec3(-0.95, 0.0, 0.f));
    vec3 timerColor = vec3(0x9A, 0x7B, 0x4F)/vec3(256.f);
    ssb->uniforms.add(ShaderUniform(&timerColor, 32));
    scene2D.add(ssb);

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
                ssb->getMaterial()->reset();

                #ifdef GENERATED_SKYBOX
                    skyboxPass.getShader().reset();
                #endif

                #ifdef DEMO_MAGE_BATTLE
                    MageMaterial->reset();
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

            case GLFW_KEY_F9 : 
                vsync = !vsync;
                glfwSwapInterval(vsync ? 1 : 0);
                break;
            
            case GLFW_KEY_TAB :
                globals.unpausedTime.toggle();
                break;

            default:
                break;
            }
        }

        float time = globals.unpausedTime.getElapsedTime();
        // sun->setDirection(normalize(vec3(0.5, -abs(cos(time*0.25)), sin(time*0.25))));

        mainInput(globals.appTime.getDelta());

        mainloopPreRenderRoutine();

        #ifdef GENERATED_SKYBOX
            skyboxPass.render(camera);
        #endif

        if(wireframe)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


        #ifdef DEMO_MAGE_BATTLE
            red.tick();
            blue.tick();
            yellow.tick();
            green.tick();
            magenta.tick();
        #endif

        ssb->text = U"time : " + UFTconvert.from_bytes(std::to_string((int)globals.unpausedTime.getElapsedTime()));
        ssb->batchText();


        scene2D.updateAllObjects();
        glEnable(GL_FRAMEBUFFER_SRGB);
        screenBuffer2D.activate();
        scene2D.draw();
        screenBuffer2D.deactivate();
        glDisable(GL_FRAMEBUFFER_SRGB);

        scene.updateAllObjects();
        scene.generateShadowMaps();
        renderBuffer.activate();
        skyTexture.bind(4);
        //sun->shadowMap.bindTexture(0, 2);
        scene.genLightBuffer();
        scene.draw();
        renderBuffer.deactivate();

        renderBuffer.bindTextures();
        
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        SSAO.render(camera);
        Bloom.render(camera);

        glViewport(0, 0, globals.windowWidth(), globals.windowHeight());
        // sun->shadowMap.bindTexture(0, 6);
        screenBuffer2D.bindTexture(0, 7);
        PostProcessing.activate();
        globals.drawFullscreenQuad();
 
        mainloopEndRoutine();
    }
}