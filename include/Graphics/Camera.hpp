#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>

#pragma once 

#define GLM_FORCE_RADIAN 
#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> 

using namespace glm;

enum CameraType
{
    PERSPECTIVE,
    ORTHOGRAPHIC
};

struct CameraState
{
        float FOV = 1.0f;
        float nearPlane = 0.1f;
        float farPlane = 100.f;
        float sensitivity = 1.5;
        bool  followMouse = true;
        bool  forceLookAtpoint = false;
        vec3 position;
        vec3 lookpoint;
        vec3 direction;
};

struct FrustumPlane
{
    vec3 normal;
    vec3 position;
    /* the distance seems useless */
    float distance;
};

struct Frustum
{
    FrustumPlane top;
    FrustumPlane bottom;
    FrustumPlane right;
    FrustumPlane left;
    FrustumPlane far_;
    FrustumPlane near_;
};

class Camera 
{ 
    private: 


        mat4 projectionMatrix; 
        mat4 viewMatrix; 
        mat4 projectionViewMatrix;

        CameraType type = PERSPECTIVE;

        Frustum frustum;

    public: 

        float dimentionFactor = 1.f;
        float width = 1920.f;
        float height = 1080.f;

        vec3 wup = vec3(0, 1, 0);
        
        CameraState state;
        
        Camera(CameraType type = PERSPECTIVE);

        void init(float FOV, float width, float height, float nearplane, float farPlane);

        void setPosition(vec3 _position); 
        void lookAt(vec3 _position);

        const vec3 getPosition() {return state.position;};
        const vec3 getDirection() {return state.direction;};
        const vec3 getLookpoint() {return state.lookpoint;};

        vec3* getPositionAddr() {return &state.position;};
        vec3* getDirectionAddr() {return &state.direction;};
        vec3* getLookpointAddr() {return &state.lookpoint;};

        const mat4 getViewMatrix() {return viewMatrix;};
        const mat4 getProjectionMatrix() {return projectionMatrix;};
        const mat4 getProjectionViewMatrix() {return projectionViewMatrix;};

        const mat4* getViewMatrixAddr() {return &viewMatrix;};
        const mat4* getProjectionMatrixAddr() {return &projectionMatrix;};
        const mat4* getProjectionViewMatrixAddr() {return &projectionViewMatrix;};
        
        void updateFrustum();
        Frustum getFrustum();

        void updateViewMatrix();
        void updateProjectionMatrix();
        const mat4 updateProjectionViewMatrix();
        const mat4* getProjectionViewMatrixAddr() const {return &projectionViewMatrix;};

        void updateMouseFollow(GLFWwindow *window);

        void toggleMouseFollow(){state.followMouse = !state.followMouse;};
        void setMouseFollow(bool enable){state.followMouse = enable;};
        bool getMouseFollow(){return state.followMouse;};

        void setDirection(vec3 direction){state.direction = direction;};

        void toggleForceLookAtPoint(){state.forceLookAtpoint = !state.forceLookAtpoint;};
        void setForceLookAtPoint(bool enable){state.forceLookAtpoint = enable;};

        void move(vec3 velocity, double deltatime);

        void setState(CameraState& newState);
        void setType(CameraType t);
        CameraType getType();
        CameraState& getState() {return state;};
};

#endif