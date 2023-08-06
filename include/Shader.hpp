#ifndef SHADER_HPP
#define SHADER_HPP

#include <string>

#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>

#include <Uniforms.hpp>



enum ShaderError
{
    ShaderOk,
    ShaderNoFile,
    ShaderCompileError,
    ShaderLinkingError
};

GLuint LoadShader(const char *vertex_path, const char *fragment_path);

class Shader
{
    private :
        GLuint type;
        GLuint shader;
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

class ShaderProgram
{
    private :

        GLuint program;

        Shader vert;
        Shader frag;
        Shader geom;

    public :

        ShaderUniformGroup uniforms;

        ShaderProgram() {};

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