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

    renderBuffer.generate();
    SSAO.setup();
    Bloom.setup();
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
        // readOBJ("ressources/cube.obj"),
        // readOBJ("ressources/fox.obj"),
        // readOBJ("ressources/nerd.obj"),
        // readOBJ("ressources/sword.obj"),
        // readOBJ("ressources/airboat.obj"),
        // readOBJ("ressources/cowgirl.obj"),
        // readOBJ("ressources/wraith.obj"),
        readOBJ("ressources/female.obj"),
    };

    uint64 TemplateId = 0;

    std::vector<MeshVao> TemplateMeshesFlatShade =
    {
        readOBJ("ressources/sphere_flatshade.obj"),
        readOBJ("ressources/hardsurface_flatshade.obj"),
        readOBJ("ressources/voronoi_flatshade.obj"),
        // readOBJ("ressources/cube_flatshade.obj"),
        // readOBJ("ressources/fox_flatshade.obj"),
        // readOBJ("ressources/nerd_flatshade.obj"),
        // readOBJ("ressources/sword_flatshade.obj"),
        // readOBJ("ressources/airboat_flatshade.obj"),
        // readOBJ("ressources/cowgirl_flatshade.obj"),
        // readOBJ("ressources/wraith_flatshade.obj"),
        readOBJ("ressources/female_flatshade.obj"),
    };

    std::vector<MeshMaterial> materials = 
    {
        // MeshMaterial(
        //     new ShaderProgram(
        //         "shader/foward rendering/basic.frag", 
        //         "shader/foward rendering/basic.vert", 
        //         "", 
        //         globals.standartShaderUniform3D())), 

        // MeshMaterial(
        //     new ShaderProgram(
        //         "shader/foward rendering/gouraud.frag", 
        //         "shader/foward rendering/gouraud.vert", 
        //         "", 
        //         globals.standartShaderUniform3D())), 

        MeshMaterial(
            new ShaderProgram(
                "shader/foward rendering/phong.frag", 
                "shader/foward rendering/phong.vert", 
                "", 
                globals.standartShaderUniform3D())), 

        // MeshMaterial(
        //     new ShaderProgram(
        //         "shader/foward rendering/toon.frag", 
        //         "shader/foward rendering/toon.vert", 
        //         "", 
        //         globals.standartShaderUniform3D())),            
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


    // scene.add(room, false);
    // scene.add(model, false);
    // scene.add(model2, false);

    bool wireframe = false;

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

    // scene.add(redLight);
    // scene.add(blueLight);

    // std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
    // std::default_random_engine generator;
    // for(int i = 0; i < 16; i++)
    // for(int i = 0; i < MAX_LIGHT_COUNTER; i++)
    // {
    //     scene.add(
    //         newPointLight(
    //             PointLight()
    //             .setColor(vec3(randomFloats(generator), randomFloats(generator), randomFloats(generator)))
    //             .setPosition(vec3(4, 2, 4) * vec3(0.5 - randomFloats(generator), randomFloats(generator), 0.5 - randomFloats(generator)))
    //             .setIntensity(1.75)
    //             .setRadius(5.0)
    //             )
    //     );
    // }

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

    Texture2D test = Texture2D().loadFromFile("ressources/test/jug.jpg").generate();

    ModelRef jug = newModel(
        uvPhong, 
        readOBJ("ressources/test/jug.obj", false),
        ModelState3D()
            .scaleScalar(5.0)
            .setPosition(vec3(0.0, 0.0, 0.0)));
    
    jug->setColorMap(test);

    ModelRef barberShopChair = newModel(
        uvPhong, 
        readOBJ("ressources/test/BarberShopChair.obj", false),
        ModelState3D()
            .scaleScalar(1.0)
            .setPosition(vec3(2.0, 0.0, 0.0)));
    
    barberShopChair->setColorMap(
        Texture2D()
            .loadFromFile("ressources/test/BarberShopChair.jpg")
            .generate());

    ModelRef woman = newModel(
        uvPhong, 
        readOBJ("ressources/test/woman/woman.obj", false),
        ModelState3D()
            .scaleScalar(1.0)
            .setPosition(vec3(-2.0, 0.0, 0.0)));
    
    woman->setColorMap(
        Texture2D()
            .loadFromFile("ressources/test/woman/color.jpg")
            .generate());

    ModelRef guitar = newModel(
        uvPhong, 
        readOBJ("ressources/test/guitar/guitar.obj", false),
        ModelState3D()
            .scaleScalar(2.0)
            .setPosition(vec3(4.0, 0.0, 0.0)));
    
    guitar->setColorMap(
        Texture2D()
            .loadFromFile("ressources/test/guitar/color.png")
            .generate());

    ModelRef skybox = newModel(
        uvBasic, 
        readOBJ("ressources/test/skybox/skybox.obj", false),
        ModelState3D()
            .scaleScalar(10.0)
            .setPosition(vec3(0.0, 0.0, 0.0))
            .setRotation(vec3(0, 90, 0)));
    
    Texture2D skyTexture =Texture2D()
            .loadFromFile("ressources/test/skybox/puresky.hdr")
            .generate();

    skybox->setColorMap(skyTexture);

    skybox->invertFaces = true;
    skybox->depthWrite = false;

    scene.add(skybox);
    // scene.add(jug);
    // scene.add(barberShopChair);
    // scene.add(woman);
    // scene.add(guitar);

    ObjectGroupRef group = newObjectGroup();
    ObjectGroupRef group2 = newObjectGroup();
    group->add(jug);
    group->add(barberShopChair);
    group->add(woman);
    group->add(guitar);

    ScenePointLight testLight = newPointLight(
        PointLight()
        .setColor(vec3(1.0, 0.2, 0.0))
        .setIntensity(2.0)
        .setRadius(3.0)
        .setPosition(vec3(3, 1, 0))
        // .setPosition(barberShopChair->state.position)
        );

    SceneDirectionalLight sun = newDirectionLight(
        DirectionLight()
            .setColor(vec3(1.0, 0.95, 0.85))
            .setDirection(normalize(vec3(1.0, 0.3, 1.0)))
            .setIntensity(0.5)
            );

    // group->add(testLight);
    group->add(sun);
    group->add(
        newModel(
            // materials[0],
            uvPhong,
            readOBJ("ressources/plane.obj"),
            ModelState3D()
                .scaleScalar(0.1)
        )
    );

    scene.add(group);
    scene.add(group2);


    ObjectGroupRef helper = std::make_shared<PointLightHelper>(PointLightHelper(testLight));
    // helper->state.scaleScalar(50.0); 
    scene.add(helper);


    scene.add(std::make_shared<DirectionalLightHelper>(DirectionalLightHelper(sun)));


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
                for(uint64 i = 0; i < materials.size(); i++)
                    materials[i]->reset();
                jug->getMaterial()->reset();
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

            case GLFW_KEY_1 :
                SSAO.toggle();
                break;
            
            case GLFW_KEY_2 :
                Bloom.toggle();
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
        // group->state.setRotation(vec3(0.0, globals.appTime.getElapsedTime()*0.5, 0.0));
        // group2->state.setRotation(vec3(0.0, globals.appTime.getElapsedTime()*0.5, 0.0));
        testLight->setColor(vec3(0.5) + vec3(0.5)*cos(vec3(globals.appTime.getElapsedTime())*vec3(0.5, 1.2, 3.5)));
        // sun->setDirection(normalize(vec3(sin(globals.appTime.getElapsedTime()*0.5), cos(globals.appTime.getElapsedTime()*0.5), 0)));
        // group->state.setPosition(vec3(0, 0, 0.001) + group->state.position);

        renderBuffer.activate();
        // ligthBuffer.activate(0);
        skyTexture.bind(1);
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