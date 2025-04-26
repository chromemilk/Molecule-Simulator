#pragma once
#include <glm/glm.hpp>
class Camera; // Forward declare

class Renderer
{
public:
    static void Init();
    static void Shutdown();
    static void DrawParticle( const glm::vec3 &position, const glm::vec3 &color, float size );
    static void SetCamera( Camera *cam );
};
