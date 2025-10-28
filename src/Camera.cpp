#include <iostream>
#include <tgmath.h>

#define  GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include <Graphics/Camera.hpp>
#include <Utils.hpp>

#include <CompilingOptions.hpp>

#define PI 3.14159265358979323846

using namespace glm;

glm::mat4 MakeInfReversedZProjRH(float fovY_radians, float aspectWbyH, float zNear)
{
    float f = 1.0f / tan(fovY_radians / 2.0f);
    return glm::mat4(
        f / aspectWbyH, 0.0f,   0.0f,   0.0f,
        0.0f,           f,      0.0f,   0.0f,
        0.0f,           0.0f,   0.0f,   -1.0f,
        0.0f,           0.0f,   zNear,  0.0f);
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
    if (state.forceLookAtpoint)
        state.direction = -normalize(state.position - state.lookpoint);
    else
        state.lookpoint = state.position + state.direction;
    
    viewMatrix = glm::lookAt(state.position, state.lookpoint, wup);
}

void Camera::updateProjectionMatrix()
{
    switch (type)
    {
    case PERSPECTIVE:
#ifdef INVERTED_Z
        projectionMatrix = MakeInfReversedZProjRH(state.FOV, width / height, state.nearPlane);
#else
        projectionMatrix = perspective(state.FOV, width / height, state.nearPlane, state.farPlane);
#endif
        break;

    case ORTHOGRAPHIC:
#ifdef INVERTED_Z
        projectionMatrix = glm::ortho<float>(-dimentionFactor*width / 2, dimentionFactor*width / 2, -dimentionFactor*height / 2, dimentionFactor*height / 2, state.farPlane, state.nearPlane);
#else
        projectionMatrix = glm::ortho<float>(-dimentionFactor*width / 2, dimentionFactor*width / 2, -dimentionFactor*height / 2, dimentionFactor*height / 2, state.nearPlane, state.farPlane);
#endif
        break;

    default:
        break;
    }
}

Frustum Camera::getFrustum() { return frustum; }

void Camera::updateFrustum()
{
    if(type == PERSPECTIVE)
    {
        const vec3 front = state.forceLookAtpoint ? normalize(state.lookpoint - state.position) : state.direction;

        const vec3 right = normalize(cross(wup, front));
        const vec3 up = cross(right, front);
        const vec3 p = state.position;
        const float d = length(p);

        const float n = state.nearPlane;
        const float f = state.farPlane;

        const float halfVSide = f * tanf(state.FOV * .5f);
        const float halfHSide = halfVSide * width / height;
        const vec3 fvec = f * front;

        frustum.near_.position = p + (n * front);
        frustum.near_.distance = length(frustum.near_.position);
        frustum.near_.normal = front;

        frustum.far_.position = p + fvec;
        frustum.far_.distance = length(frustum.far_.position);
        frustum.far_.normal = -front;

        frustum.right.distance = d;
        frustum.right.position = p;
        frustum.right.normal = normalize(cross(fvec - (right * halfHSide), up));

        frustum.left.distance = d;
        frustum.left.position = p;
        frustum.left.normal = normalize(cross(up, fvec + (right * halfHSide)));

        frustum.top.distance = d;
        frustum.top.position = p;
        frustum.top.normal = normalize(cross(right, fvec - up * halfVSide));

        frustum.bottom.distance = d;
        frustum.bottom.position = p;
        frustum.bottom.normal = normalize(cross(fvec + up * halfVSide, right));
    }
    else
    {
        const vec3 front = state.forceLookAtpoint ? normalize(state.lookpoint - state.position) : state.direction;
        const vec3 right = normalize(cross(wup, front));
        const vec3 up = cross(right, front);
        const vec3 p = state.position;
        
        const float n = state.nearPlane;
        const float f = state.farPlane;

        const float w = dimentionFactor*width/2;
        const float h = dimentionFactor*height/2;
 
        frustum.near_.position = p + (n * front);
        frustum.near_.normal = front;

        frustum.far_.position = p + (f * front);
        frustum.far_.normal = -front;

        frustum.right.position = p + (w * right);
        frustum.right.normal = -right;

        frustum.left.position = p - (w * right);
        frustum.left.normal = right;

        frustum.top.position = p + (h * up);
        frustum.top.normal = -up;

        frustum.bottom.position = p - (h * up);
        frustum.bottom.normal = up;
    }
}

const mat4 Camera::updateProjectionViewMatrix()
{
    updateViewMatrix();
    updateProjectionMatrix();
    projectionViewMatrix = projectionMatrix * viewMatrix * mat4(1.0);

    updateFrustum();

    return projectionViewMatrix;
}

void Camera::updateMouseFollow(GLFWwindow *window)
{
    // if(!state.followMouse) return;

    // vec<2, double, glm::packed_highp> Mouse_uv;
    // glfwGetCursorPos(window, &Mouse_uv.x, &Mouse_uv.y);

    // float th = state.sensitivity*Mouse_uv.y*PI*(1.0/height); // polar theta of direction
    // float ph = state.sensitivity*Mouse_uv.x*PI*(1.0/width); // polar phi of direction

    // state.direction = normalize(vec3(sin(th)*cos(ph), cos(th), sin(th)*sin(ph)));
}

void Camera::move(vec3 velocity, double deltatime)
{
    velocity *= deltatime;
    state.position += state.direction * velocity.x;

    state.position += cross(wup, state.direction) * velocity.z;

    state.position.y += velocity.y;
}

void Camera::setState(CameraState &newState)
{
    state = newState;
}

void Camera::setType(CameraType t)
{
    type = t;
}

CameraType Camera::getType()
{
    return type;
}