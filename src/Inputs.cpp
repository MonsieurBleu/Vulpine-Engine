#include <Globals.hpp>
#include <Inputs.hpp>

void giveCallbackToApp(GLFWKeyInfo input)
{
    inputs.add(input);
}

void InputBuffer::add(GLFWKeyInfo input)
{
    inputMutex.lock();
    push_back(input);
    inputMutex.unlock();
};

bool InputBuffer::pull(GLFWKeyInfo &input)
{
    inputMutex.lock();

    bool eventPulled = this->size() != 0;

    if (eventPulled)
    {
        input = front();
        pop_front();
    }

    inputMutex.unlock();

    return eventPulled;
};

EventInput &InputManager::addEventInput(std::string inputName, int keyCode, int mods, int action,
                                        InputCallback callback, InputFilter filter, bool isScanCode)
{
    EventInput input(inputName, keyCode, callback, filter, mods, action, isScanCode);
    eventInputs.push_back(input);
    return eventInputs.back();
};

ContinuousInput &InputManager::addContinuousInput(std::string inputName, int keyCode, InputCallback callback,
                                                  InputFilter filter)
{
    ContinuousInput input(inputName, keyCode, callback, filter);
    continuousInputs.push_back(input);
    return continuousInputs.back();
};

void InputManager::clearInputs()
{
    eventInputs.clear();
    continuousInputs.clear();
};

void InputManager::clearEventInputs()
{
    eventInputs.clear();
};

void InputManager::clearContinuousInputs()
{
    continuousInputs.clear();
};

void InputManager::processEventInput(const GLFWKeyInfo &event)
{
    for (auto handler : eventInputs)
    {
        if(!handler.activated) continue;

        int handlerCode = handler.isScanCode ? glfwGetKeyScancode(handler.keyCode) : handler.keyCode;
        int eventCode = handler.isScanCode ? event.scanCode : event.key;
        if (handlerCode == eventCode && handler.mods == event.mods && handler.action == event.action)
        {
            if (handler.filter())
            {
                handler.callback();
            }
        }
    }
}

void InputManager::processContinuousInputs()
{
    for (auto handler : continuousInputs)
    {
        if (glfwGetKey(globals.getWindow(), handler.keyCode) == GLFW_PRESS)
        {
            if (handler.filter())
            {
                handler.callback();
            }
        }
    }
}

std::string InputManager::getInputKeyString(std::string inputName)
{
    std::string keyString = "";

    for (auto handler : eventInputs)
    {
        if (handler.inputName == inputName)
        {
            if (handler.mods & GLFW_MOD_SHIFT)
                keyString += "Shift + ";

            if (handler.mods & GLFW_MOD_CONTROL)
                keyString += "Ctrl + ";

            if (handler.mods & GLFW_MOD_ALT)
                keyString += "Alt + ";

            if (handler.mods & GLFW_MOD_SUPER)
                keyString += "Super + ";

            keyString += glfwGetKeyName(handler.keyCode, 0);
        }
    }

    return keyString;
}