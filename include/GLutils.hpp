#ifndef GLUTILS_HPP
#define GLUTILS_HPP

#include <iostream>
#include <deque>
#include <string.h>
#include <algorithm>

#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <Utils.hpp>

// #define SHOW_GL_NOTIF
#define PREVENT_GL_NOTIF_SPAM

struct GLenum_t
{
    GLenum val;
    GLenum_t(GLenum val) : val(val) {};
};

struct errorHistoric
{
    GLuint id;
    long time;
};

std::ostream& operator<<(std::ostream& os, GLenum_t e)
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

void printGLerror(
                    GLenum _source, 
                    GLenum _type,
                    GLuint id,
                    GLenum _severity,
                    GLsizei length,
                    const GLchar* message)
{
    GLenum_t severity(_severity);
    GLenum_t type(_type);
    GLenum_t source(_source);

    const std::string *color = &TERMINAL_RESET;

    if(type.val == GL_DEBUG_TYPE_ERROR)
    {
        color = &TERMINAL_ERROR;
    }
    else if(severity.val == GL_DEBUG_SEVERITY_NOTIFICATION)
    {
        color = &TERMINAL_NOTIF;
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
}

void GLAPIENTRY MessageCallback(GLenum _source,
                                GLenum _type,
                                GLuint id,
                                GLenum _severity,
                                GLsizei length,
                                const GLchar* message,
                                const void* userParam )
{
    /*
        Historic that restrain message callback spam at each frame
        Some non duplicated error could be missed.
    */
#ifdef PREVENT_GL_NOTIF_SPAM
    static std::deque<errorHistoric> historic;
    static const long spamTimeout = 1E4;

    long now = GetTimeMs();

    for(auto i = historic.begin(); i != historic.end(); i++)
        if(i->id == id && (now-i->time) < spamTimeout)
            return;

    historic.push_front((errorHistoric){id, now});
#endif

    #ifndef SHOW_GL_NOTIF
    if(_severity == GL_DEBUG_SEVERITY_NOTIFICATION)
        return;
    #endif

    printGLerror(_source, _type, id, _severity, length, message);
};



#endif