#include "Atom.h"

const glm::vec3 gravity( 0.0f, 0.0f, 0.0f );   // No gravity anymore :)
const float boxSize = 5.0f;
const float FLOOR_Y = -0.2f;                  

Atom::Atom( const std::string &type, const glm::vec3 &pos, float mass )
    : type( type ), mass( mass ), position( pos ), velocity( glm::vec3( 0.0f ) ), radius( 0.2f ) {
    if (type == "O")      color = glm::vec3( 1.0f, 0.0f, 0.0f );
    else if (type == "H") color = glm::vec3( 1.0f, 1.0f, 1.0f );
    else if (type == "C") color = glm::vec3( 0.2f, 0.2f, 0.2f );
    else if (type == "N") color = glm::vec3( 0.0f, 0.0f, 1.0f );
    else if (type == "S") color = glm::vec3( 1.0f, 1.0f, 0.18f );
    else if (type == "P") color = glm::vec3( 1.0f, 0.5f, 0.0f );
    else if (type == "F") color = glm::vec3( 0.0f, 0.9f, 0.0f );
    else if (type == "Cl") color = glm::vec3( 0.0f, 0.8f, 0.0f );
    else if (type == "Br") color = glm::vec3( 0.65f, 0.16f, 0.16f );
    else if (type == "I") color = glm::vec3( 0.58f, 0.0f, 0.83f );
    else if (type == "He") color = glm::vec3( 0.85f, 1.0f, 1.0f );
    else if (type == "Ne") color = glm::vec3( 0.7f, 0.89f, 0.96f );
    else if (type == "Ar") color = glm::vec3( 0.5f, 0.82f, 0.89f );
    else if (type == "Li") color = glm::vec3( 0.8f, 0.5f, 1.0f );
    else if (type == "Na") color = glm::vec3( 0.0f, 0.0f, 0.8f );
    else if (type == "K")  color = glm::vec3( 0.56f, 0.25f, 0.83f );
    else if (type == "Mg") color = glm::vec3( 0.13f, 0.54f, 0.13f );
    else if (type == "Ca") color = glm::vec3( 0.24f, 1.0f, 0.0f );
    else if (type == "Fe") color = glm::vec3( 0.88f, 0.4f, 0.2f );
    else if (type == "Cu") color = glm::vec3( 0.78f, 0.5f, 0.2f );
    else if (type == "Zn") color = glm::vec3( 0.49f, 0.5f, 0.69f );
    else if (type == "B")  color = glm::vec3( 1.0f, 0.67f, 0.47f );
    else if (type == "Si") color = glm::vec3( 0.94f, 0.78f, 0.62f );
    else if (type == "Al") color = glm::vec3( 0.75f, 0.65f, 0.65f );
    else                  color = glm::vec3( 0.5f, 0.5f, 0.5f );  // fallback
}

void Atom::update( float dt ) {
    if (fixed) return;

    velocity += gravity * dt;
    position += velocity * dt;

    // Damping (air resistance) to slow down over time
    velocity *= 0.98f;

    // Wall collision 
    if (position.x < -boxSize)
    {
        position.x = -boxSize; velocity.x *= -0.6f;
    }
    if (position.x > boxSize)
    {
        position.x = boxSize;  velocity.x *= -0.6f;
    }

    if (position.z < -boxSize)
    {
        position.z = -boxSize; velocity.z *= -0.6f;
    }
    if (position.z > boxSize)
    {
        position.z = boxSize;  velocity.z *= -0.6f;
    }

    // Floor collision 
    if (position.y - radius < FLOOR_Y)
    {
        position.y = FLOOR_Y + radius;
        if (velocity.y < 0.0f) velocity.y *= -0.6f;
    }

    if (position.y > boxSize)
    {
        position.y = boxSize; velocity.y *= -0.6f;
    }
}

bool Atom::isAlive() const {
    return true;
}
