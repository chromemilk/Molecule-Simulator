#pragma once
#include <glm/glm.hpp>

class Particle
{
public:
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 color;
    float size;
    float life;

    Particle( const glm::vec3 &pos, const glm::vec3 &vel, const glm::vec3 &color, float size, float lifespan )
        : position( pos ), velocity( vel ), color( color ), size( size ), life( lifespan ) {
    }

    void update( float dt );
    bool isAlive() const;
};
