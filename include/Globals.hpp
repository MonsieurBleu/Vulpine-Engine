#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <App.hpp>
#include <Timer.hpp>

#include <Mesh.hpp>

#include <codecvt>
#include <iomanip>
#include <Controller.hpp>

typedef std::basic_ostringstream<char32_t> UFT32Stream;

class Globals
{   
    friend App;

    private :
    
        AppState _state;

        const GLFWvidmode* _videoMode;

        glm::vec2 _mousePosition;

        ivec2 _windowSize;
        ivec2 _renderSize;
        float _renderScale = 1.0;

        std::vector<ShaderUniform> _standartShaderUniform2D;
        std::vector<ShaderUniform> _standartShaderUniform3D;

        Mesh _fullscreenQuad;

        bool _mouseLeftClick = false;
        bool _mouseLeftClickDown = false;

        std::u32string textInputString;
        void* currentTextInputUser = nullptr;

        uint _drawCalls3D = 0;

        int _windowHasFocus = false;

        vec2 _scrollOffset = vec2(0);

        Controller *_currentController = nullptr;

    public :

        MeshMaterial basicMaterial;

        /**
         * Globals are not cloneable.
         */
        // Globals(Globals &other) = delete;
        /**
         * Globals are note assignable.
         */
        // void operator=(const Globals&) = delete;

        Camera *currentCamera = nullptr;
        const Controller * getController() const;

        const AppState state() const;

        const GLFWvidmode* videoMode() const;

        glm::ivec2 screenResolution() const;
        const int* screenResolutionAddr() const;
        const glm::vec2 mousePosition() const;

        BenchTimer appTime = BenchTimer("App Time");
        BenchTimer simulationTime = BenchTimer("Simulation Time");
        BenchTimer cpuTime = BenchTimer("CPU Time");
        BenchTimer gpuTime = BenchTimer("GPU Time");
        LimitTimer fpsLimiter = LimitTimer();

        bool enablePhysics = true;

        std::vector<ShaderUniform> standartShaderUniform2D() const;
        std::vector<ShaderUniform> standartShaderUniform3D() const;

        int windowWidth() const;
        int windowHeight() const;
        ivec2 windowSize() const;
        const ivec2* windowSizeAddr() const;

        float renderScale() const;
        ivec2 renderSize() const;
        const ivec2* renderSizeAddr() const;

        void drawFullscreenQuad();

        bool mouseLeftClick();
        bool mouseLeftClickDown();

        vec2 mouseScrollOffset();
        void clearMouseScroll();

        vec3 sceneChromaticAbbColor1 = vec3(0);
        vec3 sceneChromaticAbbColor2 = vec3(0);
        vec2 sceneChromaticAbbAngleAmplitude = vec2(0);
        vec4 sceneVignette = vec4(0);
        vec3 sceneHsvShift = vec4(0);

        /**
         * @brief Set the given pointer as text input user
         * @retval `true` if the text input is arleady used
         * @retval `false` if the text input is not used
         */
        bool useTextInputs(void* user);

        /**
         * @brief End the given user's right to use the text input buffer
         * @retval `true` if the given pointer was the current user
         * @retval `false` if the given pointer was not the current user
         */
        bool endTextInputs(void* user);

        /**
         * @retval `true` if the given pointer is the current user
         * @retval `false` if the given pointer is not the current user
         */
        bool canUseTextInputs(void* user);

        /**
         * @retval `true` if there is an user for text inputs
         * @retval `false` if there isn't any user for text inputs
         */
        bool isTextInputsActive();

        /**
         * @brief fill the given string with the current text input.
         * if the given pointer is not the current user, nothing will
         * be done.
         * @retval `true` if the given pointer is the current user
         * @retval `false` if the given pointer is not the current user
         */
        bool getTextInputs(void* user, std::u32string& buff);

        int windowHasFocus();
};

extern Globals globals;

#endif