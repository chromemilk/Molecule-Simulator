#include "ShaderLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <glad/glad.h>

unsigned int LoadShader( const std::string &vertexPath, const std::string &fragmentPath ) {
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    vShaderFile.open( vertexPath );
    fShaderFile.open( fragmentPath );
    std::stringstream vShaderStream, fShaderStream;
    vShaderStream << vShaderFile.rdbuf();
    fShaderStream << fShaderFile.rdbuf();
    vertexCode = vShaderStream.str();
    fragmentCode = fShaderStream.str();

    const char *vShaderCode = vertexCode.c_str();
    const char *fShaderCode = fragmentCode.c_str();

    unsigned int vertex, fragment;
    int success;
    char infoLog[ 512 ];

    // Vertex Shader
    vertex = glCreateShader( GL_VERTEX_SHADER );
    glShaderSource( vertex, 1, &vShaderCode, NULL );
    glCompileShader( vertex );

    glGetShaderiv( vertex, GL_COMPILE_STATUS, &success );
    if (!success)
    {
        glGetShaderInfoLog( vertex, 512, NULL, infoLog );
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Fragment Shader
    fragment = glCreateShader( GL_FRAGMENT_SHADER );
    glShaderSource( fragment, 1, &fShaderCode, NULL );
    glCompileShader( fragment );

    glGetShaderiv( fragment, GL_COMPILE_STATUS, &success );
    if (!success)
    {
        glGetShaderInfoLog( fragment, 512, NULL, infoLog );
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Shader Program
    unsigned int ID = glCreateProgram();
    glAttachShader( ID, vertex );
    glAttachShader( ID, fragment );
    glLinkProgram( ID );

    glGetProgramiv( ID, GL_LINK_STATUS, &success );
    if (!success)
    {
        glGetProgramInfoLog( ID, 512, NULL, infoLog );
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader( vertex );
    glDeleteShader( fragment );

    return ID;
}


unsigned int LoadComputeShader(const std::string& path)
{
  /*  std::ifstream file(path);  std::stringstream ss;  ss << file.rdbuf();
    std::string code = ss.str();
    const char* src = code.c_str();

    GLuint sh = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(sh, 1, &src, nullptr);
    glCompileShader(sh);

    GLuint prog = glCreateProgram();
    glAttachShader(prog, sh);
    glLinkProgram(prog);
    glDeleteShader(sh);
    return prog;
    */
}
