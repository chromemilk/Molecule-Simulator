#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "Mesh.h"
#include "Shader.h"
#include "Camera.h"
#include "Atom.h"

class Renderer
{
public:
    static void Init( Camera *cam );
    static void Shutdown();

    static void DrawAtom( const Atom &atom, int windowW, int windowH, bool highlight = false );

    static void DrawBondCylinder( const glm::vec3 &A, const glm::vec3 &B, float radius, int windowW, int windowH, const glm::vec3 &color = glm::vec3( 0.8f ) );
    
    
    static void DrawArrow( const glm::vec3 &start, const glm::vec3 &end, const glm::vec3 &color, int windowW, int windowH );

    static void DrawGrid( int windowW, int windowH );

private:
    static Mesh      sphereMesh;
    static Mesh      cylinderMesh;
    static Mesh      gridMesh;
    static Shader *sphereShader;
    static Shader *cylinderShader;
    static Shader *gridShader;
    static Camera *cameraPtr;

    // helpers
    static std::vector<glm::vec3> GenerateSphereVerts( int seg, int ring );
    static std::vector<glm::vec3> GenerateCylinderVerts( int seg );
};
