#include "Mesh.h"

void Mesh::Build( const std::vector<glm::vec3> &vertices ) {
    vertexCount = static_cast<GLsizei>(vertices.size());

    glGenVertexArrays( 1, &VAO );
    glGenBuffers( 1, &VBO );

    glBindVertexArray( VAO );
    glBindBuffer( GL_ARRAY_BUFFER, VBO );
    glBufferData( GL_ARRAY_BUFFER,
        vertices.size() * sizeof( glm::vec3 ),
        vertices.data(),
        GL_STATIC_DRAW );

    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( glm::vec3 ), (void *)0 );
    glEnableVertexAttribArray( 0 );

    glBindVertexArray( 0 );
}

Mesh::~Mesh() {
    if (VBO) glDeleteBuffers( 1, &VBO );
    if (VAO) glDeleteVertexArrays( 1, &VAO );
}

void Mesh::Draw() const {
    glBindVertexArray( VAO );
    glDrawArrays( GL_TRIANGLES, 0, vertexCount );
    glBindVertexArray( 0 );
}
