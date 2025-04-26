#include "Shader.h"
#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader( const char *vertexPath, const char *fragmentPath ) {
    // Read shader source
    std::ifstream vShaderFile( vertexPath );
    std::ifstream fShaderFile( fragmentPath );
    std::stringstream vShaderStream, fShaderStream;

    vShaderStream << vShaderFile.rdbuf();
    fShaderStream << fShaderFile.rdbuf();

    std::string vertexCode = vShaderStream.str();
    std::string fragmentCode = fShaderStream.str();

    const char *vShaderCode = vertexCode.c_str();
    const char *fShaderCode = fragmentCode.c_str();

    // Compile shaders
    unsigned int vertex, fragment;

    vertex = glCreateShader( GL_VERTEX_SHADER );
    glShaderSource( vertex, 1, &vShaderCode, NULL );
    glCompileShader( vertex );

    fragment = glCreateShader( GL_FRAGMENT_SHADER );
    glShaderSource( fragment, 1, &fShaderCode, NULL );
    glCompileShader( fragment );

    // Create program
    ID = glCreateProgram();
    glAttachShader( ID, vertex );
    glAttachShader( ID, fragment );
    glLinkProgram( ID );

    // Delete shaders after linking
    glDeleteShader( vertex );
    glDeleteShader( fragment );
}

void Shader::use() const {
    glUseProgram( ID );
}

void Shader::setVec3( const std::string &name, const glm::vec3 &value ) const {
    glUniform3fv( glGetUniformLocation( ID, name.c_str() ), 1, &value[ 0 ] );
}

void Shader::setFloat( const std::string &name, float value ) const {
    glUniform1f( glGetUniformLocation( ID, name.c_str() ), value );
}
