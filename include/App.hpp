#ifndef APP_HPP
#define APP_HPP

#include <iostream>

#include <Scene.hpp>

#include <Inputs.hpp>

#include <RenderPass.hpp>

enum AppState
{
    quit,
    run // default state for tests
};

class App
{
    protected :

        AppState state = run;
        GLFWwindow* window;
        uint64_t timestart;

        Camera camera;

        void mainInput(double deltatime);

        RenderBuffer renderBuffer;
        FrameBuffer screenBuffer2D;
        SSAOPass SSAO;
        BloomPass Bloom;
        ShaderProgram finalProcessingStage;

        Scene scene;
        Scene scene2D;

        void mainloopStartRoutine();
        void mainloopPreRenderRoutine();
        void mainloopEndRoutine();

        vec3 ambientLight;

        virtual bool userInput(GLFWKeyInfo input);

    public :
        App(GLFWwindow* window);

        void init();

        virtual void mainloop();
};

#endif