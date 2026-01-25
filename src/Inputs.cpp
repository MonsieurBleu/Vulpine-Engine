#include "AssetManager.hpp"
#include "Scripting/ScriptInstance.hpp"
#include <Globals.hpp>
#include <Inputs.hpp>
#include <Utils.hpp>

InputFilter InputManager::Filters::always = [](){return !globals.isTextInputsActive();};

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
                                        InputCallback callback, InputFilter filter, bool isScanCode, InputCallback falseCondCallback)
{
    EventInput input(inputName, keyCode, callback, filter, mods, action, isScanCode, falseCondCallback);
    eventInputs.push_back(input);
    return eventInputs.back();
};

EventInput &InputManager::addEventInput(std::string inputName, int keyCode, int mods, int action,
                                        std::string luaCallbackFilename, InputFilter filter, bool isScanCode, InputCallback falseCondCallback)
{
    EventInput input(inputName, keyCode, luaCallbackFilename, filter, mods, action, isScanCode, falseCondCallback);
    eventInputs.push_back(input);
    return eventInputs.back();
};

ContinuousInput &InputManager::addContinuousInput(std::string inputName, int keyCode, InputCallback callback,
                                                  InputFilter filter, InputCallback falseCondCallback)
{
    ContinuousInput input(inputName, keyCode, callback, filter, falseCondCallback);
    continuousInputs.push_back(input);
    return continuousInputs.back();
};

ContinuousInput &InputManager::addContinuousInput(std::string inputName, int keyCode,
                                                  std::string luaCallbackFilename, InputFilter filter, InputCallback falseCondCallback)
{
    ContinuousInput input(inputName, keyCode, luaCallbackFilename, filter, falseCondCallback);
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
        
        int handlerCode;
        int eventCode;
        if ((handler.keyCode & VULPINE_GAMEPAD_BIT) == 0)
        {   
            handlerCode = handler.isScanCode ? glfwGetKeyScancode(handler.keyCode) : handler.keyCode;
            eventCode = handler.isScanCode ? event.scanCode : event.key;
        }
        else {
            handlerCode = handler.keyCode;
            eventCode = event.key;
        }

        bool modsGood = !handler.mods || handler.mods == event.mods;

        if (handlerCode == eventCode && modsGood && handler.action == event.action)
        {
            if (handler.filter())
            {
                handler();
            }
        }
    }
}

void InputManager::processContinuousInputs()
{
    for (auto handler : continuousInputs)
    {
        if(!handler.activated)
            continue;

        switch (handler.keyCode) {
            case GLFW_MOUSE_BUTTON_1 :
            case GLFW_MOUSE_BUTTON_2 :
            case GLFW_MOUSE_BUTTON_3 :
            case GLFW_MOUSE_BUTTON_4 :
            case GLFW_MOUSE_BUTTON_5 :
            case GLFW_MOUSE_BUTTON_6 :
            case GLFW_MOUSE_BUTTON_7 :
            case GLFW_MOUSE_BUTTON_8 :
                if(handler.filter)
                {
                    if(glfwGetMouseButton(globals.getWindow(), handler.keyCode) == GLFW_PRESS)
                        handler();
                    else if(handler.falseCondCallback)
                        handler.falseCondCallback();
                }
            break;

            default : 
                if(handler.filter)
                {
                    if (handler.keyCode & VULPINE_GAMEPAD_BIT)
                    {
                        int gamepadButton = handler.keyCode & (~VULPINE_GAMEPAD_BIT);
                        if (InputManager::gamepadState.buttons[gamepadButton] == GLFW_PRESS)
                            handler();
                        else if (handler.falseCondCallback)
                            handler.falseCondCallback();
                    }
                    else {
                        if(glfwGetKey(globals.getWindow(), handler.keyCode) == GLFW_PRESS)
                            handler();
                        else if(handler.falseCondCallback)
                            handler.falseCondCallback();
                    }
                }
            break;
        }
    }
}

std::string InputManager::getInputKeyString(std::string inputName)
{
    for (auto handler : eventInputs)
    {
        if (handler.inputName == inputName)
        {
            return getInputKeyString(handler);
        }
    }

    WARNING_MESSAGE("Can't find input name " ,  inputName);

    return "UNKNOWN KEY";
}

std::vector<std::pair<std::string, int>> nonPrintableKeysNames = 
{
    {"MOUSE BUTTON 1", 0},
    {"MOUSE BUTTON 2", 1},
    {"MOUSE BUTTON 3", 2},
    {"MOUSE BUTTON 4", 3},
    {"MOUSE BUTTON 5", 4},
    {"MOUSE BUTTON 6", 5},
    {"MOUSE BUTTON 7", 6},
    {"MOUSE BUTTON 8", 7},
    {"0", 48},
    {"1", 49},
    {"2", 50},
    {"3", 51},
    {"4", 52},
    {"5", 53},
    {"6", 54},
    {"7", 55},
    {"8", 56},
    {"9", 57},
    {"SPACE", GLFW_KEY_SPACE},
    {"ESCAPE",256},
    {"ENTER",257},
    {"TAB",258},
    {"BACKSPACE",259},
    {"INSERT",260},
    {"DELETE",261},
    {"RIGHT",262},
    {"LEFT",263},
    {"DOWN",264},
    {"UP",265},
    {"PAGE UP",266},
    {"PAGE DOWN",267},
    {"HOME",268},
    {"END",269},
    {"CAPS LOCK",280},
    {"SCROLL LOCK",281},
    {"NUM LOCK",282},
    {"PRINT SCREEN",283},
    {"PAUSE",284},
    {"F1",290},
    {"F2",291},
    {"F3",292},
    {"F4",293},
    {"F5",294},
    {"F6",295},
    {"F7",296},
    {"F8",297},
    {"F9",298},
    {"F10",299},
    {"F11",300},
    {"F12",301},
    {"F13",302},
    {"F14",303},
    {"F15",304},
    {"F16",305},
    {"F17",306},
    {"F18",307},
    {"F19",308},
    {"F20",309},
    {"F21",310},
    {"F22",311},
    {"F23",312},
    {"F24",313},
    {"F25",314},
    {"KP 0",320},
    {"KP 1",321},
    {"KP 2",322},
    {"KP 3",323},
    {"KP 4",324},
    {"KP 5",325},
    {"KP 6",326},
    {"KP 7",327},
    {"KP 8",328},
    {"KP 9",329},
    {"KP DECIMAL",330},
    {"KP DIVIDE",331},
    {"KP MULTIPLY",332},
    {"KP SUBTRACT",333},
    {"KP ADD",334},
    {"KP ENTER",335},
    {"KP EQUAL",336},
    {"LEFT SHIFT",340},
    {"LEFT CONTROL",341},
    {"LEFT ALT",342},
    {"LEFT SUPER",343},
    {"RIGHT SHIFT",344},
    {"RIGHT CONTROL",345},
    {"RIGHT ALT",346},
    {"RIGHT SUPER",347},
    {"MENU",348},
    // TODO: Fix name when not using xbox style controller (cross square circle triangle)
    {"GAMEPAD BUTTON A", VULPINE_GAMEPAD_BUTTON_A},
    {"GAMEPAD BUTTON B", VULPINE_GAMEPAD_BUTTON_B},
    {"GAMEPAD BUTTON X", VULPINE_GAMEPAD_BUTTON_X},
    {"GAMEPAD BUTTON Y", VULPINE_GAMEPAD_BUTTON_Y},
    {"GAMEPAD BUTTON LEFT BUMPER", VULPINE_GAMEPAD_BUTTON_LEFT_BUMPER},
    {"GAMEPAD BUTTON RIGHT BUMPER", VULPINE_GAMEPAD_BUTTON_RIGHT_BUMPER},
    {"GAMEPAD BUTTON BACK", VULPINE_GAMEPAD_BUTTON_BACK},
    {"GAMEPAD BUTTON START", VULPINE_GAMEPAD_BUTTON_START},
    {"GAMEPAD BUTTON GUIDE", VULPINE_GAMEPAD_BUTTON_GUIDE},
    {"GAMEPAD BUTTON LEFT THUMB", VULPINE_GAMEPAD_BUTTON_LEFT_THUMB},
    {"GAMEPAD BUTTON RIGHT THUMB", VULPINE_GAMEPAD_BUTTON_RIGHT_THUMB},
    {"GAMEPAD BUTTON DPAD UP", VULPINE_GAMEPAD_BUTTON_DPAD_UP},
    {"GAMEPAD BUTTON DPAD RIGHT", VULPINE_GAMEPAD_BUTTON_DPAD_RIGHT},
    {"GAMEPAD BUTTON DPAD DOWN", VULPINE_GAMEPAD_BUTTON_DPAD_DOWN},
    {"GAMEPAD BUTTON DPAD LEFT", VULPINE_GAMEPAD_BUTTON_DPAD_LEFT}
};

void getInputKeySimple(const GenericInput& input, std::string& keyString)
{
    if(input.keyCode >= GLFW_MOUSE_BUTTON_1 && input.keyCode <= GLFW_MOUSE_BUTTON_8)
    {
        keyString += "MOUSE " + std::to_string(input.keyCode - GLFW_MOUSE_BUTTON_1 + 1);
        return;
    }
    else
    {
        for(auto &i : nonPrintableKeysNames)
        {
            if(i.second == input.keyCode)
            {
                keyString += i.first;
                return;
            }
        }
    }

    const char *name = glfwGetKeyName(input.keyCode, GLFW_KEY_UNKNOWN);
    if(!name)
        name = glfwGetKeyName(GLFW_KEY_UNKNOWN, input.keyCode);

    if(name)
    {
        keyString += name;
    }
    else
    {
        WARNING_MESSAGE("Can't retreive key name " ,  input.keyCode);
        keyString += "UNKNOWN KEY";
    }
}

std::string InputManager::getInputKeyString(const EventInput& input)
{
    std::string keyString = "";

    if (input.mods & GLFW_MOD_SHIFT)
        keyString += "Shift + ";

    if (input.mods & GLFW_MOD_CONTROL)
        keyString += "Ctrl + ";

    if (input.mods & GLFW_MOD_ALT)
        keyString += "Alt + ";

    if (input.mods & GLFW_MOD_SUPER)
        keyString += "Super + ";

    getInputKeySimple((GenericInput)input, keyString);

    return keyString;
}

std::string InputManager::getInputKeyString(const ContinuousInput& input)
{
    std::string keyString = "";

    getInputKeySimple((GenericInput)input, keyString);

    return keyString;
}

vec2 InputManager::getMousePosition()
{
    double x, y;
    glfwGetCursorPos(globals.getWindow(), &x, &y);
    return vec2(x, y);
}

void GenericInput::operator()() const
{
    if (useLuaCallback)
    {
        if (Loader<ScriptInstance>::loadingInfos.find(luaCallbackFilename) != Loader<ScriptInstance>::loadingInfos.end()) {
            Loader<ScriptInstance>::get(luaCallbackFilename).run();
        }
    }
    else
    {
        callback();
    }
}

void InputManager::initJoystick()
{
    for (int i = 0; i < GLFW_JOYSTICK_LAST; i++)
    {
        if (glfwJoystickPresent(i) == GLFW_TRUE)
        {
            currentJoystick = i;
            NOTIF_MESSAGE("Joystick " ,  i ,  " connected");
            return;
        }
    }
}

void InputManager::joystickCallback(int jid, int event)
{
    if (event == GLFW_CONNECTED)
    {
        currentJoystick = jid;
        NOTIF_MESSAGE("Joystick " ,  jid ,  " connected");
    }
    else if (event == GLFW_DISCONNECTED)
    {
        currentJoystick = -1;
        NOTIF_MESSAGE("Joystick " ,  jid ,  " disconnected");
    }
}

float InputManager::getGamepadAxisValue(int axisCode)
{
    if (InputManager::currentJoystick == -1)
    {
        // WARNING_MESSAGE("No gamepad connected");
        return 0.0f;
    }

    return InputManager::gamepadState.axes[axisCode];
}

void InputManager::updateGamepadState()
{
    if (currentJoystick == -1)
        return;

    if (glfwGetGamepadState(currentJoystick, &gamepadState) != GLFW_TRUE)
    {
        WARNING_MESSAGE("Unrecognized gamepad mapping, can't get gamepad input");
        return;
    }
}

std::vector<GLFWKeyInfo> InputManager::pollGamepad()
{
    GLFWgamepadstate prevState = InputManager::gamepadState;
    updateGamepadState();
    GLFWgamepadstate currState = InputManager::gamepadState;

    std::vector<GLFWKeyInfo> inputs;

    for (int i = 0; i < GLFW_GAMEPAD_BUTTON_LAST; i++)
    {
        if (prevState.buttons[i] != currState.buttons[i])
        {
            int keyCode = VULPINE_GAMEPAD_BIT + i;
            // std::cout << "Gamepad button " << i << " changed: " << (currState.buttons[i] == GLFW_PRESS ? "PRESSED" : "RELEASED") << std::endl;
            int action = currState.buttons[i];
            inputs.push_back(GLFWKeyInfo{nullptr, keyCode, keyCode, action, 0});
        }
    }

    return inputs;            
}

bool InputManager::getGamepadButtonValue(int buttonCode)
{
    if (InputManager::currentJoystick == -1)
    {
        // WARNING_MESSAGE("No gamepad connected");
        return false;
    }

    // support vulpine gamepad button codes and standard glfw gamepad button codes
    if (buttonCode & VULPINE_GAMEPAD_BIT)
    {
        buttonCode = buttonCode & (~VULPINE_GAMEPAD_BIT);
    }

    return InputManager::gamepadState.buttons[buttonCode] == GLFW_PRESS;
}

bool InputManager::isGamePadConnected()
{
    return InputManager::currentJoystick != -1;
}