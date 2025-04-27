#pragma once
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

class Mesh
{
public:
    Mesh() : VAO( 0 ), VBO( 0 ), vertexCount( 0 ) {
    }

    // forbid copy (would double-delete VAO/VBO)
    Mesh( const Mesh & ) = delete;
    Mesh &operator=( const Mesh & ) = delete;
    // allow move
    Mesh( Mesh && )            noexcept = default;
    Mesh &operator=( Mesh && ) noexcept = default;

    ~Mesh();

    void Build( const std::vector<glm::vec3> &vertices );   // create GL buffers
    void Draw() const;                                   // glDrawArrays

private:
    GLuint   VAO{}, VBO{};
    GLsizei  vertexCount{};
};
