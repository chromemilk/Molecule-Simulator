#include "Renderer.h"
#include "Shader.h"
#include "Camera.h"
#include <glad/glad.h>

// Static variables
static unsigned int VAO, VBO;
static Shader *particleShader = nullptr;
static Camera *cameraPtr = nullptr;

void Renderer::Init() {
    // Load shader
    particleShader = new Shader( "particle_vertex.glsl", "particle_fragment.glsl" );

    // Setup VAO and VBO
    glGenVertexArrays( 1, &VAO );
    glGenBuffers( 1, &VBO );

    glBindVertexArray( VAO );
    glBindBuffer( GL_ARRAY_BUFFER, VBO );

    glBufferData( GL_ARRAY_BUFFER, sizeof( glm::vec3 ), nullptr, GL_DYNAMIC_DRAW );

    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( glm::vec3 ), (void *)0 );
    glEnableVertexAttribArray( 0 );

    glBindVertexArray( 0 ); // Unbind for now
}

void Renderer::Shutdown() {
    glDeleteVertexArrays( 1, &VAO );
    glDeleteBuffers( 1, &VBO );
    delete particleShader;
}

void Renderer::DrawParticle( const glm::vec3 &position, const glm::vec3 &color, float size ) {
    if (!cameraPtr) return; // Safety check

    particleShader->use();
    particleShader->setVec3( "particleColor", color );
    particleShader->setFloat( "pointSize", size );

    // Set camera matrices
    glm::mat4 view = cameraPtr->GetViewMatrix();
    glm::mat4 projection = glm::perspective( glm::radians( cameraPtr->Zoom ), 800.0f / 600.0f, 0.1f, 100.0f );

    glUniformMatrix4fv( glGetUniformLocation( particleShader->ID, "view" ), 1, GL_FALSE, &view[ 0 ][ 0 ] );
    glUniformMatrix4fv( glGetUniformLocation( particleShader->ID, "projection" ), 1, GL_FALSE, &projection[ 0 ][ 0 ] );

    // Upload particle position
    glBindBuffer( GL_ARRAY_BUFFER, VBO );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof( glm::vec3 ), &position );

    glBindVertexArray( VAO );
    glDrawArrays( GL_POINTS, 0, 1 );
}

void Renderer::SetCamera( Camera *cam ) {
    cameraPtr = cam;
}
