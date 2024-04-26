#include <Camera.hpp>
#include <Inputs.hpp>
#include <Physics.hpp>

#include <glm/glm.hpp>
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

    static constexpr float forwardSpeed = 15.0f; // 20
    static constexpr float sideSpeed = 15.0f;    // 35
    static constexpr float backSpeed = 10.0f;    // 20

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
};

class FPSController
{
  private:
    RigidBody::Ref body;
    InputBuffer *inputs;
    bool doJump;

  public:
    FPSController(GLFWwindow *window, RigidBody::Ref body, Camera *camera, InputBuffer *inputs);
    ~FPSController();

    void update(float deltaTime);

    void doInputs(GLFWKeyInfo &input);

    void move(float forward, float side, float deltaTime);
    void accelerate(vec3 wishDirection, float wishSpeed, float accel, float deltaTime);
    void airAccelerate(vec3 wishDirection, float wishSpeed, float accel, float deltaTime);
    void friction(float deltaTime);
    void jump(float deltaTime);

    void mouseLook();
};