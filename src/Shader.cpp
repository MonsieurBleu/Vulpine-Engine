#include <Graphics/Shader.hpp>
#include <Utils.hpp>
#include <Timer.hpp>
#include <iostream>
#include <Graphics/Shadinclude.hpp>

std::string Shadinclude::shaderDefines;

// uint16 ShaderProgram::useCount[MAX_SHADER_HANDLE] = {(uint16)0};

void Shader::prepareLoading(const std::string &path, const std::string &defines)
{
    this->defines = defines;

    startbenchrono();

    Path = path;

    std::string extension = getFileExtension(Path);

    if (extension == "frag")
        type = GL_FRAGMENT_SHADER;
    else if (extension == "vert")
        type = GL_VERTEX_SHADER;
    else if (extension == "geom")
        type = GL_GEOMETRY_SHADER;
    else if (extension == "tese")
        type = GL_TESS_EVALUATION_SHADER;
    else if (extension == "tesc")
        type = GL_TESS_CONTROL_SHADER;
}

ShaderError Shader::refresh()
{
    shader = glCreateShader(type);

    // std::string code = readFile(Path);
    std::string code = Shadinclude::load(Path, "#include ", defines);

    if (code.empty())
        return ShaderNoFile;

    const char *glcode = code.c_str();
    glShaderSource(shader, 1, &glcode, NULL);
    glCompileShader(shader);

    // Checking shader

    GLint result = GL_FALSE;
    int logLength;

    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

    if (logLength > 0)
    {
        GLchar ShaderError[logLength];
        glGetShaderInfoLog(shader, logLength, NULL, ShaderError);
        std::cerr
            << TERMINAL_ERROR
            << "Error compiling shader "
            << TERMINAL_FILENAME
            << TERMINAL_UNDERLINE
            << Path
            << TERMINAL_RESET;

        // std::cerr << ShaderError << std::endl
        //           << TERMINAL_RESET;
        
        int cnt = 0;
        // while(true)
        for(; cnt < logLength; cnt++)
        {
            int lineNB = atoi(ShaderError + cnt + 2) - 1;

            int lineCNT = 0;
            int j = 0;
            int codeSize = code.size();
            for(; j < codeSize; j++)
                if((lineCNT += (code[j] == '\n')) == lineNB)
                    break;

            std::cerr << TERMINAL_ERROR << "\n";

            for(; cnt < logLength; cnt++)
            {
                if(ShaderError[cnt] == '\n')
                    break;

                std::cerr << ShaderError[cnt];
            }

            std::cerr << TERMINAL_INFO << "\n";

            for(j++; j < codeSize; j++)
            {
                if(code[j] == '\n')
                    break;

                std::cerr << code[j];
            }
            
            // std::cerr << "\n" << TERMINAL_RESET;
        }
        std::cerr << "\n" << TERMINAL_RESET;

        return ShaderCompileError;
    }

    // std::cout << "==================================================\n\n";
    // std::cout << Path << "\n";
    // std::cout << glcode << "\n";
    // std::cout << "==================================================\n\n";


    return ShaderOk;
}

ShaderProgram::ShaderProgram(const std::string _fragPath,
                             const std::string _vertPath,
                             std::vector<ShaderUniform> uniforms,
                             const std::string &defines)
    : uniforms(uniforms)
{
    frag.prepareLoading(_fragPath, defines);

    vert.prepareLoading(_vertPath, defines);

    compileAndLink();
}

ShaderProgram::ShaderProgram(const std::string _fragPath,
                             const std::string _vertPath,
                             const std::string _geomPath,
                             std::vector<ShaderUniform> uniforms,
                             const std::string &defines)
    : uniforms(uniforms)
{
    frag.prepareLoading(_fragPath, defines);
    
    vert.prepareLoading(_vertPath, defines);

    if (!_geomPath.empty())
        geom.prepareLoading(_geomPath, defines);

    compileAndLink();
}

ShaderProgram::ShaderProgram(const std::string _fragPath,
                             const std::string _vertPath, 
                             const std::string _tescPath,
                             const std::string _tesePath,
                             std::vector<ShaderUniform> uniforms,
                             const std::string &defines)
    : uniforms(uniforms)
{
    frag.prepareLoading(_fragPath, defines);

    if (!_vertPath.empty())
        vert.prepareLoading(_vertPath, defines);

    if (!_tescPath.empty())
        tesc.prepareLoading(_tescPath, defines);

    if (!_tesePath.empty())
        tese.prepareLoading(_tesePath, defines);

    compileAndLink();
}


ShaderError ShaderProgram::compileAndLink()
{
    BenchTimer timer;
    timer.start();

    //// COMPILING SHADERS
    ShaderError serrf = frag.refresh();
    ShaderError serrv = ShaderOk;
    ShaderError serrg = ShaderOk;
    ShaderError serrtc = ShaderOk;
    ShaderError serrte = ShaderOk;


    if (!vert.get_Path().empty())
        serrv = vert.refresh();

    if (!geom.get_Path().empty())
        serrg = geom.refresh();

    if (!tesc.get_Path().empty())
        serrv = tesc.refresh();

    if (!tese.get_Path().empty())
        serrg = tese.refresh();

    if (serrf != ShaderOk || serrv != ShaderOk || serrg != ShaderOk || serrtc != ShaderOk || serrte != ShaderOk)
        return ShaderCompileError;

    ///// CREATING PROGRAM AND LINKING EVERYTHING
    program = glCreateProgram();

    glAttachShader(program, frag.get_shader());

    if (!vert.get_Path().empty())
        glAttachShader(program, vert.get_shader());

    if (!geom.get_Path().empty())
        glAttachShader(program, geom.get_shader());

    if (!tesc.get_Path().empty())
        glAttachShader(program, tesc.get_shader());

    if (!tese.get_Path().empty())
        glAttachShader(program, tese.get_shader());


    glLinkProgram(program);

    GLint result = GL_FALSE;
    int logLength;
    glGetProgramiv(program, GL_LINK_STATUS, &result);
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

    if (logLength > 0 && result != GL_TRUE)
    {
        char programError[logLength];
        glGetProgramInfoLog(program, logLength, NULL, programError);
        std::cerr << TERMINAL_ERROR << "Error compiling shaders " << frag.get_Path() << " " << vert.get_Path() << " " << geom.get_Path() << " :\n";
        std::cerr << programError << std::endl
                  << TERMINAL_RESET;

        if (result != GL_TRUE)
            return ShaderLinkingError;
    }

    glDeleteShader(frag.get_shader());
    if (!vert.get_Path().empty())
        glDeleteShader(vert.get_shader());
    if (!geom.get_Path().empty())
        glDeleteShader(geom.get_shader());

    // useCount[program]++;
    handle = std::make_shared<GLuint>(program);

    timer.end();

    #ifdef SHOW_SHADER_PROGRAM_LOADING
    std::cout
        << TERMINAL_OK
        << "Shader Program (id " << program << ") "
        << TERMINAL_FILENAME
        << frag.get_Path()
        << (vert.get_Path().empty() ? "" : " " + vert.get_Path())
        << (geom.get_Path().empty() ? "" : " " + geom.get_Path())
        << TERMINAL_OK
        << " linked successfully in "
        << TERMINAL_TIMER << timer.getElapsedTime() * 1000.f << " ms\n"
        << TERMINAL_RESET;
    #endif

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
    glUseProgram(0);
}

ShaderProgram &ShaderProgram::addUniform(ShaderUniform newUniform)
{
    uniforms.add(newUniform);
    return *this;
}

ShaderProgram::~ShaderProgram()
{
    if (program != NO_PROGRAM && handle.use_count() == 1)
    {
        glDeleteProgram(program);
    }
}
