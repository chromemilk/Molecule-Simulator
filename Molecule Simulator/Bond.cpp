#include "Bond.h"
#include "Renderer.h"
#include <glm/glm.hpp>

Bond::Bond( Atom *a, Atom *b, BondType t ) : atomA( a ), atomB( b ), type( t ) {
    restLen = glm::length( atomB->position - atomA->position );
}


void Bond::applyForce() {
    if (!atomA || !atomB) return;

    glm::vec3 d = atomB->position - atomA->position;
    float      L = glm::length( d );
    if (L == 0) return;

    glm::vec3 dir = d / L;
    float      k = 4.0f;                 // spring constant
    float      x = L - restLen;          
    glm::vec3  F = k * x * dir;

    if (!atomA->fixed) atomA->velocity += F / atomA->mass;
    if (!atomB->fixed) atomB->velocity += -F / atomB->mass;
}


void Bond::render( int w, int h ) const {
    if (!atomA || !atomB) return;

    glm::vec3 A = atomA->position;
    glm::vec3 B = atomB->position;

    glm::vec3 dir = glm::normalize( B - A );

   /*
    A += dir * atomA->radius;   // stop at sphere surface
    B -= dir * atomB->radius;
    */

    float baseR = 0.06f;
    float r = baseR;
    if (type == BondType::DOUBLE) r = baseR * 1.3f;
    if (type == BondType::TRIPLE) r = baseR * 1.5f;

    float insetA = 0.0f;
    float insetB = 0.0f;
    // Now it should actually touch the spheres 
    if (atomA->radius > r) insetA = glm::sqrt( atomA->radius * atomA->radius - r * r );
    if (atomB->radius > r) insetB = glm::sqrt( atomB->radius * atomB->radius - r * r );

    A += dir * insetA;
    B -= dir * insetB;


    glm::vec3 up( 0, 1, 0 );
    if (fabs( glm::dot( dir, up ) ) > .9f) up = glm::vec3( 1, 0, 0 );
    glm::vec3 right = glm::normalize( glm::cross( dir, up ) ) * 0.15f;

    glm::vec3 bondColor( 0.8f ); // default gray for single bond
    if (type == BondType::DOUBLE) bondColor = glm::vec3( 0.0f, 1.0f, 0.0f ); // green
    if (type == BondType::TRIPLE) bondColor = glm::vec3( 0.0f, 0.0f, 1.0f ); // blue

    if (type == BondType::SINGLE)
    {
        Renderer::DrawBondCylinder( A, B, r, w, h, bondColor );
    }
    else if (type == BondType::DOUBLE)
    {
        Renderer::DrawBondCylinder( A + right, B + right, r, w, h, bondColor );
        Renderer::DrawBondCylinder( A - right, B - right, r, w, h, bondColor );
    }
    else                     // TRIPLE
    {
        Renderer::DrawBondCylinder( A, B, r, w, h, bondColor );
        Renderer::DrawBondCylinder( A + right, B + right, r, w, h, bondColor );
        Renderer::DrawBondCylinder( A - right, B - right, r, w, h, bondColor );
    }
}
