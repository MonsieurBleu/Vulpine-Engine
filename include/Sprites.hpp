
#include <iostream>
#include <Shader.hpp>


#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>

class SpriteBuffer
{
    const ShaderProgram &program;

    GLuint vertexBufferID;
    GLuint vertexArrayID;

    float vertexBuffer;
    

    void setProgram(const ShaderProgram &newProgram)
    {
        program = newProgram;
    };
};


class SpriteEngine
{

};