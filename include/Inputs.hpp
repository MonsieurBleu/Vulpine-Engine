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
    InputCallback callback;
    InputFilter filter;
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

    EventInput() = default;
};

struct ContinuousInput : public GenericInput
{
    ContinuousInput(std::string inputName, int keyCode, InputCallback callback, InputFilter filter)
        : GenericInput{inputName, keyCode, callback, filter}
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
    inline InputFilter always = []() { return true; };
    }; // namespace Filters

    EventInput &addEventInput(std::string inputName, int keyCode, int mods, int action, InputCallback callback,
                            InputFilter filter = Filters::always, bool isScanCode = true);
    
    ContinuousInput &addContinuousInput(std::string inputName, int keyCode, InputCallback callback,
                                        InputFilter filter = Filters::always);

    void processEventInput(const GLFWKeyInfo &event);
    void processContinuousInputs();
    void clearInputs();
    void clearEventInputs();
    void clearContinuousInputs();

    vec2 getMousePosition();

    std::string getInputKeyString(std::string inputName);

};

#endif