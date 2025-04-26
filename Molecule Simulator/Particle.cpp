#include "Particle.h"

const glm::vec3 gravity = glm::vec3( 0.0f, -9.8f, 0.0f ); // gravity vector (y-axis down)

void Particle::update( float dt ) {
    velocity += gravity * dt;   // Gravity affects velocity
    position += velocity * dt;  
    life -= dt;                 
}

bool Particle::isAlive() const {
    return life > 0.0f;
}
