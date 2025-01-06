#pragma once

#include <Graphics/Camera.hpp>
#include <Inputs.hpp>

class Controller
{
    protected : 
        vec3 deplacementDir;

    public : 
        /**
         * @brief Called at each frame, right after inputs management
         */
        virtual void update(){};

        /**
         * @brief This function will be called for each input 
         * 
         * @param input Engine defined input information
         * @return true if the current input is used and/or don't need to be use by any other object
         */
        virtual bool inputs(GLFWKeyInfo& input){return false;};

        /**
         * @brief Called each time the mouse moves
         */
        virtual void mouseEvent(vec2 dir, GLFWwindow* window);

        /**
         * @brief Called when this controller is switched for another one 
         */
        virtual void clean(){};

        /**
         * @brief Called when this controller is enabled as actif
         */
        virtual void init(){};
};

class SpectatorController : public Controller
{
    protected : 
        bool sprintActivated = false;
        int upFactor = 0;
        int frontFactor = 0;
        int rightFactor = 0;

        void updateDirectionStateWASD();

    public : 
        float speed = 2.0;
        float sprintFactor = 15.0;
        virtual void update();
        virtual bool inputs(GLFWKeyInfo& input);
        void clean();
        void init();
};

class OrbitController : public Controller
{
    protected : 

    public : 
        vec3 position = vec3(0);
        float distance = 10.f;
        virtual void update();
        // virtual bool inputs(GLFWKeyInfo& input);
        void clean();
        void init();

        void mouseEvent(vec2 dir, GLFWwindow* window);
};


// drag controller
// doesn't take into account the camera, only the mouse position
class DragController2D : public Controller
{
    protected : 
        vec2 position = vec2(0);
        vec2 lastPosition = vec2(0);
        vec2 delta = vec2(0);
        bool mouseDown = false;
        bool dragging = false;
        bool clicked = false;

    public : 
        float scale = 1.f;
        virtual void update();
        virtual bool inputs(GLFWKeyInfo& input);
        void clean();
        void init();

        void mouseEvent(vec2 dir, GLFWwindow* window);

        vec2 getPosition(){return position;};
        vec2 getDelta(){return delta;};
        bool isDragging(){return dragging;};
        float getScale(){return scale;};
};
