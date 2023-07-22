#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <App.hpp>
#include <Timer.hpp>

class Globals
{   
    friend App;

    private :
    
        AppState _state;

        const GLFWvidmode* _videoMode;

        glm::vec2 _mousePosition;


    public :

        /**
         * Globals are not cloneable.
         */
        // Globals(Globals &other) = delete;
        /**
         * Globals are note assignable.
         */
        // void operator=(const Globals&) = delete;


        const AppState state() const {return _state;};

        const GLFWvidmode* videoMode() const {return _videoMode;};

        glm::ivec2 screenResolution() const {return ivec2(_videoMode->width, _videoMode->height);};
        const int* screenResolutionAddr() const {return &(_videoMode->width);};
        const glm::vec2 mousePosition() const {return _mousePosition;};

        // const BenchTimer appTime() const {};
        // const BenchTimer unposedTime()const {};


        BenchTimer appTime;
        BenchTimer unposedTime;
};


Globals globals;

#endif