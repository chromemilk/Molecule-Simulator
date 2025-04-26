#include "Atom.h"

const glm::vec3 gravity( 0.0f, 0.0f, 0.0f );
const float boxSize = 5.0f;

Atom::Atom( const std::string &type, const glm::vec3 &pos, float mass )
    : type( type ), mass( mass ), position( pos ), velocity( glm::vec3( 0.0f ) ), radius( 0.2f ) {

    // Set color based on element type
    if (type == "O")
        color = glm::vec3( 1.0f, 0.0f, 0.0f ); // Red for Oxygen
    else if (type == "H")
        color = glm::vec3( 1.0f, 1.0f, 1.0f ); // White for Hydrogen
    else if (type == "C")
        color = glm::vec3( 0.2f, 0.2f, 0.2f ); // Dark gray for Carbon
    else if (type == "N")
        color = glm::vec3( 0.0f, 0.0f, 1.0f ); // Blue for Nitrogen
    else
        color = glm::vec3( 0.5f, 0.5f, 0.5f ); // Default: light gray
}

void Atom::update( float dt ) {
    if (fixed) return;

    velocity += gravity * dt;
    position += velocity * dt;

    // Damping (air resistance) to slow down
    velocity *= 0.98f; // 2% speed loss every frame

    // Bounce inside cube (wall collision)
    if (position.x < -boxSize)
    {
        position.x = -boxSize; velocity.x *= -0.6f;
    }
    if (position.x > boxSize)
    {
        position.x = boxSize; velocity.x *= -0.6f;
    }
    if (position.y < -boxSize)
    {
        position.y = -boxSize; velocity.y *= -0.6f;
    }
    if (position.y > boxSize)
    {
        position.y = boxSize; velocity.y *= -0.6f;
    }
    if (position.z < -boxSize)
    {
        position.z = -boxSize; velocity.z *= -0.6f;
    }
    if (position.z > boxSize)
    {
        position.z = boxSize; velocity.z *= -0.6f;
    }
}

bool Atom::isAlive() const {
    return true; 
}
