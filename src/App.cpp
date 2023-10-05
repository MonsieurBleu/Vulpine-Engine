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


// Ancienement DrawCurve
ModelRef newCurve(GenericSharedBuffer TabPointsOfCurve, long nbPoints)
{
    static MeshMaterial basic3DShader(
        new ShaderProgram(
            "shader/foward rendering/basic.frag", 
            "shader/foward rendering/basic.vert", 
            "", 
            globals.standartShaderUniform3D()
        )
    );

    MeshVao lineGeometry(new VertexAttributeGroup(
        {
                VertexAttribute(TabPointsOfCurve, 0, nbPoints-1, 3, GL_FLOAT, false),
                VertexAttribute(TabPointsOfCurve, 1, nbPoints-1, 3, GL_FLOAT, false),
                VertexAttribute(TabPointsOfCurve, 2, nbPoints-1, 3, GL_FLOAT, false)
        }
    ));

    ModelRef lines(new MeshModel3D(
        basic3DShader,
        lineGeometry
    ));

    return lines;
}

GenericSharedBuffer HermiteCubicCurve(vec3 P0, vec3 P1, vec3 V0, vec3 V1, long nbU)
{
    size_t bufferSize = sizeof(vec3)*nbU;
    GenericSharedBuffer linePositions = GenericSharedBuffer(new char[bufferSize]);
    char* positionReader = linePositions.get();

    for(long i = 0; i < nbU; i++)
    {
        float u = (float)i/(float)(nbU-1);

        float u2 = u*u;
        float u3 = u*u2;
        float F1 = 2*u3 - 3*u2+ 1;
        float F2 = -2*u3 + 3*u2;
        float F3 = u3 - 2*u2 + u;
        float F4 = u3 - u2;

        vec3 newPoint = F1*P0 + F2*P1 + F3*V0 + F4*V1;
        memcpy(positionReader, &newPoint, sizeof(vec3));
        positionReader += sizeof(vec3);
    }

    return linePositions;
}

int fact(int x)
{
    int res = 1;

    for(int i = 2; i <= x; i++)
        res *= i;
    
    return res;
}

GenericSharedBuffer BezierCurveByBernstein(GenericSharedBuffer tabControlPoint, long nbControlPoint, long nbU)
{
    size_t bufferSize = sizeof(vec3)*nbU;
    GenericSharedBuffer linePositions = GenericSharedBuffer(new char[bufferSize]);
    char* positionReader = linePositions.get();

    for(long i = 0; i < nbU; i++)
    {
        float u = (float)i/(float)(nbU-1);
        long n = nbControlPoint-1;
        vec3 newPoint(0.0);
        
        for(long j = 0; j < nbControlPoint; j++)
        {
            float B = 
                ((float)fact(n)/((float)fact(j)*fact(n-j))) 
                * pow(u, j) 
                * pow(1.0-u, n-j);
            newPoint += ((vec3*)tabControlPoint.get())[j]*B;
        }

        memcpy(positionReader, &newPoint, sizeof(vec3));
        positionReader += sizeof(vec3);
    }
    
    return linePositions;
}

vec3 CJRecurrence(GenericSharedBuffer points, long k, long i, float u, long maxRec)
{
    if(k == 0)
        return ((vec3*)points.get())[i];

    if(maxRec == 0)
        return ((vec3*)points.get())[i];

    return (1.f-u)*CJRecurrence(points, k-1, i, u, maxRec-1) + u *CJRecurrence(points, k-1, i+1, u, maxRec-1);
}

vec3 CJRecurrence2(GenericSharedBuffer points, long k, long i, float u, long maxRec, std::vector<vec3> buff)
{
    if(k == 0)
        return ((vec3*)points.get())[i];

    vec3 v = (1.f-u)*CJRecurrence(points, k-1, i, u, maxRec-1);
    vec3 v2 = u *CJRecurrence(points, k-1, i+1, u, maxRec-1);
    
    if(maxRec == 0)
    {
        buff.push_back(v+v2);
    }

    return v + v2;
}


GenericSharedBuffer BezierCurveByCasteljau(GenericSharedBuffer TabControlPoint, long nbControlPoint, long nbU, long maxRec = 0) 
{
    size_t bufferSize = sizeof(vec3)*nbU;
    GenericSharedBuffer linePositions = GenericSharedBuffer(new char[bufferSize]);
    char* positionReader = linePositions.get();

    long it = nbU - maxRec*4;

    for(long i = 0; i < nbU; i++)
    {
        float u = (float)i/(float)(nbU-1);
        
        vec3 newPoint;

        // if(i < maxRec*2)
        //     newPoint = ((vec3*)TabControlPoint.get())[(int)u*nbControlPoint];
        // else
            newPoint = CJRecurrence(TabControlPoint, nbControlPoint-1, 0, u, 0xFFFF);

        memcpy(positionReader, &newPoint, sizeof(vec3));
        positionReader += sizeof(vec3);        
    }
    
    return linePositions;
}

GenericSharedBuffer BezierCurveByCasteljau2(GenericSharedBuffer TabControlPoint, long nbControlPoint, long nbU, long maxRec = 0) 
{
    size_t bufferSize = sizeof(vec3)*nbControlPoint;
    GenericSharedBuffer linePositions = GenericSharedBuffer(new char[bufferSize]);
    char* positionReader = linePositions.get();

    for(long i = 0; i < nbControlPoint; i++)
    {
        vec3 newPoint = CJRecurrence(TabControlPoint, maxRec, i, 0.5, 0xFFFFF);
        memcpy(positionReader, &newPoint, sizeof(vec3));
        positionReader += sizeof(vec3);
    }

    return linePositions;
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

    ShaderProgram PostProcessing = ShaderProgram(
        "shader/post-process/final composing.frag", 
        "shader/post-process/basic.vert", 
        "", 
        globals.standartShaderUniform2D());
    
    PostProcessing.addUniform(ShaderUniform(Bloom.getIsEnableAddr(), 10));

    #ifdef INVERTED_Z
    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
    #endif

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
            .scaleScalar(10.0)
            .setPosition(vec3(0.0, 0.0, 0.0)));
    
    Texture2D skyTexture =Texture2D()
            .loadFromFile("ressources/test/skybox/puresky.hdr")
            .generate();

    skybox->setColorMap(skyTexture);

    skybox->invertFaces = true;
    skybox->depthWrite = false;

    

    SceneDirectionalLight sun = newDirectionLight(
        DirectionLight()
            .setColor(vec3(1.0, 0.85, 0.75))
            .setDirection(normalize(vec3(-1.0, 0.1, 1.0)))
            .setIntensity(1.0)
            );

    scene.add(sun);
    // scene.add(skybox);

    // MeshMaterial basic3DShader(
    //     new ShaderProgram(
    //         "shader/foward rendering/basic.frag", 
    //         "shader/foward rendering/basic.vert", 
    //         "", 
    //         globals.standartShaderUniform3D()
    //     )
    // );

    vec3 points [] =
    {
        vec3(0., 0., 0.),
        vec3(1., 0., 0.),
        vec3(1., 1., 0.),
        vec3(2., 1., 0.)
    };

    GenericSharedBuffer pointsBuff = GenericSharedBuffer(new char[4*sizeof(vec3)]);
    memcpy(pointsBuff.get(), points, 4*sizeof(vec3));

    // GenericSharedBuffer linePositions = GenericSharedBuffer(new char[sizeof(points)]);
    // memcpy(linePositions.get(), (char*)points, sizeof(points));

    // MeshVao lineGeometry(new VertexAttributeGroup(
    //     {
    //             VertexAttribute(linePositions, 0, 4, 3, GL_FLOAT, false),
    //             VertexAttribute(linePositions, 1, 4, 3, GL_FLOAT, false),
    //             VertexAttribute(linePositions, 2, 4, 3, GL_FLOAT, false)
    //     }
    // ));

    // ModelRef lines(new MeshModel3D(
    //     basic3DShader,
    //     lineGeometry
    // ));
    long nbU = 10;
    // ModelRef lines = newCurve(
    //     HermiteCubicCurve(
    //         vec3(0, 0, 0),
    //         vec3(2, 0, 0), 
    //         vec3(1, 1, 0), 
    //         vec3(1, -1, 0),
    //         nbU), 
    //     nbU);

    // ModelRef lines = newCurve(
    //     BezierCurveByBernstein(
    //         pointsBuff, 
    //         4, 
    //         nbU),
    //     nbU
    // );

    // ModelRef lines = newCurve(
    //     BezierCurveByBernstein(
    //         HermiteCubicCurve(
    //             vec3(0, 0, 0),
    //             vec3(2, 0, 0), 
    //             vec3(1, 1, 0), 
    //             vec3(1, -1, 0),
    //             nbU), 
    //         nbU, 
    //         nbU),
    //     nbU
    // );

    ModelRef lines = newCurve(
        BezierCurveByCasteljau(
            pointsBuff, 
            4, 
            nbU),
        nbU
    );
    
    const int tabSize = 1;
    ModelRef linesTab[tabSize];
    for(int i = 0; i < tabSize; i++)
        linesTab[i] = newCurve(
            BezierCurveByCasteljau2(
                pointsBuff, 
                4, 
                nbU, 
                0),
            4
        );

    // scene.add(lines);

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
                skybox->getMaterial()->reset();
                lines->getMaterial()->reset();
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

        renderBuffer.activate();
        // ligthBuffer.activate(0);
        skyTexture.bind(1);
        scene.draw();
        
        // lines->draw(GL_LINE_STRIP);
        for(int i = 0; i < tabSize; i++)
            linesTab[i]->draw(GL_LINE_STRIP);
       
        // glBegin(GL_LINE_STRIP);
        // glColor3f(1.0, 0.0, 0.0);
        // glVertex3f(0.0, 0.0, 0.0);
        // glVertex3f(10.0, 10.0, .0);
        // glVertex3f(-10.0, 10.0, .0);
        // glEnd();


        renderBuffer.deactivate();
        renderBuffer.bindTextures();

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // SSAO.render(camera);
        // Bloom.render(camera);

        glViewport(0, 0, globals.windowWidth(), globals.windowHeight());
        PostProcessing.activate();
        globals.drawFullscreenQuad();
 
        mainloopEndRoutine();
    }
}