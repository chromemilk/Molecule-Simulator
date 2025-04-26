#pragma once
#include <string>
#include <glm/glm.hpp>

class Shader
{
public:
    unsigned int ID;

    Shader( const char *vertexPath, const char *fragmentPath );

    void use() const;
    void setVec3( const std::string &name, const glm::vec3 &value ) const;
    void setFloat( const std::string &name, float value ) const;
};
