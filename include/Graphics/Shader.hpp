#ifndef SHADER_HPP
#define SHADER_HPP

#include <string>

#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>

#include <Uniforms.hpp>

#include <Filewatcher.hpp>

#include <unordered_map>

#define NO_PROGRAM 0xFFFFFF

// #define SHOW_SHADER_PROGRAM_LOADING

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
        std::string defines;

    public :
        Shader(){};

        static inline std::unordered_map<std::string, std::pair<Filewatcher, bool>> fileWatchers;

        // Prepare the shader for loading from the file at the given path
        //    .vert for vertex shaders
        //    .geom for geometry shaders
        //    .frag for fragment shaders
        // You then can load the shader using the refresh() method
        // This method is only needed to be called once
        void prepareLoading(const std::string& Path, const std::string &defines = "");
        ShaderError refresh(std::vector<std::string> &dependencies);

        GLuint get_shader(){return shader;};
        const std::string &get_Path(){return Path;};

};

#define MAX_SHADER_HANDLE 0x1000

class ShaderProgram
{
    private :

        GLuint program = NO_PROGRAM;
        std::shared_ptr<GLuint> handle;

        
    public :
        std::vector<std::string> dependencies;
        
        Shader vert;
        Shader frag;
        Shader geom;
        Shader tesc;
        Shader tese;

        ShaderUniformGroup uniforms;

        ShaderProgram(){};
        ~ShaderProgram();

        ShaderProgram(const std::string _fragPath, 
                      const std::string _vertPath, 
                      std::vector<ShaderUniform> uniforms = {},
                      const std::string &defines = "");

        ShaderProgram(const std::string _fragPath, 
                      const std::string _vertPath, 
                      const std::string _geomPath,
                      std::vector<ShaderUniform> uniforms = {},
                      const std::string &defines = "");

        ShaderProgram(const std::string _fragPath, 
                      const std::string _vertPath, 
                      const std::string _tescPath,
                      const std::string _tesePath,
                      std::vector<ShaderUniform> uniforms = {},
                      const std::string &defines = "");

        ShaderError compileAndLink();
        ShaderError reset(bool hotReload = true);

        void activate() const;
        void deactivate() const;

        GLuint getProgram() const {return program;};

        ShaderProgram& addUniform(ShaderUniform newUniform);

        bool needRefresh();
};

#endif