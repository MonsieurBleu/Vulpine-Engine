#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <Timer.hpp>

#include <Graphics/Mesh.hpp>

#include <codecvt>
#include <iomanip>
#include <Controller.hpp>
#include <Graphics/Scene.hpp>
#include <Enums.hpp>
#include <Graphics/Fonts.hpp>

typedef std::basic_ostringstream<char32_t> UFT32Stream;

class Game;
class NoiseApp;

class Globals
{
    friend App;
    friend Game;
    friend NoiseApp;

private:
    AppState _state;

    const GLFWvidmode *_videoMode;

    GLFWwindow *_window;

    glm::vec2 _mousePosition;

    ivec2 _windowSize;
    ivec2 _renderSize;
    float _renderScale = 1.0;
    float _UI_res_scale = 2.0;

    std::vector<ShaderUniform> _standartShaderUniform2D;
    std::vector<ShaderUniform> _standartShaderUniform3D;

    Mesh _fullscreenQuad;

    bool _mouseLeftClick = false;
    bool _mouseLeftClickDown = false;

    bool _mouseRightClick = false;
    bool _mouseRightClickDown = false;

    bool _mouseMiddleClick = false;
    bool _mouseMiddleClickDown = false;

    bool _mouse4Click = false;
    bool _mouse4ClickDown = false;

    bool _mouse5Click = false;
    bool _mouse5ClickDown = false;

    std::u32string textInputString;
    void *currentTextInputUser = nullptr;

    std::vector<std::string> _dropInput;

    uint _drawCalls3D = 0;

    int _windowHasFocus = false;

    vec2 _scrollOffset = vec2(0);


    Scene *_gameScene;
    Scene *_gameScene2D;

public:
    // MeshMaterial basicMaterial;

    Controller *_currentController = nullptr;
    Scene *getScene() const; 
    Scene *getScene2D() const; 

    /**
     * Globals are not cloneable.
     */
    // Globals(Globals &other) = delete;
    /**
     * Globals are note assignable.
     */
    // void operator=(const Globals&) = delete;

    FontRef baseFont;
    Camera *currentCamera = nullptr;
    Controller *getController();

    const AppState state() const;

    const GLFWvidmode *videoMode() const;

    glm::ivec2 screenResolution() const;
    const int *screenResolutionAddr() const;
    const glm::vec2 mousePosition() const;

    BenchTimer appTime = BenchTimer("App");
    BenchTimer mainThreadTime = BenchTimer("Main Thread");
    BenchTimer simulationTime = BenchTimer("Simulation");
    BenchTimer cpuTime = BenchTimer("CPU");
    BenchTimer gpuTime = BenchTimer("GPU");
    LimitTimer fpsLimiter = LimitTimer();

    bool enablePhysics = true;

    std::vector<ShaderUniform> standartShaderUniform2D() const;
    std::vector<ShaderUniform> standartShaderUniform3D() const;

    int windowWidth() const;
    int windowHeight() const;
    ivec2 windowSize() const;
    const ivec2 *windowSizeAddr() const;

    float renderScale() const;
    ivec2 renderSize() const;
    const ivec2 *renderSizeAddr() const;

    void drawFullscreenQuad();

    bool mouseLeftClick();
    bool mouseLeftClickDown();

    bool mouseRightClick();
    bool mouseRightClickDown();

    bool mouseMiddleClick();
    bool mouseMiddleClickDown();

    bool mouse4Click();
    bool mouse4ClickDown();

    bool mouse5Click();
    bool mouse5ClickDown();

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
    bool useTextInputs(void *user, const std::u32string & baseInput = U"");

    /**
     * @brief End the given user's right to use the text input buffer
     * @retval `true` if the given pointer was the current user
     * @retval `false` if the given pointer was not the current user
     */
    bool endTextInputs(void *user);

    /**
     * @retval `true` if the given pointer is the current user
     * @retval `false` if the given pointer is not the current user
     */
    bool canUseTextInputs(void *user);

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
    bool getTextInputs(void *user, std::u32string &buff);

    const std::vector<std::string> & getDropInput();
    void clearDropInput();

    int windowHasFocus();

    GLFWwindow *getWindow();
};

extern Globals globals;

#endif