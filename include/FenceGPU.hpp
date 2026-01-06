#pragma once 

#include <string_view>
#include <unordered_map>

#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>

class FenceGPU
{
    private : 
        GLsync fence;
        GLenum r = 0;

    public : 

        FenceGPU();
        static inline std::unordered_map<std::string_view,  FenceGPU> list;
        static void printState();

        std::string toString();
};
