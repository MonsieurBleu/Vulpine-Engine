#include <filesystem>
#include <stb/stb_image.h>

#include <App.hpp>
#include <AssetManager.hpp>
#include <Audio.hpp>
#include <CompilingOptions.hpp>
#include <Controller.hpp>
#include <Globals.hpp>
#include <Graphics/Animation.hpp>
#include <Graphics/FrameBuffer.hpp>
#include <Graphics/Shadinclude.hpp>
#include <Graphics/Skeleton.hpp>
#include <Uniforms.hpp>

std::mutex inputMutex;
std::mutex physicsMutex;
InputBuffer inputs;

Globals globals;

std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> UFTconvert;

void App::loadAllAssetsInfos(const char *filename)
{
    for (auto f : std::filesystem::recursive_directory_iterator(filename))
    {
        if (f.is_directory())
            continue;

        char ext[1024];
        char p[4096];

        strcpy(ext, (char *)f.path().extension().string().c_str());
        strcpy(p, (char *)f.path().string().c_str());

        if (!strcmp(ext, ".vulpineGroup"))
            Loader<ObjectGroup>::addInfos(p);
        else if (!strcmp(ext, ".vulpineGroupRef"))
            Loader<ObjectGroupRef>::addInfos(p);
        else if (!strcmp(ext, ".vulpineModel"))
            Loader<MeshModel3D>::addInfos(p);
        else if (!strcmp(ext, ".vulpineMaterial"))
            Loader<MeshMaterial>::addInfos(p);
        else if (!strcmp(ext, ".vulpineMeshModel"))
            Loader<MeshModel3D>::addInfos(p);
        else if (!strcmp(ext, ".vulpineAnimation"))
            Loader<AnimationRef>::addInfosTextless(p);
        else if (!strcmp(ext, ".vulpineMesh"))
            Loader<MeshVao>::addInfosTextless(p);
        else if (!strcmp(ext, ".obj"))
            Loader<MeshVao>::addInfosTextless(p);
        else if (!strcmp(ext, ".vulpineSkeleton"))
            Loader<SkeletonRef>::addInfosTextless(p);
        else if (!strcmp(ext, ".ktx") || !strcmp(ext, ".ktx2") || !strcmp(ext, ".png") || !strcmp(ext, ".jpg"))
            Loader<Texture2D>::addInfosTextless(p, "source");
    }
};

void App::resizeCallback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
    camera.width = width;
    camera.height = height;

    // renderBuffer.getTexture(0).setResolution(ivec2(width, height)).generate();
    // renderBuffer.getTexture(1).setResolution(ivec2(width, height)).generate();
    // renderBuffer.getTexture(2).setResolution(ivec2(width, height)).generate();
    // renderBuffer.getTexture(3).setResolution(ivec2(width, height)).generate();
    // renderBuffer.getTexture(4).setResolution(ivec2(width, height)).generate();

    ivec2 newres = vec2(width, height) * globals._renderScale;

    renderBuffer.resizeAll(newres);
    screenBuffer2D.resizeAll(vec2(width, height) * globals._UI_res_scale);

    Bloom.getFBO().resizeAll(newres);
    Bloom.getFBO2().resizeAll(newres);

    SSAO.getFBO().resizeAll(newres);

    globals._renderSize = newres;
    globals._windowSize = ivec2(width, height);
}

void App::init()
{
    if (!alCall(alDistanceModel, AL_INVERSE_DISTANCE_CLAMPED))
    {
        std::cerr << "ERROR: Could not set Distance Model to AL_INVERSE_DISTANCE_CLAMPED" << std::endl;
    }

    glfwSetKeyCallback(window, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
        giveCallbackToApp(GLFWKeyInfo{window, key, scancode, action, mods});
    });

    glfwSetMouseButtonCallback(window, [](GLFWwindow *window, int button, int action, int mods) {
        giveCallbackToApp(GLFWKeyInfo{window, button, button, action, mods});
    });

    globals._gameScene = &scene;
    globals._gameScene2D = &scene2D;

    /*
        TODO :
            Test if the videoMode automaticlly update
    */
    globals._videoMode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    glfwGetWindowSize(window, &globals._windowSize.x, &globals._windowSize.y);
    globals._renderSize =
        ivec2(globals._windowSize.x * globals._renderScale, globals._windowSize.y * globals._renderScale);

    globals._standartShaderUniform2D = {
        ShaderUniform(globals.windowSizeAddr(), 0),
        ShaderUniform(globals.appTime.getElapsedTimeAddr(), 1),
    };

    globals._standartShaderUniform3D = {
        ShaderUniform(globals.windowSizeAddr(), 0),
        ShaderUniform(globals.appTime.getElapsedTimeAddr(), 1),
        ShaderUniform(camera.getProjectionViewMatrixAddr(), 2),
        ShaderUniform(camera.getViewMatrixAddr(), 3),
        ShaderUniform(camera.getProjectionMatrixAddr(), 4),
        ShaderUniform(camera.getPositionAddr(), 5),
        // ShaderUniform(camera.getDirectionAddr(),              6),
        ShaderUniform(&ambientLight, 15),
    };


    globals._standartShaderUniform3D.push_back(ShaderUniform(&sunDir, 17));
    globals._standartShaderUniform3D.push_back(ShaderUniform(&moonPos, 18));
    globals._standartShaderUniform3D.push_back(ShaderUniform(&planetPos, 19));
    globals._standartShaderUniform3D.push_back(ShaderUniform(&tangentSpace, 9));
    
    globals._standartShaderUniform2D.push_back(ShaderUniform(&sunDir, 17));
    globals._standartShaderUniform2D.push_back(ShaderUniform(&moonPos, 18));
    globals._standartShaderUniform2D.push_back(ShaderUniform(&planetPos, 19));
    globals._standartShaderUniform2D.push_back(ShaderUniform(&tangentSpace, 9));


    globals._fullscreenQuad.setVao(MeshVao(new VertexAttributeGroup(
        {VertexAttribute(GenericSharedBuffer((char *)new float[12]{-1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,
                                                                   1.0f, 1.0f, -1.0f, -1.0f}),
                         0, 6, 2, GL_FLOAT, false)})));

    globals._fullscreenQuad.getVao()->generate();
    renderBuffer.generate();
    SSAO.setup();
    Bloom.setup();

    screenBuffer2D
        .addTexture(Texture2D()
                        .setResolution(ivec2(vec2(globals._renderSize) * globals._UI_res_scale))
                        .setInternalFormat(GL_SRGB8_ALPHA8)
                        .setFormat(GL_RGBA)
                        .setPixelType(GL_UNSIGNED_BYTE)
                        .setFilter(GL_LINEAR)
                        .setWrapMode(GL_CLAMP_TO_EDGE)
                        .setAttachement(GL_COLOR_ATTACHMENT0))
        .addTexture(Texture2D()
                        .setResolution(ivec2(vec2(globals._renderSize) * globals._UI_res_scale))
                        .setInternalFormat(GL_DEPTH_COMPONENT)
                        .setFormat(GL_DEPTH_COMPONENT)
                        .setPixelType(GL_UNSIGNED_BYTE)
                        .setFilter(GL_LINEAR)
                        .setWrapMode(GL_CLAMP_TO_EDGE)
                        .setAttachement(GL_DEPTH_ATTACHMENT))
        .generate();

    globals.currentCamera = &camera;

    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    glfwSetCharCallback(
        window, [](GLFWwindow *window, unsigned int codepoint) { globals.textInputString.push_back(codepoint); });

    glfwSetScrollCallback(window, [](GLFWwindow *window, double xoffset, double yoffset) {
        globals._scrollOffset = vec2(xoffset, yoffset);
        GLFWKeyInfo i;
        i.key = 0;
        inputs.add(i);
    });

    static App *mainApp = this;

    glfwSetFramebufferSizeCallback(
        window, [](GLFWwindow *window, int width, int height) { mainApp->resizeCallback(window, width, height); });

    /// CENTER WINDOW
    glfwSetWindowPos(window, (globals.screenResolution().x - globals.windowWidth()) / 2,
                     (globals.screenResolution().y - globals.windowHeight()) / 2);

#ifdef INVERTED_Z
    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
#endif

    glLineWidth(3.0);

    // globals.basicMaterial = MeshMaterial(
    //     new ShaderProgram(
    //         "shader/foward/basic.frag",
    //         "shader/foward/basic.vert",
    //         "",
    //         globals.standartShaderUniform3D()));

    // Loader<MeshMaterial>::addInfos("shader/vulpineMaterials/basicHelper.vulpineMaterial");
    // Loader<MeshMaterial>::addInfos("shader/vulpineMaterials/basicFont3D.vulpineMaterial");
    // Loader<MeshMaterial>::addInfos("shader/vulpineMaterials/basicPBR.vulpineMaterial");
    // Loader<MeshMaterial>::addInfos("shader/vulpineMaterials/animatedPBR.vulpineMaterial");
    // Loader<MeshMaterial>::addInfos("shader/vulpineMaterials/mdFont.vulpineMaterial");
    loadAllAssetsInfos("shader/vulpineMaterials/");
}

void App::activateMainSceneBindlessTextures()
{
    scene.useBindlessTextures = true;
    // scene2D.useBindlessTextures = true;
    Shadinclude::shaderDefines += "#define ARB_BINDLESS_TEXTURE\n";
}

void App::activateMainSceneClusteredLighting(ivec3 dimention, float vFar)
{
    scene.activateClusteredLighting(dimention, vFar);
    globals._standartShaderUniform3D.push_back(ShaderUniform(&scene.getClusteredLight().ivFar, 13));
    globals._standartShaderUniform3D.push_back(ShaderUniform(&scene.getClusteredLight().dim(), 14));
    Shadinclude::shaderDefines += "#define USE_CLUSTERED_RENDERING\n";
}

void App::setIcon(const std::string &filename)
{
    GLFWimage image[1];
    image[0].pixels = stbi_load(filename.c_str(), &image[0].width, &image[0].height, 0, 4);
    glfwSetWindowIcon(window, 1, image);
    stbi_image_free(image[0].pixels);
}

bool App::userInput(GLFWKeyInfo input)
{
    return false;
};

bool App::baseInput(GLFWKeyInfo input)
{
    bool used = false;

    if (globals._currentController)
        used = globals._currentController->inputs(input);

    if (used)
        return true;

    if (input.action == GLFW_RELEASE)
        switch (input.key)
        {
        case GLFW_MOUSE_BUTTON_LEFT:
            globals._mouseLeftClickDown = false;
            // used = true;
            break;

        case GLFW_MOUSE_BUTTON_RIGHT:
            globals._mouseRightClickDown = false;
            // used = true;
            break;

        case GLFW_MOUSE_BUTTON_MIDDLE :
            globals._mouseMiddleClickDown = false;
        default:
            break;
        }

    if (input.action == GLFW_PRESS)
        switch (input.key)
        {
        case GLFW_MOUSE_BUTTON_LEFT:
            globals._mouseLeftClick = true;
            globals._mouseLeftClickDown = true;
            // used = true;
            break;

        case GLFW_MOUSE_BUTTON_RIGHT:
            globals._mouseRightClick = true;
            globals._mouseRightClickDown = true;
            // used = true;
            break;

        case GLFW_MOUSE_BUTTON_MIDDLE:
            globals._mouseMiddleClick = true;
            globals._mouseMiddleClickDown = true;
            // used = true;
            break;

        case GLFW_KEY_V:
            if (input.mods & GLFW_MOD_CONTROL)
            {
                if (globals.isTextInputsActive())
                {
                    globals.textInputString += UFTconvert.from_bytes(glfwGetClipboardString(window));
                    used = true;
                }
            }
            break;

        case GLFW_KEY_ENTER:
            if (globals.isTextInputsActive())
            {
                globals.textInputString.push_back(U'\n');
                used = true;
            }
            break;

        case GLFW_KEY_DELETE:
        case GLFW_KEY_BACKSPACE:
            if (globals.isTextInputsActive() && !globals.textInputString.empty())
            {
                globals.textInputString.pop_back();
                used = true;
            }
            break;

        default:
            break;
        }

    return used;
}

void App::setController(Controller *c)
{
    if (globals._currentController)
        globals._currentController->clean();

    globals._currentController = c;

    if(c)
    {
        c->init();
        glfwSetCursorPosCallback(globals.getWindow(), [](GLFWwindow *window, double dx, double dy) {
            if (globals._currentController)
                globals._currentController->mouseEvent(vec2(dx, dy), window);
        });
    }
}

App::App(GLFWwindow *window)
    : window(window), renderBuffer(globals.renderSizeAddr()), SSAO(renderBuffer), Bloom(renderBuffer)
{
    globals._window = window;
    // if(!alCall(alDistanceModel, AL_INVERSE_DISTANCE_CLAMPED))
    // {
    //     std::cerr << "ERROR: Could not set Distance Model to AL_INVERSE_DISTANCE_CLAMPED" << std::endl;
    // }

    // glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
    // {
    //     giveCallbackToApp(GLFWKeyInfo{window, key, scancode, action, mods});
    // });

    // glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods){
    //     giveCallbackToApp(GLFWKeyInfo{window, button, button, action, mods});
    // });

    // /*
    //     TODO :
    //         Test if the videoMode automaticlly update
    // */
    // globals._videoMode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    // glfwGetWindowSize(window, &globals._windowSize.x, &globals._windowSize.y);
    // globals._renderSize = ivec2(globals._windowSize.x*globals._renderScale,
    // globals._windowSize.y*globals._renderScale);

    // globals._standartShaderUniform2D =
    // {
    //     ShaderUniform(globals.windowSizeAddr(), 0),
    //     ShaderUniform(globals.appTime.getElapsedTimeAddr(),   1),
    // };

    // globals._standartShaderUniform3D =
    // {
    //     ShaderUniform(globals.windowSizeAddr(),               0),
    //     ShaderUniform(globals.appTime.getElapsedTimeAddr(),   1),
    //     ShaderUniform(camera.getProjectionViewMatrixAddr(),   2),
    //     ShaderUniform(camera.getViewMatrixAddr(),             3),
    //     ShaderUniform(camera.getProjectionMatrixAddr(),       4),
    //     ShaderUniform(camera.getPositionAddr(),               5),
    //     ShaderUniform(camera.getDirectionAddr(),              6),
    //     ShaderUniform(&ambientLight,                         15),
    // };

    // globals._fullscreenQuad.setVao(
    //     MeshVao(new VertexAttributeGroup(
    //         {
    //             VertexAttribute(
    //                 GenericSharedBuffer(
    //                     (char *)new float[12]{-1.0f,  1.0f, 1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f, 1.0f,
    //                     -1.0f, -1.0f}
    //                     ),
    //                 0,
    //                 6,
    //                 2,
    //                 GL_FLOAT,
    //                 false
    //             )
    //         }
    //     )));

    // globals._fullscreenQuad.getVao()->generate();
    // renderBuffer.generate();
    // SSAO.setup();
    // Bloom.setup();

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
    //     .addTexture(
    //         Texture2D()
    //             .setResolution(globals.windowSize())
    //             .setInternalFormat(GL_DEPTH_COMPONENT)
    //             .setFormat(GL_DEPTH_COMPONENT)
    //             .setPixelType(GL_UNSIGNED_BYTE)
    //             .setFilter(GL_LINEAR)
    //             .setWrapMode(GL_CLAMP_TO_EDGE)
    //             .setAttachement(GL_DEPTH_ATTACHMENT))
    //     .generate();

    // globals.currentCamera = &camera;

    // glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    // glfwSetCharCallback(window, [](GLFWwindow* window, unsigned int codepoint)
    // {
    //     globals.textInputString.push_back(codepoint);
    // });

    // glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset)
    // {
    //     globals._scrollOffset = vec2(xoffset, yoffset);
    //     GLFWKeyInfo i;
    //     i.key = 0;
    //     inputs.add(i);
    // });

    // /// CENTER WINDOW
    // glfwSetWindowPos(
    //     window,
    //     (globals.screenResolution().x - globals.windowWidth())/2,
    //     (globals.screenResolution().y - globals.windowHeight())/2);

    // #ifdef INVERTED_Z
    // glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
    // #endif

    // glLineWidth(3.0);

    // globals.basicMaterial = MeshMaterial(
    //     new ShaderProgram(
    //         "shader/foward/basic.frag",
    //         "shader/foward/basic.vert",
    //         "",
    //         globals.standartShaderUniform3D()));
}

void App::mainloopStartRoutine()
{
    globals.fpsLimiter.start();
    globals.mainThreadTime.start();
    globals.appTime.start();
    globals.simulationTime.start();
    globals.cpuTime.start();

    globals._mouseLeftClick = false;
    globals._mouseRightClick = false;

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
    vec3 camDir[2] = {globals.currentCamera->getDirection(), cross(camFront, cross(camFront, vec3(0, 1, 0)))};

    // set current listener position
    alListener3f(AL_POSITION, -camPos.x, camPos.y, -camPos.z);

    // set current listener orientation
    alListenerfv(AL_ORIENTATION, (float *)camDir);

    if (globals._currentController)
        globals._currentController->update();
}

void App::mainloopEndRoutine()
{
    globals.cpuTime.end();

    globals.gpuTime.start();
    glfwSwapBuffers(window);
    // glFlush();
    globals.gpuTime.end();
    // glFinish();

    globals.mainThreadTime.end();
    globals.fpsLimiter.waitForEnd();
    globals.appTime.end();
    globals.simulationTime.end();

    scene.endTimers();
    scene2D.endTimers();
}

/*
    The default mainloop is empty
*/
void App::mainloop()
{
    /*
        ... Last main loop inits

        while(state != quit)
        {
            mainloopStartRoutine();

            for (GLFWKeyInfo input; inputs.pull(input); userInput(input));
            ... and other logic/input based updates

            mainloopPreRenderRoutine();

            .. render cals

            mainloopEndRoutine();
        }

        ... Main loop end cleanup (example : joining physics thread)
    */
}

App::~App()
{
    globals._gameScene = nullptr;
}