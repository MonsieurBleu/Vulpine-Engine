#include <Globals.hpp>
#include <App.hpp>

const AppState Globals::state() const {return _state;};

const GLFWvidmode* Globals::videoMode() const {return _videoMode;};

const Controller * Globals::getController() const {return _currentController;};

glm::ivec2 Globals::screenResolution() const {return ivec2(_videoMode->width, _videoMode->height);};
const int* Globals::screenResolutionAddr() const {return &(_videoMode->width);};
const glm::vec2 Globals::mousePosition() const {return _mousePosition;};

std::vector<ShaderUniform> Globals::standartShaderUniform2D() const {return _standartShaderUniform2D;};
std::vector<ShaderUniform> Globals::standartShaderUniform3D() const {return _standartShaderUniform3D;};

int Globals::windowWidth() const {return _windowSize.x;};
int Globals::windowHeight() const {return _windowSize.y;};
ivec2 Globals::windowSize() const {return _windowSize;};
const ivec2* Globals::windowSizeAddr() const {return &_windowSize;};

float Globals::renderScale() const {return _renderScale;};
ivec2 Globals::renderSize() const {return _renderSize;};
const ivec2* Globals::renderSizeAddr() const {return &_renderSize;};

void Globals::drawFullscreenQuad() {_fullscreenQuad.drawVAO();};

bool Globals::mouseLeftClick(){return _mouseLeftClick;};
bool Globals::mouseLeftClickDown(){return _mouseLeftClickDown;};
vec2 Globals::mouseScrollOffset(){return _scrollOffset;};
void Globals::clearMouseScroll(){_scrollOffset = vec2(0);};

Scene *Globals::getScene() const {return _gameScene;};

bool Globals::useTextInputs(void* user)
{
    textInputString.clear();
    bool ret = currentTextInputUser == nullptr;
    currentTextInputUser = user;
    return ret;
}

bool Globals::endTextInputs(void* user)
{
    if(currentTextInputUser == user)
    {
        currentTextInputUser = nullptr;
        textInputString.clear();
        return true;
    }
    return false;
}

bool Globals::canUseTextInputs(void* user)
{
    return currentTextInputUser == user;
}

bool Globals::isTextInputsActive()
{
    return currentTextInputUser != nullptr;
}

bool Globals::getTextInputs(void* user, std::u32string& buff)
{
    if(currentTextInputUser == user)
    {
        buff.clear();
        buff += textInputString;
        return true;
    }

    return false;
}

int Globals::windowHasFocus(){return _windowHasFocus;};


