#ifndef APP_HPP
#define APP_HPP

#include <iostream>

#include <Camera.hpp>
#include <Shader.hpp>

#include <Inputs.hpp>

enum AppState
{
    quit,
    run // default state for tests
};

class App
{
    private :

        AppState state = run;
        GLFWwindow* window;
        uint64_t timestart;

        // Map World;

        Camera camera;

        void mainInput(double deltatime);

    public :
        App(GLFWwindow* window);

        void mainloopStartRoutine();
        void mainloop();
        void mainloopPreRenderRoutine();
        void mainloopEndRoutine();

};


#endif