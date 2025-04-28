#define GLM_ENABLE_EXPERIMENTAL
#include "Renderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

// statics
Mesh    Renderer::sphereMesh;
Mesh    Renderer::cylinderMesh;
Mesh    Renderer::gridMesh;
Shader *Renderer::sphereShader = nullptr;
Shader *Renderer::cylinderShader = nullptr;
Shader *Renderer::gridShader = nullptr;
Camera *Renderer::cameraPtr = nullptr;

static glm::quat RotationBetween( const glm::vec3 &a, const glm::vec3 &b ) {
    glm::vec3 v0 = glm::normalize( a );
    glm::vec3 v1 = glm::normalize( b );
    float d = glm::dot( v0, v1 );
    if (d < -0.999f)
    {
        glm::vec3 axis = glm::cross( glm::vec3( 1, 0, 0 ), v0 );
        if (glm::length2( axis ) < 1e-4f)
            axis = glm::cross( glm::vec3( 0, 1, 0 ), v0 );
        return glm::angleAxis( glm::pi<float>(), glm::normalize( axis ) );
    }
    glm::vec3 axis = glm::cross( v0, v1 );
    float s = sqrt( (1 + d) * 2 );
    return glm::normalize( glm::quat( s * 0.5f, axis * (1.0f / s) ) );
}

std::vector<glm::vec3> Renderer::GenerateSphereVerts( int seg, int ring ) {
    std::vector<glm::vec3> pts;
    for (int y = 0; y <= ring; ++y)
    {
        for (int x = 0; x <= seg; ++x)
        {
            float xs = x / (float)seg, ys = y / (float)ring;
            float xp = cos( xs * glm::two_pi<float>() ) * sin( ys * glm::pi<float>() );
            float yp = cos( ys * glm::pi<float>() );
            float zp = sin( xs * glm::two_pi<float>() ) * sin( ys * glm::pi<float>() );
            pts.emplace_back( xp, yp, zp );
        }
    }
    std::vector<glm::vec3> tris;
    for (int y = 0; y < ring; ++y)
    {
        for (int x = 0; x < seg; ++x)
        {
            int a = y * (seg + 1) + x;
            int b = (y + 1) * (seg + 1) + x;
            int c = b + 1;
            int d = a + 1;
            tris.push_back( pts[ a ] ); tris.push_back( pts[ b ] ); tris.push_back( pts[ c ] );
            tris.push_back( pts[ a ] ); tris.push_back( pts[ c ] ); tris.push_back( pts[ d ] );
        }
    }
    return tris;
}

std::vector<glm::vec3> Renderer::GenerateCylinderVerts( int seg ) {
    std::vector<glm::vec3> verts;
    float h = 1.0f, r = 0.5f;
    for (int i = 0; i < seg; ++i)
    {
        float t1 = (float)i / seg * glm::two_pi<float>();
        float t2 = (float)(i + 1) / seg * glm::two_pi<float>();
        glm::vec3 p1( r * cos( t1 ), h * 0.5f, r * sin( t1 ) );
        glm::vec3 p2( r * cos( t2 ), h * 0.5f, r * sin( t2 ) );
        glm::vec3 p3( r * cos( t2 ), -h * 0.5f, r * sin( t2 ) );
        glm::vec3 p4( r * cos( t1 ), -h * 0.5f, r * sin( t1 ) );
        verts.insert( verts.end(), { p1,p2,p3, p1,p3,p4 } );
    }
    return verts;
}

void Renderer::Init( Camera *cam ) {
    cameraPtr = cam;
    sphereShader = new Shader( "sphere.vert", "sphere.frag" );
    cylinderShader = new Shader( "cyl.vert", "cyl.frag" );
    gridShader = new Shader( "grid.vert", "grid.frag" );

    sphereMesh.Build( GenerateSphereVerts( 16, 16 ) );
    cylinderMesh.Build( GenerateCylinderVerts( 16 ) );

    // build a giant XZ plane
    std::vector<glm::vec3> plane = {
        {-50,0,-50},{ 50,0,-50},{ 50,0, 50},
        {-50,0,-50},{ 50,0, 50},{-50,0, 50}
    };
    gridMesh.Build( plane );
}

void Renderer::Shutdown() {
    delete sphereShader;   sphereShader = nullptr;
    delete cylinderShader; cylinderShader = nullptr;
    delete gridShader;     gridShader = nullptr;
    sphereMesh = Mesh();
    cylinderMesh = Mesh();
    gridMesh = Mesh();
    cameraPtr = nullptr;
}

void Renderer::DrawGrid( int w, int h ) {
    glm::mat4 view = cameraPtr->GetViewMatrix();
    glm::mat4 proj = glm::perspective( glm::radians( cameraPtr->Zoom ),
        (float)w / (float)h, 0.1f, 100.0f );

    gridShader->use();

    glm::mat4 model = glm::mat4( 1.0f );

    // Center grid under camera XZ
    glm::vec3 camXZ = glm::vec3( cameraPtr->Position.x, 0.0f, cameraPtr->Position.z );
    model = glm::translate( model, camXZ );
    model = glm::translate( model, glm::vec3( 0.0f, -0.2f, 0.0f ) ); // drop a little below atoms

    gridShader->setMat4( "model", model );
    gridShader->setMat4( "view", view );
    gridShader->setMat4( "projection", proj );

    gridShader->setVec3( "cameraPos", cameraPtr->Position );

    gridMesh.Draw();
}

void Renderer::DrawAtom( const Atom &a, int w, int h, bool highlight ) {
    glm::mat4 proj = glm::perspective( glm::radians( 45.0f ),
        (float)w / (float)h, 0.1f, 100.0f );
    glm::mat4 model = glm::translate( glm::mat4( 1 ), a.position ) *
        glm::scale( glm::mat4( 1 ), glm::vec3( a.radius ) );

    sphereShader->use();
    sphereShader->setMat4( "view", cameraPtr->GetViewMatrix() );
    sphereShader->setMat4( "projection", proj );
    sphereShader->setMat4( "model", model );
    sphereShader->setVec3( "color", a.color );
    sphereShader->setVec3( "lightPos", glm::vec3( 5.0f, 5.0f, 5.0f ) );
    sphereShader->setVec3( "viewPos", cameraPtr->Position );
    sphereShader->setFloat( "emissiveBoost", highlight ? 1.0f : 0.0f );

    sphereMesh.Draw();
}



void Renderer::DrawBondCylinder( const glm::vec3 &A,
    const glm::vec3 &B,
    float radius,
    int w, int h,
    const glm::vec3 &color ) {
    glm::vec3 dir = B - A;
    float len = glm::length( dir );
    glm::vec3 mid = 0.5f * (A + B);
    glm::quat rot = RotationBetween( glm::vec3( 0, 1, 0 ), dir );

    glm::mat4 model = glm::translate( glm::mat4( 1 ), mid )
        * glm::mat4_cast( rot )
        * glm::scale( glm::mat4( 1 ), glm::vec3( radius, len * 0.5f, radius ) );
    glm::mat4 proj = glm::perspective( glm::radians( 45.0f ),
        (float)w / (float)h, 0.1f, 100.0f );

    cylinderShader->use();
    cylinderShader->setMat4( "view", cameraPtr->GetViewMatrix() );
    cylinderShader->setMat4( "projection", proj );
    cylinderShader->setMat4( "model", model );
    cylinderShader->setVec3( "color", color );   
    cylinderMesh.Draw();
}

void Renderer::DrawArrow( const glm::vec3 &start,
    const glm::vec3 &end,
    const glm::vec3 &color,
    int w, int h ) {
    glm::vec3 dir = end - start;
    float len = glm::length( dir );
    if (len < 1e-4f) return;
    glm::vec3 shaftEnd = start + dir * 0.8f;
    DrawBondCylinder( start, shaftEnd, 0.03f, w, h, glm::vec3( 1.0f, 1.0f, 1.0f ) );

    glm::vec3 up( 0, 1, 0 );
    if (fabs( glm::dot( glm::normalize( dir ), up ) ) > 0.999f) up = glm::vec3( 1, 0, 0 );
    glm::quat q = RotationBetween( up, glm::normalize( dir ) );
    glm::mat4 model = glm::translate( glm::mat4( 1 ), (shaftEnd + end) * 0.5f )
        * glm::mat4_cast( q )
        * glm::scale( glm::mat4( 1 ), glm::vec3( 0.07f, len * 0.2f * 0.5f, 0.0f ) );
    glm::mat4 proj = glm::perspective( glm::radians( 45.0f ), (float)w / (float)h, 0.1f, 100.0f );

    cylinderShader->use();
    cylinderShader->setMat4( "view", cameraPtr->GetViewMatrix() );
    cylinderShader->setMat4( "projection", proj );
    cylinderShader->setMat4( "model", model );
    cylinderShader->setVec3( "color", color );
    cylinderMesh.Draw();
}
