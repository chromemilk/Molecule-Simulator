#include "Bond.h"
#include "Atom.h"
#include <glad/glad.h>
#include "Shader.h"  
#include "Camera.h"  // So we can use the camera
extern Camera camera;         


Bond::Bond( Atom *a, Atom *b, BondType type )
    : atomA( a ), atomB( b ), type( type ) {

    restLength = 1.0f;
    stiffness = (type == BondType::Single) ? 5.0f :
        (type == BondType::Double) ? 8.0f :
        (type == BondType::Triple) ? 10.0f : 5.0f;
}

void Bond::applyForce() {
    if (!atomA || !atomB) return;

    glm::vec3 delta = atomB->position - atomA->position;
    float currentLength = glm::length( delta );
    if (currentLength == 0.0f) return;

    glm::vec3 direction = delta / currentLength;
    float displacement = currentLength - restLength;
    glm::vec3 force = stiffness * displacement * direction;

    if (!atomA->fixed)
        atomA->velocity += force / atomA->mass;
    if (!atomB->fixed)
        atomB->velocity -= force / atomB->mass;
}
void Bond::render() const {
    if (!atomA || !atomB) return;

    static Shader *bondShader = nullptr;
    static unsigned int bondVAO = 0, bondVBO = 0;

    if (!bondShader)
    {
        bondShader = new Shader( "bond_vertex.glsl", "bond_fragment.glsl" );

        glGenVertexArrays( 1, &bondVAO );
        glGenBuffers( 1, &bondVBO );

        glBindVertexArray( bondVAO );
        glBindBuffer( GL_ARRAY_BUFFER, bondVBO );
        glBufferData( GL_ARRAY_BUFFER, sizeof( glm::vec3 ) * 2, nullptr, GL_DYNAMIC_DRAW );

        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( glm::vec3 ), (void *)0 );
        glEnableVertexAttribArray( 0 );
    }

    glm::vec3 positions[ 2 ] = { atomA->position, atomB->position };

    bondShader->use();

    // Set camera uniforms
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective( glm::radians( camera.Zoom ), 800.0f / 600.0f, 0.1f, 100.0f );

    bondShader->setMat4( "view", view );
    bondShader->setMat4( "projection", projection );

    glBindBuffer( GL_ARRAY_BUFFER, bondVBO );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof( positions ), positions );

    glBindVertexArray( bondVAO );
    glDrawArrays( GL_LINES, 0, 2 );
}

