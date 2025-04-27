#include "ParticleSystem.h"
#include "Renderer.h"  
#include "MathUtils.h"      // For random functions

ParticleSystem::ParticleSystem( unsigned int maxParticles )
    : maxParticles( maxParticles ) {
}

void ParticleSystem::update( float dt ) {
    for (auto &p : particles)
    {
        if (p.isAlive())
        {
            p.update( dt );
        }
    }

    // Detect particle-particle collisions
    const float collisionDistance = 0.1f; 

    for (size_t i = 0; i < particles.size(); ++i)
    {
        for (size_t j = i + 1; j < particles.size(); ++j)
        {
            if (particles[ i ].isAlive() && particles[ j ].isAlive())
            {
                glm::vec3 delta = particles[ i ].position - particles[ j ].position;
                float distance = glm::length( delta );

                if (distance < collisionDistance)
                {
                    // Simple elastic collision: swap velocities
                    glm::vec3 temp = particles[ i ].velocity;
                    particles[ i ].velocity = particles[ j ].velocity;
                    particles[ j ].velocity = temp;
                }
            }
        }
    }

    // Remove dead particles
    particles.erase(
        std::remove_if( particles.begin(), particles.end(), []( const Particle &p ) {
            return !p.isAlive();
            } ),
        particles.end()
    );
}

void ParticleSystem::render() {
    for (auto &p : particles)
    {
        if (p.isAlive())
        {
          //  Renderer::DrawParticle( p.position, p.color, p.size );
        }
    }
}

void ParticleSystem::spawnParticle() {
    if (particles.size() < maxParticles)
    {
        glm::vec3 pos = MathUtils::GetRandomVector3( -0.5f, 0.5f );
        glm::vec3 vel = MathUtils::GetRandomVector3( -0.5f, 0.5f );

        glm::vec3 color = MathUtils::GetRandomVector3( 0.5f, 1.0f ); // bright colors
        float size = 5.0f + static_cast<float>(rand() % 10); // random size between 5 and 15

        float lifespan = 5.0f; // seconds
        particles.emplace_back( pos, vel, color, size, lifespan );
    }
}
