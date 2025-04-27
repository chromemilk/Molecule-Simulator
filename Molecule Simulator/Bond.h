#pragma once

#include <glm/glm.hpp>

class Atom; // Forward declaration

enum class BondType
{
    SINGLE = 1,
    DOUBLE = 2,
    TRIPLE = 3
};

class Bond
{
public:
    Atom *atomA;
    Atom *atomB;
    float restLength;
    float stiffness;
    BondType type;

    Bond( Atom *a, Atom *b, BondType bondType = BondType::SINGLE );

    void applyForce();
    void render() const;

    int bondOrder() const {
        return static_cast<int>(type);
    }
};
