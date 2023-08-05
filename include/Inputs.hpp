#ifndef INPUTS_HPP
#define INPUTS_HPP

#include <mutex>
#include <list>

#define GLFW_DLL
#include <GLFW/glfw3.h>

struct GLFWKeyInfo
{
    GLFWwindow* window;
    int key;
    int scancode;
    int action;
    int mods;
};

void giveCallbackToApp(GLFWKeyInfo input);

class App;

class InputBuffer : private std::list<GLFWKeyInfo>
{
    friend void giveCallbackToApp(GLFWKeyInfo input);
    void add(GLFWKeyInfo input);

    public : 
        bool pull(GLFWKeyInfo& input);
};

extern std::mutex inputMutex;
extern InputBuffer inputs;

#endif