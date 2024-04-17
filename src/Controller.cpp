#include <Controller.hpp>
#include <Globals.hpp>

void SpectatorController::clean()
{
    sprintActivated = false;
    upFactor = 0;
    frontFactor = 0;
    rightFactor = 0;
}


void SpectatorController::init()
{
    sprintActivated = false;
    upFactor = 0;
    frontFactor = 0;
    rightFactor = 0;
}

void SpectatorController::updateDirectionStateWASD()
{
    sprintActivated = false;
    upFactor = 0;
    frontFactor = 0;
    rightFactor = 0;

    GLFWwindow *window = globals.getWindow();

    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        frontFactor ++;

    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        frontFactor --;

    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        rightFactor ++;

    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        rightFactor --;

    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        upFactor ++;

    if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        upFactor --;

    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        sprintActivated = true;
}

void SpectatorController::update()
{
    updateDirectionStateWASD();

    const vec3 cpos = globals.currentCamera->getPosition();
    const float delta = globals.appTime.getDelta();
    const float dspeed = speed * delta * (sprintActivated ? sprintFactor : 1.f);

    const vec3 front = globals.currentCamera->getDirection();
    const vec3 up = vec3(0, 1, 0);
    const vec3 right = cross(up, front);
    deplacementDir = front*(float)frontFactor + up*(float)upFactor + right*(float)rightFactor;

    float l = length(deplacementDir);
    globals.currentCamera->setPosition(cpos + dspeed*deplacementDir/max(l, 1e-9f));
}

bool SpectatorController::inputs(GLFWKeyInfo& input)
{
    // switch (input.action)
    // {
    // case GLFW_PRESS:
    //     switch (input.key)
    //     {
    //     case GLFW_KEY_W : frontFactor ++; break;
    //     case GLFW_KEY_S : frontFactor --; break;
    //     case GLFW_KEY_A : rightFactor ++; break;
    //     case GLFW_KEY_D : rightFactor --; break;
    //     case GLFW_KEY_SPACE : upFactor ++; break;
    //     case GLFW_KEY_LEFT_CONTROL : upFactor --; break;
    //     case GLFW_KEY_LEFT_SHIFT : sprintActivated = true; break;
    //     default:break;
    //     }
    //     break;

    // case GLFW_RELEASE:
    //     switch (input.key)
    //     {
    //     case GLFW_KEY_W : frontFactor --; break;
    //     case GLFW_KEY_S : frontFactor ++; break;
    //     case GLFW_KEY_A : rightFactor --; break;
    //     case GLFW_KEY_D : rightFactor ++; break;
    //     case GLFW_KEY_SPACE : upFactor --; break;
    //     case GLFW_KEY_LEFT_CONTROL : upFactor ++; break;
    //     case GLFW_KEY_LEFT_SHIFT : sprintActivated = false; break;
    //     default:break;
    //     }
    //     break;

    // default:break;
    // }


    return false;
}

void Controller::mouseEvent(vec2 dir, GLFWwindow* window)
{
    static bool lastCameraFollow = !globals.currentCamera->getMouseFollow();
    bool cameraFollow = globals.currentCamera->getMouseFollow();

    if(!lastCameraFollow && cameraFollow)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    else if(lastCameraFollow && !cameraFollow)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    lastCameraFollow = cameraFollow;

    if(globals.currentCamera->getMouseFollow())
    {
        {
            vec2 center(globals.windowWidth()*0.5, globals.windowHeight()*0.5);
            vec2 sensibility(50.0);
            dir = sensibility * (dir-center)/center;

            float yaw = radians(-dir.x);
            float pitch = radians(-dir.y);

            vec3 up = vec3(0,1,0);
            vec3 front = mat3(rotate(mat4(1), yaw, up)) * globals.currentCamera->getDirection();
            front = mat3(rotate(mat4(1), pitch, cross(front, up))) * front;
            front = normalize(front);

            front.y = clamp(front.y, -0.9f, 0.9f);
            globals.currentCamera->setDirection(front);

            glfwSetCursorPos(window, center.x, center.y);
        }
    }
}


