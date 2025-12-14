#ifndef INPUTS_HPP
#define INPUTS_HPP

#include <deque>
#include <functional>
#include <mutex>
#include <string>
#include <memory>
#include <list>

#define GLFW_DLL
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

struct GLFWKeyInfo
{
    GLFWwindow *window;
    int key;
    int scanCode;
    int action;
    int mods;
};

void giveCallbackToApp(GLFWKeyInfo input);

class App;

class InputBuffer : private std::deque<GLFWKeyInfo>
{
    friend App;
    friend void giveCallbackToApp(GLFWKeyInfo input);
    void add(GLFWKeyInfo input);

  public:
    bool pull(GLFWKeyInfo &input);
};

extern std::mutex inputMutex;
extern InputBuffer inputs;

typedef std::function<void()> InputCallback;
typedef std::function<bool()> InputFilter;

enum class InputType
{
    EVENT,
    CONTINUOUS
};

struct GenericInput
{
    std::string inputName;
    int keyCode;
    bool useLuaCallback = false;
    InputCallback callback;
    std::string luaCallbackFilename;
    InputFilter filter;
    InputCallback falseCondCallback;
    bool activated = true;

    GenericInput(std::string inputName, int keyCode, InputCallback callback, InputFilter filter, InputCallback falseCondCallback)
        : inputName{inputName}, keyCode(keyCode), callback{callback}, filter{filter}, falseCondCallback(falseCondCallback)
    {
    }

    GenericInput(std::string inputName, int keyCode, std::string luaCallbackFilename, InputFilter filter, InputCallback falseCondCallback)
        : inputName{inputName}, keyCode(keyCode), useLuaCallback(true), luaCallbackFilename{luaCallbackFilename}, filter{filter}, falseCondCallback(falseCondCallback)
    {
    }

    GenericInput() = default;

    void operator()() const;
};

struct EventInput : public GenericInput
{
    int mods;
    int action;
    bool isScanCode = true;

    EventInput(std::string inputName, int keyCode, InputCallback callback, InputFilter filter, int mods, int action,
               bool isScanCode = true, InputCallback falseCondCallback = {})
        : GenericInput{inputName, keyCode, callback, filter, falseCondCallback}, mods{mods}, action{action}, isScanCode{isScanCode}
    {
    }

    EventInput(std::string inputName, int keyCode, std::string luaCallbackFilename, InputFilter filter, int mods, int action,
            bool isScanCode = true, InputCallback falseCondCallback = {})
    : GenericInput{inputName, keyCode, luaCallbackFilename, filter, falseCondCallback}, mods{mods}, action{action}, isScanCode{isScanCode}
    {
    }

    EventInput() = default;
};

struct ContinuousInput : public GenericInput
{
    ContinuousInput(std::string inputName, int keyCode, InputCallback callback, InputFilter filter, InputCallback falseCondCallback = {})
        : GenericInput{inputName, keyCode, callback, filter, falseCondCallback}
    {
    }

    ContinuousInput(std::string inputName, int keyCode, std::string luaCallbackFilename, InputFilter filter, InputCallback falseCondCallback = {})
        : GenericInput{inputName, keyCode, luaCallbackFilename, filter, falseCondCallback}
    {
    }
};

#define VULPINE_GAMEPAD_BIT                  0b1000000000
#define VULPINE_GAMEPAD_BUTTON_A             VULPINE_GAMEPAD_BIT + GLFW_GAMEPAD_BUTTON_A
#define VULPINE_GAMEPAD_BUTTON_B             VULPINE_GAMEPAD_BIT + GLFW_GAMEPAD_BUTTON_B
#define VULPINE_GAMEPAD_BUTTON_X             VULPINE_GAMEPAD_BIT + GLFW_GAMEPAD_BUTTON_X
#define VULPINE_GAMEPAD_BUTTON_Y             VULPINE_GAMEPAD_BIT + GLFW_GAMEPAD_BUTTON_Y
#define VULPINE_GAMEPAD_BUTTON_LEFT_BUMPER   VULPINE_GAMEPAD_BIT + GLFW_GAMEPAD_BUTTON_LEFT_BUMPER
#define VULPINE_GAMEPAD_BUTTON_RIGHT_BUMPER  VULPINE_GAMEPAD_BIT + GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER
#define VULPINE_GAMEPAD_BUTTON_BACK          VULPINE_GAMEPAD_BIT + GLFW_GAMEPAD_BUTTON_BACK
#define VULPINE_GAMEPAD_BUTTON_START         VULPINE_GAMEPAD_BIT + GLFW_GAMEPAD_BUTTON_START
#define VULPINE_GAMEPAD_BUTTON_GUIDE         VULPINE_GAMEPAD_BIT + GLFW_GAMEPAD_BUTTON_GUIDE
#define VULPINE_GAMEPAD_BUTTON_LEFT_THUMB    VULPINE_GAMEPAD_BIT + GLFW_GAMEPAD_BUTTON_LEFT_THUMB
#define VULPINE_GAMEPAD_BUTTON_RIGHT_THUMB   VULPINE_GAMEPAD_BIT + GLFW_GAMEPAD_BUTTON_RIGHT_THUMB
#define VULPINE_GAMEPAD_BUTTON_DPAD_UP       VULPINE_GAMEPAD_BIT + GLFW_GAMEPAD_BUTTON_DPAD_UP
#define VULPINE_GAMEPAD_BUTTON_DPAD_RIGHT    VULPINE_GAMEPAD_BIT + GLFW_GAMEPAD_BUTTON_DPAD_RIGHT
#define VULPINE_GAMEPAD_BUTTON_DPAD_DOWN     VULPINE_GAMEPAD_BIT + GLFW_GAMEPAD_BUTTON_DPAD_DOWN
#define VULPINE_GAMEPAD_BUTTON_DPAD_LEFT     VULPINE_GAMEPAD_BIT + GLFW_GAMEPAD_BUTTON_DPAD_LEFT

#define VULPINE_GAMEPAD_BUTTON_LAST             VULPINE_GAMEPAD_BUTTON_DPAD_LEFT

#define VULPINE_GAMEPAD_BUTTON_CROSS            VULPINE_GAMEPAD_BUTTON_A
#define VULPINE_GAMEPAD_BUTTON_CIRCLE           VULPINE_GAMEPAD_BUTTON_B
#define VULPINE_GAMEPAD_BUTTON_SQUARE           VULPINE_GAMEPAD_BUTTON_X
#define VULPINE_GAMEPAD_BUTTON_TRIANGLE         VULPINE_GAMEPAD_BUTTON_Y

namespace InputManager
{
    inline std::list<EventInput> eventInputs;
    inline std::list<ContinuousInput> continuousInputs;

    inline GLFWgamepadstate gamepadState;

    inline int currentJoystick = -1; // -1 means no joystick is connected

    void initJoystick();
    void joystickCallback(int jid, int event);

    // generic filter functions
    namespace Filters
    {
        extern InputFilter always;
    }; // namespace Filters

    EventInput &addEventInput(std::string inputName, int keyCode, int mods, int action, InputCallback callback,
                            InputFilter filter = Filters::always, bool isScanCode = true, InputCallback falseCondCallback = {});
    EventInput &addEventInput(std::string inputName, int keyCode, int mods, int action, std::string luaCallbackFilename,
                            InputFilter filter = Filters::always, bool isScanCode = true, InputCallback falseCondCallback = {});
    
    ContinuousInput &addContinuousInput(std::string inputName, int keyCode, InputCallback callback,
                                        InputFilter filter = Filters::always, InputCallback falseCondCallback = {});
    ContinuousInput &addContinuousInput(std::string inputName, int keyCode, std::string luaCallbackFilename,
                                        InputFilter filter = Filters::always, InputCallback falseCondCallback = {});

    float getGamepadAxisValue(int axisCode);
    bool getGamepadButtonValue(int buttonCode);
    bool isGamePadConnected();
    std::vector<GLFWKeyInfo> pollGamepad();

    void processEventInput(const GLFWKeyInfo &event);
    void processContinuousInputs();
    void updateGamepadState();
    void clearInputs();
    void clearEventInputs();
    void clearContinuousInputs();

    glm::vec2 getMousePosition();

    std::string getInputKeyString(std::string inputName);
    std::string getInputKeyString(const EventInput& input);
    std::string getInputKeyString(const ContinuousInput& input);
};

#endif