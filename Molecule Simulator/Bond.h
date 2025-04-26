#pragma once

#include <glm/glm.hpp>

class Atom; // Forward declaration to avoid circular include

enum class BondType
{
    Single,
    Double,
    Triple
};

class Bond
{
public:
    Atom *atomA;
    Atom *atomB;
    float restLength;
    float stiffness;
    BondType type;

    Bond( Atom *a, Atom *b, BondType bondType = BondType::Single );

    void applyForce();
    void render() const;
};
