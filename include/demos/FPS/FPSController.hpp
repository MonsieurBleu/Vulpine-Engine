#include "PhysicsEngine.hpp"
#include "Camera.hpp"
#include "Inputs.hpp"

#include <iostream>
#include <vector>

using namespace glm;

struct FPSVariables
{
    static constexpr float maxSpeed = 32.0f;
    static constexpr float stopSpeed = 20.0f;
    static constexpr float acceleration = 2.0f;
    static constexpr float airAcceleration = 0.07f;
    static constexpr float friction = 20.0f;
    static constexpr float jumpForce = 60.0f;
    static constexpr float sensitivity = 0.01f;

    static constexpr float forwardSpeed = 20.0f;
    static constexpr float sideSpeed = 35.0f;
    static constexpr float backSpeed = 20.0f;

    static float cursorXOld;
    static float cursorYOld;

    static float cursorXNew;
    static float cursorYNew;

    static bool W;
    static bool A;
    static bool S;
    static bool D;

    static bool grounded;
    static bool lockJump;

    // probably should have a better name
    static std::vector<RigidBodyRef> thingsYouCanStandOn;
};

class FPSController
{
private:
    RigidBodyRef body;
    Camera *camera;
    InputBuffer *inputs;

public:
    FPSController(GLFWwindow *window, RigidBodyRef body, Camera *camera, InputBuffer *inputs);
    ~FPSController();

    void update(float deltaTime);

    void move(float forward, float side, float deltaTime);
    void accelerate(vec3 wishDirection, float wishSpeed, float accel, float deltaTime);
    void airAccelerate(vec3 wishDirection, float wishSpeed, float accel, float deltaTime);
    void friction(float deltaTime);
    void jump(float deltaTime);

    void mouseLook();
};