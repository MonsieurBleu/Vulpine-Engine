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

    // screenBuffer2D
    //     .addTexture(
    //         Texture2D().
    //             setResolution(globals.windowSize())
    //             .setInternalFormat(GL_SRGB8_ALPHA8)
    //             .setFormat(GL_RGBA)
    //             .setPixelType(GL_UNSIGNED_BYTE)
    //             .setFilter(GL_LINEAR)
    //             .setWrapMode(GL_CLAMP_TO_EDGE)
    //             .setAttachement(GL_COLOR_ATTACHMENT0))
    //     // .addTexture(
    //     //     Texture2D() 
    //     //         .setResolution(globals.windowSize())
    //     //         .setInternalFormat(GL_DEPTH_COMPONENT)
    //     //         .setFormat(GL_DEPTH_COMPONENT)
    //     //         .setPixelType(GL_UNSIGNED_BYTE)
    //     //         .setFilter(GL_LINEAR)
    //     //         .setWrapMode(GL_CLAMP_TO_EDGE)
    //     //         .setAttachement(GL_DEPTH_ATTACHMENT))
    //     .generate();

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


struct VoxelInfos
{
    vec3 position;
    float scale;
};



vec3 v[8] =
{
    vec3(0.500000, 0.500000, -0.500000),
    vec3(0.500000, -0.500000, -0.500000),
    vec3(0.500000, 0.500000, 0.500000),
    vec3(0.500000, -0.500000, 0.500000),
    vec3(-0.500000, 0.500000, -0.500000),
    vec3(-0.500000, -0.500000, -0.500000),
    vec3(-0.500000, 0.500000, 0.500000),
    vec3(-0.500000, -0.500000, 0.500000),
};

vec3 vn[6]
{
    vec3(-0.0000, 1.0000, -0.0000),
    vec3(-0.0000, -0.0000, 1.0000),
    vec3(-1.0000, -0.0000, -0.0000),
    vec3(-0.0000, -1.0000, -0.0000),
    vec3(1.0000, -0.0000, -0.0000),
    vec3(-0.0000, -0.0000, -1.0000)
};

vec3 voxelModelPositions[36] = 
{
    v[4], v[2], v[0],
    v[2], v[7], v[3],
    v[6], v[5], v[7],
    v[1], v[7], v[5],
    v[0], v[3], v[1],
    v[4], v[1], v[5],
    v[4], v[6], v[2],
    v[2], v[6], v[7],
    v[6], v[4], v[5],
    v[1], v[3], v[7],
    v[0], v[2], v[3],
    v[4], v[0], v[1]
};

vec3 voxelModelNormales[36] = 
{
    vn[0], vn[0], vn[0],
    vn[1], vn[1], vn[1],
    vn[2], vn[2], vn[2],
    vn[3], vn[3], vn[3],
    vn[4], vn[4], vn[4],
    vn[5], vn[5], vn[5],
    vn[0], vn[0], vn[0],
    vn[1], vn[1], vn[1],
    vn[2], vn[2], vn[2],
    vn[3], vn[3], vn[3],
    vn[4], vn[4], vn[4],
    vn[5], vn[5], vn[5]
};

MeshVao genVaoFromVoxels(std::vector<VoxelInfos> vox)
{
    const int vertexPerVoxel = 36;
    int size = vox.size();
    GenericSharedBuffer pos(new char[sizeof(vec3)*vertexPerVoxel*size]);
    GenericSharedBuffer nor(new char[sizeof(vec3)*vertexPerVoxel*size]);
    GenericSharedBuffer uvs(new char[sizeof(vec2)*vertexPerVoxel*size]);
    vec3 *p = (vec3*) pos.get();
    vec3 *n = (vec3*) nor.get();
    vec2 *u = (vec2*) uvs.get();  
    
    for(int i = 0; i < size; i++)
    {
        int id = i*vertexPerVoxel;

        for(int j = 0; j < vertexPerVoxel; j++)
        {
            p[id+j] = (voxelModelPositions[j]*vox[i].scale) + vox[i].position;
            n[id+j] = voxelModelNormales[j];
            u[id+j] = vec2(0);
        }
    }

    MeshVao res(
        new VertexAttributeGroup({
            VertexAttribute(pos, 0, size*vertexPerVoxel, 3, GL_FLOAT, false),
            VertexAttribute(nor, 1, size*vertexPerVoxel, 3, GL_FLOAT, false),
            VertexAttribute(uvs, 2, size*vertexPerVoxel, 2, GL_FLOAT, false)
        })
    );

    res->generate();

    return res;
}

void VoxelAddSimpleCube(std::vector<VoxelInfos> &v, vec3 pos, float scale)
{
    v.push_back(VoxelInfos{vec3(0), 1});
}

void VoxelAddSphereSP(std::vector<VoxelInfos> &v, vec3 pos, float radius, int precision = 32)
{
    float voxelScale = (radius*2.f)/precision;
    for(int i = 0; i < precision; i++)
    for(int j = 0; j < precision; j++)
    for(int k = 0; k < precision; k++)
    {
        vec3 p = vec3(1.0) - vec3((float)i, (float)j, (float)k)/vec3(precision*0.5);

        if(dot(p, p) <= 1.f)
            v.push_back(VoxelInfos{(p*radius)+pos, voxelScale*0.5f});
    }
}

void VoxelAddTubeSP(std::vector<VoxelInfos> &v, vec3 pos, vec3 dir, float radius, int precision = 32)
{
    float voxelScale = (radius*2.f)/precision;
    for(int i = 0; i < precision; i++)
    for(int j = 0; j < precision; j++)
    for(int k = 0; k < precision; k++)
    {
        vec3 p = vec3(1.0) - vec3((float)i, (float)j, (float)k)/vec3(precision*0.5);
        vec3 tmp = cross(p, dir);

        if(dot(tmp, tmp) <= 1.f)
            v.push_back(VoxelInfos{(p*radius)+pos, voxelScale*0.5f});
    }
}

enum CombinaisonType
{
    SUBSTRACTION,
    UNION,
    INTERSECTION
};

void VoxelAddTubeSphereCombinaison(
    std::vector<VoxelInfos> &v, 
    vec3 TubePos, vec3 TubeDir, float TubeRadius, 
    vec3 SpherePos, float SphereRadius, 
    float scale,
    int precision,
    CombinaisonType type)
{
    float voxelScale = (scale*2.f)/precision;
    for(int i = 0; i < precision; i++)
    for(int j = 0; j < precision; j++)
    for(int k = 0; k < precision; k++)
    {
        vec3 p = vec3(1.0) - vec3((float)i, (float)j, (float)k)/vec3(precision*0.5);
        p *= scale;

        bool sphere = distance(p, SpherePos) <= SphereRadius;

        bool tube = length(cross(p-TubePos, TubeDir)) <= TubeRadius;

        bool b = false;

        switch (type)
        {
        case UNION : b = sphere || tube; break;
        case INTERSECTION : b = sphere && tube; break;
        case SUBSTRACTION : b = sphere && !tube; break;
        
        default:
            break;
        }

        if(b)
            v.push_back(VoxelInfos{p, voxelScale});
    }
}


struct OctreeInfo
{
    vec3 position;
    float scale;
    int depth = 0;
};

void VoxelAddSphereOctree(std::vector<VoxelInfos> &v, vec3 pos, float radius, int maxDepth, int minDepth)
{
    std::list<OctreeInfo> o;

    o.push_back(OctreeInfo{vec3(0), 1, 0});

    while(!o.empty())
    {
        OctreeInfo node = o.front();
        o.pop_front();

        int nbPushed = 0;

        for(int i = -1; i < 2; i+=2)
        for(int j = -1; j < 2; j+=2)
        for(int k = -1; k < 2; k+=2)
        {
            vec3 p = node.position + (vec3(i, j, k)*vec3(node.scale*0.5));
            
            if(length(p) <= 1.0 || node.depth < minDepth)
            {
                if(node.depth < maxDepth)
                {
                    o.push_back(OctreeInfo{p, node.scale*0.5f, node.depth+1});
                    nbPushed++;
                }
                else if(node.depth == maxDepth)
                {
                    v.push_back(VoxelInfos{(p*radius) + pos, 0.01f * (radius/node.scale)});
                }
            }
        }

        if(nbPushed == 8 &&  node.depth >= minDepth)
        {
            for(int i = 0; i < 8; i++)
                o.pop_back();

            v.push_back(VoxelInfos{(node.position*radius) + pos, 0.01f * 2.f*(radius/node.scale)});
        }
    }

    std::cout << v.size() << "\n";
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
    
    SceneDirectionalLight sun = newDirectionLight(
        DirectionLight()
            .setColor(vec3(143, 107, 71)/vec3(255))
            .setDirection(normalize(vec3(-1.0, -1.0, 0.0)))
            .setIntensity(1.0)
            );
    // sun->cameraResolution = vec2(2048);
    sun->cameraResolution = vec2(8192);
    sun->shadowCameraSize = vec2(90, 90);
    sun->activateShadows();
    scene.add(sun);
    


    ObjectGroupRef materialTesters = newObjectGroup();
    std::vector<MeshVao> mtGeometry = 
    {
        readOBJ("ressources/material demo/sphere.obj"),
        readOBJ("ressources/material demo/d20.obj"),
        readOBJ("ressources/material demo/cube.obj"),
    };

    glLineWidth(15.0);


    // MeshVao cube = readOBJ("ressources/material demo/cube.obj");
    // vec3 cube->attributes[0].buffer.get();

    ModelRef surface(new MeshModel3D);

    surface->setMaterial(uvPhong);
    surface->setMap(
        Texture2D()
            .loadFromFile("ressources/voxel/color.png")
            .setFormat(GL_RGBA)
            .setInternalFormat(GL_SRGB8_ALPHA8)
            .generate(),
        0);

    surface->setMap(
        Texture2D()
            .loadFromFile("ressources/voxel/NRM.png")
            .setFormat(GL_RGBA)
            .setInternalFormat(GL_SRGB8_ALPHA8)
            .generate(),
        1);
    
    std::vector<VoxelInfos> v;

    // VoxelAddSphereSP(v, vec3(0), 5.f, 18);
    VoxelAddSphereOctree(v, vec3(0), 5.f, 3, 1);

    // VoxelAddTubeSP(v, vec3(0), vec3(0, 1, 0), 5.f, 16);

    // VoxelAddTubeSphereCombinaison(
    //     v, 
    //     vec3(0), normalize(vec3(0, 1.0, 1.0)), 3.0f,
    //     vec3(0), 5.0f,
    //     5.f,
    //     16,
    //     INTERSECTION
    // );

    surface->setVao(genVaoFromVoxels(v));

    // surface->defaultMode = GL_LINES;

    scene.add(surface);



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

        float time = globals.unpausedTime.getElapsedTime()*0.25;
        sun->setDirection(normalize(vec3(0.5, -abs(cos(time)), sin(time))));

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
            // red.tick();
            // blue.tick();
            // yellow.tick();
            // green.tick();
            // magenta.tick();
        #endif

        scene.updateAllObjects();
        scene.generateShadowMaps();
        renderBuffer.activate();
        skyTexture.bind(4);
        sun->shadowMap.bindTexture(0, 2);
        scene.genLightBuffer();
        scene.draw();
        renderBuffer.deactivate();

        renderBuffer.bindTextures();
        
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        SSAO.render(camera);
        Bloom.render(camera);

        glViewport(0, 0, globals.windowWidth(), globals.windowHeight());
        sun->shadowMap.bindTexture(0, 6);
        // screenBuffer2D.bindTexture(0, 7);
        PostProcessing.activate();
        globals.drawFullscreenQuad();
 
        mainloopEndRoutine();

        Bloom.disable();
        SSAO.disable();
    }
}