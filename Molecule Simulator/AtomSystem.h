#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>

#include "Atom.h"
#include "Bond.h"
#include "TextRenderer.h"

class AtomSystem
{
public:
    float latestCorrectionStrength = 0.0f;

    AtomSystem( unsigned int maxAtoms, TextRenderer &textRenderer );

    void update( float dt );
    void render( int windowWidth, int windowHeight );

    void spawnAtom( const std::string &type );
    void addAtom( const Atom &atom );
    void createBond( int indexA, int indexB, BondType bondType = BondType::SINGLE );

    void updateLonePairs(); // compute lone pairs
    void computeLonePairPositions( std::unordered_map<Atom *, std::vector<glm::vec3>> &out );


private:
    std::vector<Atom> atoms;
    std::vector<Bond> bonds;
    unsigned int maxAtoms;

    TextRenderer &textRenderer;

    void applyVSEPRForces( float dt );
    float getIdealBondAngle( const Atom &atom );
    void renderBondAngles( int windowWidth, int windowHeight );
};
