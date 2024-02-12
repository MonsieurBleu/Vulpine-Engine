#ifndef APP_HPP
#define APP_HPP

#include <iostream>

#include <Scene.hpp>

#include <Inputs.hpp>

#include <RenderPass.hpp>
#include <Controller.hpp>

#include <Enums.hpp>

class App
{
    protected :

        AppState state = AppState::init;
        GLFWwindow* window;

        Camera camera;

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

        void activateMainSceneBindlessTextures();
        void activateMainSceneClusteredLighting(ivec3 dimention);

        vec3 ambientLight;

        virtual bool userInput(GLFWKeyInfo input);
        bool baseInput(GLFWKeyInfo input);
        void setController(Controller *c);

        void setIcon(const std::string &filename);

    public :
        App(GLFWwindow* window);

        void init();

        virtual void mainloop();
};

#endif