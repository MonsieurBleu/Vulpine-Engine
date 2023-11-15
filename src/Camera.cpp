#include <iostream>
#include <App.hpp>
#include <tgmath.h>

#include <glm/gtx/string_cast.hpp>

#include <Camera.hpp> 
#include <Utils.hpp>

#include <CompilingOptions.hpp>

#define PI 3.14159265358979323846

using namespace glm;

glm::mat4 MakeInfReversedZProjRH(float fovY_radians, float aspectWbyH, float zNear)
{
    float f = 1.0f / tan(fovY_radians / 2.0f);
    return glm::mat4(
        f / aspectWbyH, 0.0f,  0.0f,  0.0f,
                  0.0f,    f,  0.0f,  0.0f,
                  0.0f, 0.0f,  0.0f, -1.0f,
                  0.0f, 0.0f, zNear,  0.0f);
}

Camera::Camera(CameraType type) : type(type)
{
}

void Camera::init(float _FOV, float _width, float _height, float _nearPlane, float _farPlane)
{ 
    state.FOV = _FOV;
    width = _width;
    height = _height;
    state.nearPlane = _nearPlane;
    state.farPlane = _farPlane;

    state.position = vec3(0.0f, 0.0f, 4.0f); 
 
    state.lookpoint = vec3(0.0);
} 
 
void Camera::setPosition(vec3 _position)
{ 
   state.position = _position; 
} 

void Camera::lookAt(vec3 _position)
{
    state.lookpoint = _position;
}

void Camera::updateViewMatrix()
{
    if(state.forceLookAtpoint)
        viewMatrix = glm::lookAt(state.position, state.lookpoint, vec3(0,1,0));
    else
        viewMatrix = glm::lookAt(state.position, state.position+state.direction, vec3(0,1,0));
}

void Camera::updateProjectionMatrix()
{
    switch (type)
    {
    case PERSPECTIVE : 
        #ifdef INVERTED_Z
        projectionMatrix = MakeInfReversedZProjRH(state.FOV, width / height, state.nearPlane);
        #else
        projectionMatrix = perspective(state.FOV, width / height, state.nearPlane, state.farPlane);
        #endif
        break;
    
    case ORTHOGRAPHIC :
        #ifdef INVERTED_Z
        projectionMatrix = glm::ortho<float>(-width/2, width/2, -height/2, height/2, state.farPlane, state.nearPlane);
        #else
        projectionMatrix = glm::ortho<float>(-width/2, width/2, -height/2, height/2, state.nearPlane, state.farPlane);
        #endif
        break;
    
    default:
        break;
    }

}

const mat4 Camera::updateProjectionViewMatrix()
{
    updateViewMatrix();
    updateProjectionMatrix();
    projectionViewMatrix = projectionMatrix*viewMatrix*mat4(1.0);
    return projectionViewMatrix;
}

void Camera::updateMouseFollow(GLFWwindow *window)
{
    if(!state.followMouse) return;

    vec<2, double, glm::packed_highp> Mouse_uv;
    glfwGetCursorPos(window, &Mouse_uv.x, &Mouse_uv.y);

    float th = state.sensitivity*Mouse_uv.y*PI*(1.0/height); // polar theta of direction
    float ph = state.sensitivity*Mouse_uv.x*PI*(1.0/width); // polar phi of direction

    state.direction = normalize(vec3(sin(th)*cos(ph), cos(th), sin(th)*sin(ph)));
}

void Camera::move(vec3 velocity, double deltatime)
{
    velocity *= deltatime;
    state.position += state.direction*velocity.x;

    state.position += cross(vec3(0.f, 1.f, 0.f), state.direction) * velocity.z;

    state.position.y += velocity.y;
}

void Camera::setState(CameraState& newState)
{
    state = newState;
}