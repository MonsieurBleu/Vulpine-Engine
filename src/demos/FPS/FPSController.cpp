#define _USE_MATH_DEFINES
#include "demos/FPS/FPSController.hpp"
#include <iostream>
#include <math.h>

float FPSVariables::cursorXOld = 0.0f;
float FPSVariables::cursorYOld = 0.0f;
float FPSVariables::cursorXNew = 0.0f;
float FPSVariables::cursorYNew = 0.0f;

bool FPSVariables::W = false;
bool FPSVariables::A = false;
bool FPSVariables::S = false;
bool FPSVariables::D = false;

bool FPSVariables::grounded = false;
bool FPSVariables::lockJump = false;

std::vector<RigidBodyRef> FPSVariables::thingsYouCanStandOn;

FPSController::FPSController(GLFWwindow *window, RigidBodyRef body, Camera *camera, InputBuffer *inputs) : body(body), inputs(inputs)
{
    // glfwSetKeyCallback(window, [](GLFWwindow *window, int key, int scancode, int action, int mods)
    //                    {
    //     GLFWKeyInfo input;
    //     input.window = window;
    //     input.key = key;
    //     input.scancode = scancode;
    //     input.action = action;
    //     input.mods = mods;

    //     giveCallbackToApp(input); });

    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glfwSetCursorPosCallback(window, [](GLFWwindow *window, double xpos, double ypos)
    //                          {  FPSVariables::cursorXNew = xpos;
    //                             FPSVariables::cursorYNew = ypos; });

    // glfwSetCursorPos(window, 0, 0);
}

FPSController::~FPSController()
{
}

void FPSController::doInputs(GLFWKeyInfo& input)
{
    doJump = false;
    if (input.action == GLFW_PRESS)
    {
        switch (input.key)
        {
        case GLFW_KEY_W:
            FPSVariables::W = true;
            break;
        case GLFW_KEY_S:
            FPSVariables::S = true;
            break;
        case GLFW_KEY_A:
            FPSVariables::A = true;
            break;
        case GLFW_KEY_D:
            FPSVariables::D = true;
            break;
        case GLFW_KEY_SPACE:
            doJump = true;
            break;
        default:
            // std::cout << "Key: " << input.key << "\n";
            break;
        }
    }
    else if (input.action == GLFW_RELEASE)
    {
        switch (input.key)
        {
        case GLFW_KEY_W:
            FPSVariables::W = false;
            break;
        case GLFW_KEY_S:
            FPSVariables::S = false;
            break;
        case GLFW_KEY_A:
            FPSVariables::A = false;
            break;
        case GLFW_KEY_D:
            FPSVariables::D = false;
            break;

        default:
            // std::cout << "Key: " << input.key << "\n";
            break;
        }
    }
}

void FPSController::update(float deltaTime)
{
    float forward = 0.0f;
    float side = 0.0f;

    if (FPSVariables::W)
    {
        forward += FPSVariables::forwardSpeed;
    }
    if (FPSVariables::S)
    {
        forward -= FPSVariables::backSpeed;
    }
    if (FPSVariables::A)
    {
        side -= FPSVariables::sideSpeed;
    }
    if (FPSVariables::D)
    {
        side += FPSVariables::sideSpeed;
    }

    // grounded test
    Ray ray{body->getPosition() + vec3(0, -1.95, 0), vec3(0.0f, -1.0f, 0.0f)};
    float t;
    RigidBodyRef bodyIntersect;
    FPSVariables::grounded = raycast(ray, FPSVariables::thingsYouCanStandOn, 0.2f, t, bodyIntersect);
    if (!FPSVariables::grounded)
        FPSVariables::lockJump = false;

    // std::cout << "grounded: " << FPSVariables::grounded << "\n";
    // std::cout << "ray: " << ray.origin.x << ", " << ray.origin.y << ", " << ray.origin.z << "\n";
    // std::cout << "     " << ray.direction.x << ", " << ray.direction.y << ", " << ray.direction.z << "\n";
    // std::cout << "t: " << t << "\n";
    // std::cout << "\n";

    this->friction(deltaTime);

    if (doJump && !FPSVariables::lockJump)
    {
        this->jump(deltaTime);
    }

    move(forward, side, deltaTime);

    vec3 pos = body->getPosition();

    // head bobbing
    float bob = sin(glfwGetTime() * 10.0f) * 0.1f;
    float speed = length(vec2(body->getVelocity().x, body->getVelocity().z));
    if (speed > 0)
        pos.y += bob;
    globals.currentCamera->setPosition(pos);

    // std::cout << "\r" << speed << " m/s   " << std::flush;
    // std::cout << "\r" << body->getVelocity().x << ", " << body->getVelocity().y << ", " << body->getVelocity().z << " m/s   " << std::flush;

    mouseLook();
}

vec3 nnormalize(vec3 v)
{
    return (v[0] == 0 && v[1] == 0 && v[2] == 0) ? vec3(0) : normalize(v);
}

void FPSController::move(float fmove, float smove, float deltaTime)
{
    vec3 forward, right;

    const vec3 camDir = globals.currentCamera->getDirection();
    right = normalize(cross(camDir, vec3(0.0f, 1.0f, 0.0f)));
    forward = normalize(cross(vec3(0.0f, 1.0f, 0.0f), right));

    forward = nnormalize(forward);

    right = nnormalize(right);

    vec3 wishVel = forward * fmove + right * smove;

    vec3 wishDir;
    wishDir = nnormalize(wishVel);

    float wishSpeed = length(wishVel);

    if (wishSpeed > FPSVariables::maxSpeed)
    {
        wishVel *= FPSVariables::maxSpeed / wishSpeed;
        wishSpeed = FPSVariables::maxSpeed;
    }

    // std::cout << "wishVel: " << wishVel.x << ", " << wishVel.y << ", " << wishVel.z << "\n";
    // std::cout << "wishDir: " << wishDir.x << ", " << wishDir.y << ", " << wishDir.z << "\n";
    // std::cout << "wishSpeed: " << wishSpeed << "\n";
    // std::cout << "forward: " << forward.x << ", " << forward.y << ", " << forward.z << "\n";
    // std::cout << "right: " << right.x << ", " << right.y << ", " << right.z << "\n";
    // std::cout << "camera direction: " << camera->getDirection().x << ", " << camera->getDirection().y << ", " << camera->getDirection().z << "\n";
    // std::cout << std::endl;

    if (FPSVariables::grounded)
    {
        // std::cout << "grounded\n";
        this->accelerate(wishDir, wishSpeed, FPSVariables::acceleration, deltaTime);
    }
    else
    {
        // std::cout << "not grounded\n";
        this->airAccelerate(wishDir, wishSpeed, FPSVariables::airAcceleration, deltaTime);
    }
}

void FPSController::accelerate(vec3 wishDirection, float wishSpeed, float accel, float deltaTime)
{
    vec3 vel = body->getVelocity();
    float currentSpeed = dot(vel, wishDirection);
    float addSpeed = wishSpeed - currentSpeed;

    if (addSpeed <= 0)
    {
        return;
    }

    float accelSpeed = accel * deltaTime * wishSpeed;

    if (accelSpeed > addSpeed)
    {
        accelSpeed = addSpeed;
    }

    vel += accelSpeed * wishDirection;

    // std::cout << "accel: " << accel << "\n";
    // std::cout << "wishDirection: " << wishDirection.x << ", " << wishDirection.y << ", " << wishDirection.z << "\n";

    body->setVelocity(vel);
}

void FPSController::airAccelerate(vec3 wishDirection, float wishSpeed, float accel, float deltaTime)
{
    vec3 vel = body->getVelocity();
    float currentSpeed = dot(vel, wishDirection);
    float addSpeed = wishSpeed - currentSpeed;

    if (wishSpeed > FPSVariables::maxSpeed)
    {
        wishSpeed = FPSVariables::maxSpeed;
    }

    if (addSpeed <= 0)
    {
        return;
    }

    float accelSpeed = accel * deltaTime * wishSpeed;

    if (accelSpeed > addSpeed)
    {
        accelSpeed = addSpeed;
    }

    vel += accelSpeed * wishDirection;

    body->setVelocity(vel);
}

void FPSController::friction(float deltaTime)
{
    vec3 vel = body->getVelocity();
    float speed = length(vel);

    if (speed < 0.1f)
        return;

    float drop = 0.0f;

    if (FPSVariables::grounded)
    {
        float control = speed < FPSVariables::stopSpeed ? FPSVariables::stopSpeed : speed;
        drop = control * .6 * deltaTime;
    }

    float newSpeed = speed - drop;
    if (newSpeed < 0)
        newSpeed = 0;

    newSpeed /= speed;

    vel *= newSpeed;

    body->setVelocity(vel);
}

void FPSController::jump(float deltaTime)
{
    if (!FPSVariables::grounded)
        return;
    vec3 vel = body->getVelocity();
    vel.y += FPSVariables::jumpForce;

    body->setVelocity(vel);

    FPSVariables::grounded = false;
    FPSVariables::lockJump = true;
}

#define YAW 0
#define PITCH 1
#define ROLL 2

void FPSController::mouseLook()
{
    /*
    float xoffset = FPSVariables::cursorXOld - FPSVariables::cursorXNew;
    float yoffset = FPSVariables::cursorYOld - FPSVariables::cursorYNew;

    FPSVariables::cursorXOld = FPSVariables::cursorXNew;
    FPSVariables::cursorYOld = FPSVariables::cursorYNew;

    const float sensitivity = FPSVariables::sensitivity;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    const vec3 camDir = globals.currentCamera->getDirection();
    const vec3 camRight = normalize(cross(camDir, vec3(0.0f, 1.0f, 0.0f)));
    // const vec3 camUp = normalize(cross(camRight, camDir));

    quat pitchQuat = angleAxis(yoffset, camRight);
    quat yawQuat = angleAxis(xoffset, vec3(0.0f, 1.0f, 0.0f));

    quat newRotation = yawQuat * pitchQuat * globals.currentCamera->getDirection();

    // print camera->getDirection()
    // std::cout << "Camera direction: " << camera->getDirection().x << ", " << camera->getDirection().y << ", " << camera->getDirection().z << "\n";
    // std::cout << "camera position: " << camera->getPosition().x << ", " << camera->getPosition().y << ", " << camera->getPosition().z << "\n";
    vec3 rot = eulerAngles(newRotation);
    globals.currentCamera->setDirection(rot);
    */
}
