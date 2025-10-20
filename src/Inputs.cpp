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
    for (auto handler : eventInputs)
    {
        if (handler.inputName == inputName)
        {
            return getInputKeyString(handler);
        }
    }

    WARNING_MESSAGE("Can't find input name " << inputName);

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
    {"MENU",348}
};

std::string InputManager::getInputKeyString(const EventInput& input)
{
    std::string keyString;

    if (input.mods & GLFW_MOD_SHIFT)
        keyString += "Shift + ";

    if (input.mods & GLFW_MOD_CONTROL)
        keyString += "Ctrl + ";

    if (input.mods & GLFW_MOD_ALT)
        keyString += "Alt + ";

    if (input.mods & GLFW_MOD_SUPER)
        keyString += "Super + ";

    // keyString = glfwGetKeyName(input.keyCode, 0);

    

    if(input.keyCode >= GLFW_MOUSE_BUTTON_1 && input.keyCode <= GLFW_MOUSE_BUTTON_8)
    {
        return keyString + "MOUSE " + std::to_string(input.keyCode - GLFW_MOUSE_BUTTON_1 + 1);
    }
    else
    {
        for(auto &i : nonPrintableKeysNames)
        {
            if(i.second == input.keyCode)
                return keyString + i.first;
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
        WARNING_MESSAGE("Can't retreive key name " << input.keyCode);
        keyString += "UNKNOWN KEY";
    }

    return keyString;
}

vec2 InputManager::getMousePosition()
{
    double x, y;
    glfwGetCursorPos(globals.getWindow(), &x, &y);
    return vec2(x, y);
}