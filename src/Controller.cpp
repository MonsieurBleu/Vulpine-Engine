#include <Controller.hpp>
#include <Globals.hpp>
#include <MathsUtils.hpp>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

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

    if(globals.currentCamera && globals.currentCamera->getMouseFollow())
    {
        vec2 center(globals.windowWidth()*0.5, globals.windowHeight()*0.5);
        vec2 sensibility(50.0);
        dir = sensibility * (dir-center)/center;

        if(globals.windowHeight() > globals.windowWidth())
            dir.x /= globals.windowHeight()/globals.windowWidth();
        else
            dir.y /= globals.windowWidth()/globals.windowHeight();

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


void OrbitController::init()
{
    globals.currentCamera->setForceLookAtPoint(true);
    globals.currentCamera->setPosition(vec3(10, 0, 0));
}

void OrbitController::clean()
{
    globals.currentCamera->setForceLookAtPoint(false);
}

void OrbitController::update()
{
    if(globals.currentCamera)
    {
        if(globals.mouseScrollOffset().y != 0 && globals.currentCamera->getMouseFollow())
        {
            // distance -= globals.mouseScrollOffset().y*0.5;
            distance *= 1.f - globals.mouseScrollOffset().y*0.1;

            distance = clamp(distance, 0.01f, 1e7f);

            
            globals.clearMouseScroll();
        }

        globals.currentCamera->setForceLookAtPoint(true);
        
        globals.currentCamera->lookAt(position);

        globals.currentCamera->setPosition(position + 
            normalize(globals.currentCamera->getPosition() - position)*distance
            );
    }
}

void OrbitController::mouseEvent(vec2 dir, GLFWwindow* window)
{
    // static bool lastCameraFollow = !globals.currentCamera->getMouseFollow();
    // bool cameraFollow = globals.currentCamera->getMouseFollow();

    // if(!lastCameraFollow && cameraFollow)
    //     glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // else if(lastCameraFollow && !cameraFollow)
    //     glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // lastCameraFollow = cameraFollow;
    vec2 tmpdir = dir;
    vec3 front;

    if(globals.currentCamera 
    && globals.currentCamera->getMouseFollow()
    )
    {
        static vec2 center(globals.windowWidth()*0.5, globals.windowHeight()*0.5);

        

        vec2 sensibility(100.0);
        dir = sensibility * (dir-center)/center;

        if(globals.windowHeight() > globals.windowWidth())
            dir.x /= globals.windowHeight()/globals.windowWidth();
        else
            dir.y /= globals.windowWidth()/globals.windowHeight();

        if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && globals.mouseMiddleClickDown())
        {
            vec3 front = normalize(globals.currentCamera->getPosition() - position);

            vec3 right = normalize(cross(front, vec3(0, 1, 0)));
            vec3 up = normalize(cross(front, right));
            
            vec3 off = -0.02f*right*dir.x + 0.02f*up*dir.y;
            position += off;


            globals.currentCamera->setPosition(globals.currentCamera->getPosition() + off);
        }
        else

        if(globals.mouseMiddleClickDown())
        {
            float yaw = radians(dir.x);
            float pitch = radians(dir.y);


            vec3 up = vec3(0,1,0);

            front = mat3(rotate(mat4(1), yaw, up)) * normalize(globals.currentCamera->getPosition() - position);
            front = mat3(rotate(mat4(1), pitch, cross(front, up))) * front;
            front = normalize(front);

            front.y = clamp(front.y, -0.9f, 0.9f);
            front = normalize(front);

            globals.currentCamera->setPosition(position + front*distance);
        }
        
        // glfwSetCursorPos(window, center.x, center.y);
        center = tmpdir;
    }

}

void DragController2D::init()
{
    position = vec2(0);
    lastPosition = vec2(0);
    delta = vec2(0);
    dragging = false;
    clicked = false;
}

void DragController2D::clean()
{
    position = vec2(0);
    lastPosition = vec2(0);
    delta = vec2(0);
    dragging = false;
    clicked = false;
}

// drag controller
// doesn't take into account the camera, only the mouse position
// drags the position variable on a 2d plane

void DragController2D::update()
{
    clicked = false;

    if (globals.mouseScrollOffset().y != 0)
    {
        scale *= 1.0 + globals.mouseScrollOffset().y * 0.1;
        globals.clearMouseScroll();
    }

    delta = vec2(0);
}

bool DragController2D::inputs(GLFWKeyInfo& input)
{
    if(input.action == GLFW_PRESS)
    {
        if(input.key == GLFW_MOUSE_BUTTON_LEFT)
        {
            mouseDown = true;
        }
    }
    else if(input.action == GLFW_RELEASE)
    {
        if(input.key == GLFW_MOUSE_BUTTON_LEFT)
        {
            if (!dragging)
                clicked = true;
            dragging = false;
            mouseDown = false;

            delta = vec2(0);
            lastPosition = vec2(0);
        }
    }

    return false;
}

void DragController2D::mouseEvent(vec2 pos, GLFWwindow* window)
{
    if(mouseDown)
    {
        dragging = true;
        
        vec2 center(globals.windowWidth()*0.5, globals.windowHeight()*0.5);
        vec2 sensitivity(1.5);
        // adjust sensitivity based on the scale parameter
        // sensitivity /= scale;

        vec2 posNDC = (pos-center)/center;

        posNDC *= sensitivity;

        if (lastPosition == vec2(0))
            lastPosition = posNDC;

        delta = posNDC - lastPosition;

        position += delta;
        lastPosition = posNDC;
    }
}