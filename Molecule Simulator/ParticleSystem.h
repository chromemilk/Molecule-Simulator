#pragma once
#include <vector>
#include "Particle.h"

class ParticleSystem
{
public:
    ParticleSystem( unsigned int maxParticles );

    void update( float dt );
    void render();
    void spawnParticle();

private:
    std::vector<Particle> particles;
    unsigned int maxParticles;
};
