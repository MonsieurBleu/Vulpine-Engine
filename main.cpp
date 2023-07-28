// #include <iostream>
// #include <GL/glew.h>
// #define GLFW_DLL
// #include <GLFW/glfw3.h>

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

int main()
{
    #ifdef DO_TEST
    featureTest();
    #endif

    atexit(checkHeap);

    system("cls");
    std::cout << "#### RAY TRACED VOXELS ####\n";

    // start GL context and O/S window using the GLFW helper library
    if (!glfwInit())
    {
        std::cerr << "ERROR: could not start GLFW3\n";
        return 1;
    } 

    // uncomment these lines if on Apple OS X
    /*glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);*/

    GLFWwindow* window = glfwCreateWindow(1920, 1080, "RT Voxels", NULL, NULL);
    // GLFWwindow* window = glfwCreateWindow(3440, 1440, "RT Voxels", NULL, NULL);
    if (!window) {
    std::cerr << "ERROR: could not open window with GLFW3\n";
    glfwTerminate();
    return 1;
    }
    glfwMakeContextCurrent(window);
                                    
    // start GLEW extension handler
    glewExperimental = GL_TRUE;
    glewInit();

    glEnable( GL_DEBUG_OUTPUT );
    glDebugMessageCallback(MessageCallback, 0);

    // get version info
    const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
    const GLubyte* version = glGetString(GL_VERSION); // version as a string
    std::cout << TERMINAL_INFO 
    << "Renderer: " << renderer << "\n" 
    << "OpenGL version supported " << version << "\n"
    << TERMINAL_RESET << "\n";

    // tell GL to only draw onto a pixel if the shape is closer to the viewer
    // glEnable(GL_DEPTH_TEST); // enable depth-testing
    // glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"

    /* OTHER STUFF GOES HERE NEXT */

    App RTVoxl(window);
    RTVoxl.mainloop();

    // close GL context and any other GLFW resources
    glfwTerminate();

    return EXIT_SUCCESS;
}