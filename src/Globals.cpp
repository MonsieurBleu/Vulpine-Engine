#include <Globals.hpp>

const AppState Globals::state() const {return _state;};

const GLFWvidmode* Globals::videoMode() const {return _videoMode;};

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
