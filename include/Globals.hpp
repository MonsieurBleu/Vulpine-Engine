#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <App.hpp>
#include <Timer.hpp>

#include <Mesh.hpp>

class Globals
{   
    friend App;

    private :
    
        AppState _state;

        const GLFWvidmode* _videoMode;

        glm::vec2 _mousePosition;

        ivec2 _windowSize;
        ivec2 _renderSize;
        float _renderScale = 1.0;

        std::vector<ShaderUniform> _standartShaderUniform2D;
        std::vector<ShaderUniform> _standartShaderUniform3D;

        Mesh _fullscreenQuad;

        bool _mouseLeftClick = false;

    public :

        /**
         * Globals are not cloneable.
         */
        // Globals(Globals &other) = delete;
        /**
         * Globals are note assignable.
         */
        // void operator=(const Globals&) = delete;

        Camera *currentCamera;

        const AppState state() const;

        const GLFWvidmode* videoMode() const;

        glm::ivec2 screenResolution() const;
        const int* screenResolutionAddr() const;
        const glm::vec2 mousePosition() const;

        BenchTimer appTime;
        BenchTimer unpausedTime;

        std::vector<ShaderUniform> standartShaderUniform2D() const;
        std::vector<ShaderUniform> standartShaderUniform3D() const;

        int windowWidth() const;
        int windowHeight() const;
        ivec2 windowSize() const;
        const ivec2* windowSizeAddr() const;

        float renderScale() const;
        ivec2 renderSize() const;
        const ivec2* renderSizeAddr() const;

        void drawFullscreenQuad();

        bool mouseLeftClick();
};

extern Globals globals;

#endif