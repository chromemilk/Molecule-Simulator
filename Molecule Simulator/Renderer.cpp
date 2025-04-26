#include "Renderer.h"
#include <glad/glad.h>
#include <vector>

static unsigned int VAO, VBO;

void Renderer::Init() {
    glGenVertexArrays( 1, &VAO );
    glGenBuffers( 1, &VBO );

    glBindVertexArray( VAO );
    glBindBuffer( GL_ARRAY_BUFFER, VBO );

    // Allocate space for 1 particle
    glBufferData( GL_ARRAY_BUFFER, sizeof( glm::vec3 ), nullptr, GL_DYNAMIC_DRAW );

    // Describe layout
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( glm::vec3 ), (void *)0 );
    glEnableVertexAttribArray( 0 );
}

void Renderer::Shutdown() {
    glDeleteVertexArrays( 1, &VAO );
    glDeleteBuffers( 1, &VBO );
}

void Renderer::DrawParticle( const glm::vec3 &position, const glm::vec3 &color, float size ) {
    glBindBuffer( GL_ARRAY_BUFFER, VBO );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof( glm::vec3 ), &position );

    glBindVertexArray( VAO );

    glPointSize( size );

    // Set color manually (simple fixed shader for now)
    glEnable( GL_PROGRAM_POINT_SIZE );
    glEnable( GL_VERTEX_PROGRAM_POINT_SIZE );

    glDrawArrays( GL_POINTS, 0, 1 );
}
