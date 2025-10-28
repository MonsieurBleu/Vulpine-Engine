#ifndef INPUTS_HPP
#define INPUTS_HPP

#include <deque>
#include <functional>
#include <mutex>
#include <string>
#include <vector>
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

    GenericInput(std::string inputName, int keyCode, InputCallback callback, InputFilter filter)
        : inputName{inputName}, keyCode{keyCode}, callback{callback}, filter{filter}
    {
    }

    GenericInput(std::string inputName, int keyCode, std::string luaCallbackFilename, InputFilter filter)
        : inputName{inputName}, keyCode{keyCode}, useLuaCallback(true), luaCallbackFilename{luaCallbackFilename}, filter{filter}
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

    bool activated = true;

    EventInput(std::string inputName, int keyCode, InputCallback callback, InputFilter filter, int mods, int action,
               bool isScanCode = true)
        : GenericInput{inputName, keyCode, callback, filter}, mods{mods}, action{action}, isScanCode{isScanCode}
    {
    }

    EventInput(std::string inputName, int keyCode, std::string luaCallbackFilename, InputFilter filter, int mods, int action,
            bool isScanCode = true)
    : GenericInput{inputName, keyCode, luaCallbackFilename, filter}, mods{mods}, action{action}, isScanCode{isScanCode}
    {
    }

    EventInput() = default;
};

struct ContinuousInput : public GenericInput
{
    ContinuousInput(std::string inputName, int keyCode, InputCallback callback, InputFilter filter)
        : GenericInput{inputName, keyCode, callback, filter}
    {
    }

    ContinuousInput(std::string inputName, int keyCode, std::string luaCallbackFilename, InputFilter filter)
        : GenericInput{inputName, keyCode, luaCallbackFilename, filter}
    {
    }
};

namespace InputManager
{
    inline std::list<EventInput> eventInputs;
    inline std::list<ContinuousInput> continuousInputs;

    // generic filter functions
    namespace Filters
    {
        extern InputFilter always;
    }; // namespace Filters

    EventInput &addEventInput(std::string inputName, int keyCode, int mods, int action, InputCallback callback,
                            InputFilter filter = Filters::always, bool isScanCode = true);
    EventInput &addEventInput(std::string inputName, int keyCode, int mods, int action, std::string luaCallbackFilename,
                            InputFilter filter = Filters::always, bool isScanCode = true);
    
    ContinuousInput &addContinuousInput(std::string inputName, int keyCode, InputCallback callback,
                                        InputFilter filter = Filters::always);
    ContinuousInput &addContinuousInput(std::string inputName, int keyCode, std::string luaCallbackFilename,
                                        InputFilter filter = Filters::always);

    void processEventInput(const GLFWKeyInfo &event);
    void processContinuousInputs();
    void clearInputs();
    void clearEventInputs();
    void clearContinuousInputs();

    glm::vec2 getMousePosition();

    std::string getInputKeyString(std::string inputName);

};

#endif