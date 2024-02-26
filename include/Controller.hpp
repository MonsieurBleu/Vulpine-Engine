#pragma once

#include <Camera.hpp>
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
         * @brief Called each time the mouse mouve
         */
        virtual void mouseEvent(vec2 dir, GLFWwindow* window);
};

class SpectatorController : public Controller
{
    private : 
        bool sprintActivated;
        int upFactor = 0;
        int frontFactor = 0;
        int rightFactor = 0;

    public : 
        float speed = 2.0;
        float sprintFactor = 15.0;
        void update();
        bool inputs(GLFWKeyInfo& input);
};
