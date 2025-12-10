#ifndef APP_HPP
#define APP_HPP

#include <iostream>

#include <Graphics/Scene.hpp>

#include <Inputs.hpp>

#include <Graphics/RenderPass.hpp>
#include <Controller.hpp>

#include <Enums.hpp>

class App
{
    protected :

        AppState state = AppState::init;
        GLFWwindow* window;

        Camera camera;

        RenderBuffer* renderBuffer;
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
        void activateMainSceneClusteredLighting(ivec3 dimention, float vFar);


        virtual bool userInput(GLFWKeyInfo input);
        bool baseInput(GLFWKeyInfo input);

        void setIcon(const std::string &filename);

        void loadAllAssetsInfos(const char *filename);

        
        vec3 sunDir = vec3(0.0f);
        vec3 moonPos = vec3(0.0f);
        vec3 planetPos = vec3(0.0f);
        vec3 planetRot = vec3(1.0f);
        vec3 moonRot = vec3(1.0f);
        mat3 tangentSpace = mat3(1.0f);

    public :
    
        static inline vec3 ambientLight;
    
        static void setController(Controller *c);

        void resizeCallback(GLFWwindow* window, int width, int height);

        App(GLFWwindow* window, RenderBuffer *renderBuffer);
        ~App();

        void init();

        virtual void mainloop();
};

#endif