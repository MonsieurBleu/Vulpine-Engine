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
    InputCallback falseCondCallback;
    bool activated = true;

    GenericInput(std::string inputName, int keyCode, InputCallback callback, InputFilter filter, InputCallback falseCondCallback)
        : inputName{inputName}, keyCode{keyCode}, callback{callback}, filter{filter}, falseCondCallback(falseCondCallback)
    {
    }

    GenericInput(std::string inputName, int keyCode, std::string luaCallbackFilename, InputFilter filter, InputCallback falseCondCallback)
        : inputName{inputName}, keyCode{keyCode}, useLuaCallback(true), luaCallbackFilename{luaCallbackFilename}, filter{filter}, falseCondCallback(falseCondCallback)
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
                            InputFilter filter = Filters::always, bool isScanCode = true, InputCallback falseCondCallback = {});
    EventInput &addEventInput(std::string inputName, int keyCode, int mods, int action, std::string luaCallbackFilename,
                            InputFilter filter = Filters::always, bool isScanCode = true, InputCallback falseCondCallback = {});
    
    ContinuousInput &addContinuousInput(std::string inputName, int keyCode, InputCallback callback,
                                        InputFilter filter = Filters::always, InputCallback falseCondCallback = {});
    ContinuousInput &addContinuousInput(std::string inputName, int keyCode, std::string luaCallbackFilename,
                                        InputFilter filter = Filters::always, InputCallback falseCondCallback = {});

    void processEventInput(const GLFWKeyInfo &event);
    void processContinuousInputs();
    void clearInputs();
    void clearEventInputs();
    void clearContinuousInputs();

    glm::vec2 getMousePosition();

    std::string getInputKeyString(std::string inputName);
    std::string getInputKeyString(const EventInput& input);
    std::string getInputKeyString(const ContinuousInput& input);
};

#endif