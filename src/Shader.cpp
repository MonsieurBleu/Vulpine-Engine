#include <Shader.hpp>
#include <Utils.hpp>

#include <iostream>
#include "Shadinclude.hpp"


void Shader::prepareLoading(const std::string& path)
{
    startbenchrono();

    Path = path;

    std::string extension = getFileExtension(Path);

    if(extension == "frag")
        type = GL_FRAGMENT_SHADER;
    else if(extension == "vert")
        type = GL_VERTEX_SHADER;
    else if(extension == "geom")
        type = GL_GEOMETRY_SHADER;
}

ShaderError Shader::refresh()
{
    shader = glCreateShader(type);

    // std::string code = readFile(Path);
    std::string code = Shadinclude::load(Path, "#include");

    if(code.empty())
        return ShaderNoFile;
    
    const char *glcode = code.c_str();
    glShaderSource(shader, 1, &glcode, NULL);
    glCompileShader(shader);

    // Checking shader

    GLint result = GL_FALSE;
    int logLength;

    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

    if(logLength > 0)
    {
        GLchar ShaderError[logLength];
        glGetShaderInfoLog(shader, logLength, NULL, ShaderError);
        std::cerr 
        << TERMINAL_ERROR 
        << "Error compiling shader " 
        << TERMINAL_FILENAME 
        << Path 
        << TERMINAL_ERROR 
        << " :\n";
        std::cerr << ShaderError << std::endl << TERMINAL_RESET;

        return ShaderCompileError;
    }

    std::cout 
    << TERMINAL_OK 
    << "Shader " 
    << TERMINAL_FILENAME
    << Path 
    << TERMINAL_OK 
    << " loaded and compiled successfully";
    endbenchrono();

    return ShaderOk;
}


ShaderProgram::ShaderProgram(const std::string _fragPath, 
                             const std::string _vertPath, 
                             const std::string _geomPath,
                             std::vector<ShaderUniform> uniforms)
                             : uniforms(uniforms)
{
    frag.prepareLoading(_fragPath);

    if(!_vertPath.empty())
        vert.prepareLoading(_vertPath);

    if(!_geomPath.empty())
        geom.prepareLoading(_geomPath);

    compileAndLink();
}

ShaderError ShaderProgram::compileAndLink()
{
    //// COMPILING SHADERS
    ShaderError serrf = frag.refresh();
    ShaderError serrv = ShaderOk;
    ShaderError serrg = ShaderOk;


    if(!vert.get_Path().empty())
        serrv = vert.refresh();

    if(!geom.get_Path().empty())
        serrg = geom.refresh();

    if(serrf != ShaderOk || serrv != ShaderOk || serrg != ShaderOk)
        return ShaderCompileError;

    ///// CREATING PROGRAM AND LINKING EVERYTHING
    startbenchrono();
    program = glCreateProgram();    

    glAttachShader(program, frag.get_shader());

    if(!vert.get_Path().empty())
        glAttachShader(program, vert.get_shader());
    
    if(!geom.get_Path().empty())
        glAttachShader(program, geom.get_shader());
    
    glLinkProgram(program);

    GLint result = GL_FALSE;
    int logLength;
    glGetProgramiv(program, GL_LINK_STATUS, &result);
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

    if(logLength > 0 && result != GL_TRUE)
    // if(logLength > 0)
    {
        char programError[logLength];
        glGetProgramInfoLog(program, logLength, NULL, programError);
        std::cerr << TERMINAL_ERROR << "Error compiling shaders " << frag.get_Path() << " " << vert.get_Path() << " " << geom.get_Path() << " :\n";
        std::cerr << programError << std::endl << TERMINAL_RESET;

        if(result != GL_TRUE)
            return ShaderLinkingError;
    }

    std::cout 
    << TERMINAL_OK 
    << "Shader Program (id " << program << ") "
    << TERMINAL_FILENAME
    << frag.get_Path() 
    << (vert.get_Path().empty() ? "" : " "+vert.get_Path())
    << (geom.get_Path().empty() ? "" : " "+geom.get_Path())
    << TERMINAL_OK 
    << " linked successfully";
    endbenchrono();

    glDeleteShader(frag.get_shader());
    if(!vert.get_Path().empty()) glDeleteShader(vert.get_shader());
    if(!geom.get_Path().empty()) glDeleteShader(geom.get_shader());

    return ShaderOk;
}

ShaderError ShaderProgram::reset()
{
    glDeleteProgram(program);
    return compileAndLink();
}

void ShaderProgram::activate() const
{
    // std::cout << "activating program " << program << "\n";

    glUseProgram(program);
    uniforms.update();
}

void ShaderProgram::deactivate() const
{
    glUseProgram(program);
}

ShaderProgram& ShaderProgram::addUniform(ShaderUniform newUniform)
{
    uniforms.add(newUniform);
    return *this;
}