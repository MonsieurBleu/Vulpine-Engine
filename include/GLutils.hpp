#ifndef GLUTILS_HPP
#define GLUTILS_HPP

#include <iostream>

#include <string.h>

#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>

#include <Utils.hpp>

#define SHOW_GL_NOTIF

/// OPEN GL
struct BlueGLenum
{
    GLenum val;
    BlueGLenum(GLenum val) : val(val) {};
};

std::ostream& operator<<(std::ostream& os, BlueGLenum e)
{
    switch (e.val)
    {
    case GL_DEBUG_SEVERITY_HIGH : os << "HIGHT"; break;

    case GL_DEBUG_SEVERITY_MEDIUM : os << "MEDIUM"; break;

    case GL_DEBUG_SEVERITY_LOW : os << "LOW"; break;

    case GL_DEBUG_SEVERITY_NOTIFICATION : os << "NOTIFICATION"; break;
    
    case GL_DEBUG_TYPE_ERROR : os << "ERROR"; break;

    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR : os << "DEPRECATED BEHAVIOR"; break;

    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR : os << "UNDEFINED BEHAVIOR"; break;

    case GL_DEBUG_TYPE_PORTABILITY : os << "UNPORTABLE FUNCTIONALITY"; break;

    case GL_DEBUG_TYPE_PERFORMANCE : os << "POSSIBLE PERFORMANCE ISSUE"; break;

    case GL_DEBUG_TYPE_MARKER : os << "COMMAND STREAM ANNOTATION"; break;

    case GL_DEBUG_TYPE_PUSH_GROUP : os << "GROUP PUSHING"; break;

    case GL_DEBUG_TYPE_POP_GROUP : os << "GROUP POPING"; break;

    case GL_DEBUG_TYPE_OTHER : os << "OTHER TYPE"; break;

    case GL_DEBUG_SOURCE_API : os << "OPENGL API"; break;

    case GL_DEBUG_SOURCE_WINDOW_SYSTEM : os << "WINDOW-SYSTEM API"; break;

    case GL_DEBUG_SOURCE_SHADER_COMPILER : os << "SHADER COMPILER"; break;

    case GL_DEBUG_SOURCE_THIRD_PARTY : os << "OPENGL ASSOCIATED APPLICATION"; break;

    case GL_DEBUG_SOURCE_APPLICATION : os << "USER"; break;

    case GL_DEBUG_SOURCE_OTHER : os << "OTHER SOURCE"; break;

    default: os << "UNKNOWN"; break;
    }

    return os;
}

void GLAPIENTRY MessageCallback(GLenum _source,
                                GLenum _type,
                                GLuint id,
                                GLenum _severity,
                                GLsizei length,
                                const GLchar* message,
                                const void* userParam )
{
    return;
    if(id == 1281) // Invalid shader programm, spam if a shader fail to compile
        return;

    const std::string *color = &TERMINAL_RESET;
    BlueGLenum severity(_severity);
    BlueGLenum type(_type);
    BlueGLenum source(_source);

    if(type.val == GL_DEBUG_TYPE_ERROR)
    {
        color = &TERMINAL_ERROR;
    }
    else if(severity.val == GL_DEBUG_SEVERITY_NOTIFICATION)
    {
    #ifndef SHOW_GL_NOTIF
        color = &TERMINAL_NOTIF;
    #else
        return;
    #endif
    }
    else
    {
        color = &TERMINAL_WARNING;
    }

    std::cerr
    << TERMINAL_NOTIF << "\nGL CALLBACK "       << *color << "[" << type << "]"
    << TERMINAL_RESET << "\n\tid       = "      << *color << id
    << TERMINAL_RESET << "\n\tfrom     = "      << *color << source
    << TERMINAL_RESET << "\n\tseverity = "      << *color << severity
    << TERMINAL_RESET << "\n\tmessage  = "      << *color << message
    << "\n\n" << TERMINAL_RESET;
};


#endif