// #include <iostream>
// #include <GL/glew.h>
// #define GLFW_DLL
// #include <GLFW/glfw3.h>

#include <Audio.hpp>
#include <App.hpp>
#include <Utils.hpp>
#include <GLutils.hpp>

// #define DO_TEST
#ifdef DO_TEST
#include <Uniforms.hpp>
int featureTest()
{
    float test = 5.0;

    vec2 test2d(3.1415f, 3.1415f);

    ShaderUniformGroup testSUG(
        {
            ShaderUniform(0, 0),
            ShaderUniform(&test, 0),
            ShaderUniform(3.1415f, 3),
            ShaderUniform(ivec2(3, 9), 1),
            ShaderUniform(&test2d, 2),
        });

    std::cout << testSUG;

    exit(EXIT_SUCCESS);
};
#endif

ALCdevice* openALDevice = nullptr;
ALCcontext* openALContext = nullptr;

int main()
{
    int status = EXIT_FAILURE;
    GLFWwindow* window;
    ALCboolean contextMadeCurrent = false;

    #ifdef DO_TEST
    featureTest();
    #endif

    atexit(checkHeap);

    system("cls");

    openALDevice = alcOpenDevice(nullptr);
    if(!openALDevice)
    {
        std::cerr << "ERROR: could not start OpenAL\n";
        goto quit;
    }

    if(!alcCall(alcCreateContext, openALContext, openALDevice, openALDevice, nullptr) || !openALContext)
    {
        std::cerr << "ERROR: could not create audio context\n";
        goto quitALC;
    }

    if(!alcCall(alcMakeContextCurrent, contextMadeCurrent, openALDevice, openALContext) || contextMadeCurrent != ALC_TRUE)
    {
        std::cerr << "ERROR: Could not make audio context current\n" << std::endl;
        goto quitALC;
    }

    // start GL context and O/S window using the GLFW helper library
    if (!glfwInit())
    {
        std::cerr << "ERROR: could not start GLFW3\n";
        goto quitALC;
    } 

    // uncomment these lines if on Apple OS X
    /*glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);*/

    window = glfwCreateWindow(1920, 1080, "Game Engine", NULL, NULL);
    if (!window) {
        std::cerr << "ERROR: could not open window with GLFW3\n";
        goto quitGLFW;
    }
    glfwMakeContextCurrent(window);
                                    
    // start GLEW extension handler
    glewExperimental = GL_TRUE;
    glewInit();

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(MessageCallback, 0);

    if(true)
    {
        const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
        const GLubyte* version = glGetString(GL_VERSION); // version as a string
        std::cout << TERMINAL_INFO 
        << "Renderer: " << renderer << "\n" 
        << "OpenGL version supported " << version << "\n"
        << TERMINAL_RESET << "\n";

        App app(window);
        app.mainloop();
    }

    status = EXIT_SUCCESS;

quitGLFW :
    glfwTerminate();
quitALC :
    if(openALContext)
    {
        alcCall(alcMakeContextCurrent, contextMadeCurrent, openALDevice, nullptr);
        alcCall(alcDestroyContext, openALDevice, openALContext);
    }
    alcCloseDevice(openALDevice);
quit :

    return status;
}