#pragma once
#include "Atom.h"

enum class BondType
{
    SINGLE = 1, DOUBLE = 2, TRIPLE = 3
};

class Bond
{
public:
    Bond( Atom *a, Atom *b, BondType t = BondType::SINGLE );

    void applyForce();

    void render( int windowW, int windowH ) const;

    int  bondOrder() const {
        return static_cast<int>(type);
    }

    bool contains( const glm::vec3 &rayO, const glm::vec3 &rayDir ) const;

    Atom *atomA{};
    Atom *atomB{};
    BondType type{ BondType::SINGLE };
    float restLen{ 1.0f };
};
