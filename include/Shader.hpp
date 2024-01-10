#ifndef SHADER_HPP
#define SHADER_HPP

#include <string>

#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>

#include <Uniforms.hpp>

#define NO_PROGRAM 0xFFFFFF

enum ShaderError
{
    ShaderOk,
    ShaderNoFile,
    ShaderCompileError,
    ShaderLinkingError
};

class Shader
{
    private :
        GLuint type;
        GLuint shader = NO_PROGRAM;
        std::string Path;

    public :
        Shader(){};

        // Prepare the shader for loading from the file at the given path
        //    .vert for vertex shaders
        //    .geom for geometry shaders
        //    .frag for fragment shaders
        // You then can load the shader using the refresh() method
        // This method is only needed to be called once
        void prepareLoading(const std::string& Path);
        ShaderError refresh();

        GLuint get_shader(){return shader;};
        const std::string &get_Path(){return Path;};
};

#define MAX_SHADER_HANDLE 0x1000

class ShaderProgram
{
    private :

        GLuint program = NO_PROGRAM;

        Shader vert;
        Shader frag;
        Shader geom;

        static uint16 useCount[MAX_SHADER_HANDLE];


    public :
        ShaderUniformGroup uniforms;


        ShaderProgram& operator=(const ShaderProgram& other);

        ShaderProgram(){};
        ShaderProgram(const ShaderProgram& other);
        ~ShaderProgram();

        ShaderProgram(const std::string _fragPath, 
                      const std::string _vertPath = "", 
                      const std::string _geomPath = "",
                      std::vector<ShaderUniform> uniforms = {});

        ShaderError compileAndLink();
        ShaderError reset();

        void activate() const;
        void deactivate() const;

        GLuint getProgram() const {return program;};

        ShaderProgram& addUniform(ShaderUniform newUniform);
};

#endif