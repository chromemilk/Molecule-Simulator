#include "Particle.h"

const glm::vec3 gravity = glm::vec3( 0.0f, -9.8f, 0.0f );
const float boxSize = 5.0f;

void Particle::update( float dt ) {
    velocity += gravity * dt;
    position += velocity * dt;
    life -= dt;

    if (position.x < -boxSize)
    {
        position.x = -boxSize;
        velocity.x *= -0.6f; // bounce and lose some energy
    }
    if (position.x > boxSize)
    {
        position.x = boxSize;
        velocity.x *= -0.6f;
    }
    if (position.y < 0.0f)
    {
        position.y = 0.0f;
        velocity.y *= -0.6f;
    }
    if (position.y > boxSize)
    {
        position.y = boxSize;
        velocity.y *= -0.6f;
    }
    if (position.z < -boxSize)
    {
        position.z = -boxSize;
        velocity.z *= -0.6f;
    }
    if (position.z > boxSize)
    {
        position.z = boxSize;
        velocity.z *= -0.6f;
    }
}

bool Particle::isAlive() const {
    return life > 0.0f;
}
