#include "Bond.h"
#include "Renderer.h"
#include <glm/glm.hpp>

Bond::Bond( Atom *a, Atom *b, BondType t ) : atomA( a ), atomB( b ), type( t ) {
}

void Bond::applyForce() {
    if (!atomA || !atomB) return;

    glm::vec3 d = atomB->position - atomA->position;
    float     L = glm::length( d );
    if (L == 0) return;

    glm::vec3 dir = d / L;
    float      k = 4.0f;          // spring constant
    float      x = L - 1.0f;      // rest length = 1
    glm::vec3 F = k * x * dir;

    if (!atomA->fixed) atomA->velocity += F / atomA->mass;
    if (!atomB->fixed) atomB->velocity += -F / atomB->mass;
}

void Bond::render( int w, int h ) const {
    if (!atomA || !atomB) return;

    glm::vec3 A = atomA->position;
    glm::vec3 B = atomB->position;

    glm::vec3 dir = glm::normalize( B - A );
    A += dir * atomA->radius;   // stop at sphere surface
    B -= dir * atomB->radius;

    float baseR = 0.06f;
    float r = baseR;
    if (type == BondType::DOUBLE) r = baseR * 1.3f;
    if (type == BondType::TRIPLE) r = baseR * 1.5f;

    glm::vec3 up( 0, 1, 0 );
    if (fabs( glm::dot( dir, up ) ) > .9f) up = glm::vec3( 1, 0, 0 );
    glm::vec3 right = glm::normalize( glm::cross( dir, up ) ) * 0.15f;

    if (type == BondType::SINGLE)
    {
        Renderer::DrawBondCylinder( A, B, r, w, h );
    }
    else if (type == BondType::DOUBLE)
    {
        Renderer::DrawBondCylinder( A + right, B + right, r, w, h );
        Renderer::DrawBondCylinder( A - right, B - right, r, w, h );
    }
    else                     // TRIPLE
    {
        Renderer::DrawBondCylinder( A, B, r, w, h );
        Renderer::DrawBondCylinder( A + right, B + right, r, w, h );
        Renderer::DrawBondCylinder( A - right, B - right, r, w, h );
    }
}
