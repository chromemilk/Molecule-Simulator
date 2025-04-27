#include "AtomSystem.h"
#include "Renderer.h"
#include "PeriodicTable.h"
#include "MathUtils.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <GLFW/glfw3.h>

extern Camera camera;
extern GLFWwindow *window;

AtomSystem::AtomSystem( unsigned int maxAtoms, TextRenderer &tr )
    : maxAtoms( maxAtoms ), textRenderer( tr ) {
}

void AtomSystem::update( float dt ) {
    for (auto &bond : bonds)
        bond.applyForce();

    applyVSEPRForces( dt );

    for (auto &atom : atoms)
        atom.update( dt );
}
void AtomSystem::render( int w, int h ) {

    /* first central atom's geometry for HUD */
    if (firstCentralGeometry.empty())
    {
        for (auto &a : atoms)
        {
            if (a.bondedAtoms.size() >= 2 || a.lonePairs > 0)
            {
                firstCentralGeometry = determineGeometry( a );
                break;
            }
        }
    }

    for (auto &b : bonds)
    {
        b.render( w, h );
    }

    for (auto &a : atoms)
    {
        Renderer::DrawAtom( a, w, h );
    }

    for (auto &a : atoms)
    {
        textRenderer.DrawText( a.type, a.position, w, h );
    }

    renderBondAngles( w, h );

    // Lone pairs as smaller spheres 
    std::unordered_map<Atom *, std::vector<glm::vec3>> lp;
    computeLonePairPositions( lp );

    for (auto &kv : lp)
        for (auto &pos : kv.second)
        {
            Atom dot( "LP", pos, 1.f );
            dot.radius = 0.08f;
            dot.color = glm::vec3( 0.7f, 0.7f, 1.0f );
            Renderer::DrawAtom( dot, w, h );
        }

    /* HUD: geometry name (top-left) */
    if (!firstCentralGeometry.empty())
    {
        textRenderer.DrawScreenText( "Geometry: " + firstCentralGeometry,
            10, 90, w, h );
    }

}


void AtomSystem::spawnAtom( const std::string &type ) {
    if (atoms.size() < maxAtoms)
    {
        glm::vec3 pos = MathUtils::GetRandomVector3( -2.0f, 2.0f );
        atoms.emplace_back( type, pos, 1.0f );
    }
}

void AtomSystem::addAtom( const Atom &atom ) {
    if (atoms.size() < maxAtoms)
        atoms.push_back( atom );
}

void AtomSystem::createBond( int indexA, int indexB, BondType bondType ) {
    if (indexA >= 0 && indexB >= 0 && indexA < atoms.size() && indexB < atoms.size())
    {
        bonds.emplace_back( &atoms[ indexA ], &atoms[ indexB ], bondType );
        atoms[ indexA ].bondedAtoms.push_back( &atoms[ indexB ] );
        atoms[ indexB ].bondedAtoms.push_back( &atoms[ indexA ] );
    }
    else
    {
        std::cerr << "Invalid bond indices!" << std::endl;
    }
}

void AtomSystem::applyVSEPRForces( float dt ) {
    for (auto &atom : atoms)
    {
        if (atom.bondedAtoms.size() < 2) continue;

        for (size_t i = 0; i < atom.bondedAtoms.size(); ++i)
        {
            for (size_t j = i + 1; j < atom.bondedAtoms.size(); ++j)
            {
                Atom *neighborA = atom.bondedAtoms[ i ];
                Atom *neighborB = atom.bondedAtoms[ j ];

                glm::vec3 vecA = glm::normalize( neighborA->position - atom.position );
                glm::vec3 vecB = glm::normalize( neighborB->position - atom.position );

                float currentAngle = glm::degrees( acos( glm::clamp( glm::dot( vecA, vecB ), -1.0f, 1.0f ) ) );
                float idealAngle = getIdealBondAngle( atom );

                float angleError = currentAngle - idealAngle;

                if (fabs( angleError ) > 0.5f)
                {
                    glm::vec3 correction = glm::normalize( vecA + vecB ) * (angleError * 0.2f);

                    if (!neighborA->fixed) neighborA->velocity += correction;
                    if (!neighborB->fixed) neighborB->velocity += correction;

                    latestCorrectionStrength = glm::length( correction );
                }
            }
        }
    }
}

void AtomSystem::renderBondAngles( int windowWidth, int windowHeight ) {
    for (auto &atom : atoms)
    {
        if (atom.bondedAtoms.size() < 2) continue;

        for (size_t i = 0; i < atom.bondedAtoms.size(); ++i)
        {
            for (size_t j = i + 1; j < atom.bondedAtoms.size(); ++j)
            {
                Atom *neighborA = atom.bondedAtoms[ i ];
                Atom *neighborB = atom.bondedAtoms[ j ];

                glm::vec3 vecA = neighborA->position - atom.position;
                glm::vec3 vecB = neighborB->position - atom.position;

                float angle = glm::degrees( acos( glm::clamp( glm::dot( glm::normalize( vecA ), glm::normalize( vecB ) ), -1.0f, 1.0f ) ) );

                glm::vec3 labelPos = (atom.position + neighborA->position + neighborB->position) / 3.0f;

                float ideal = getIdealBondAngle( atom );
                std::string label = "Angle: " + std::to_string( int( angle ) ) + " (Ideal: " + std::to_string( int( ideal ) ) + ")";

                textRenderer.DrawText( label, labelPos, windowWidth, windowHeight );
            }
        }
    }
}

void AtomSystem::updateLonePairs() {
    for (auto &atom : atoms)
    {
        int bondElectronPairs = 0;
        for (auto &bond : bonds)
        {
            if (bond.atomA == &atom || bond.atomB == &atom)
                bondElectronPairs += bond.bondOrder();
        }
        int valence = PeriodicTable::Instance().Get( atom.type ).valenceElectrons;
        int lonePairElectrons = std::max( 0, valence - bondElectronPairs );
        atom.lonePairs = lonePairElectrons / 2;
    }
}

float AtomSystem::computeDipole() {
    netDipole = glm::vec3( 0.0f );
    auto &pt = PeriodicTable::Instance();

    for (auto &bond : bonds)
    {
        const auto &elemA = pt.Get( bond.atomA->type );
        const auto &elemB = pt.Get( bond.atomB->type );

        float enA = elemA.electronegativity;
        float enB = elemB.electronegativity;

        glm::vec3 dir = glm::normalize( bond.atomB->position - bond.atomA->position );

        if (enA > enB) dir = -dir;

        float deltaEN = fabs( enA - enB );        // strength
        netDipole += dir * deltaEN;          // accumulate vector
    }

    dipoleMag = glm::length( netDipole );
    isPolar = (dipoleMag > 1e-2f);
    return dipoleMag;                            
}


void AtomSystem::computeLonePairPositions(
    std::unordered_map<Atom *, std::vector<glm::vec3>> &out ) {
    out.clear();

    /* universal tetrahedral directions (object-space unit vectors) */
    static const glm::vec3 tetraDir[ 4 ] = {
        glm::normalize( glm::vec3( 1,  1,  1 ) ),
        glm::normalize( glm::vec3( -1, -1,  1 ) ),
        glm::normalize( glm::vec3( 1, -1, -1 ) ),
        glm::normalize( glm::vec3( -1,  1, -1 ) )
    };

    for (auto &atom : atoms)
    {
        if (atom.lonePairs == 0) continue;

        int bonded = (int)atom.bondedAtoms.size();
        int groups = bonded + atom.lonePairs;

        if (groups <= 3)
        {
            glm::vec3 ref( 1, 0, 0 );
            if (bonded > 0) ref = glm::normalize( atom.bondedAtoms[ 0 ]->position - atom.position );

            glm::vec3 normal( 0, 0, 1 );                                      // default plane normal
            if (bonded >= 2)
                normal = glm::normalize( glm::cross(
                    atom.bondedAtoms[ 0 ]->position - atom.position,
                    atom.bondedAtoms[ 1 ]->position - atom.position ) );

            for (int i = 0; i < atom.lonePairs; ++i)
            {
                float ang = i * glm::two_pi<float>() / atom.lonePairs;
                glm::vec3 dir = glm::normalize( glm::vec3(
                    ref.x * cos( ang ) - ref.y * sin( ang ),
                    ref.x * sin( ang ) + ref.y * cos( ang ),
                    0.0f ) );

                if (groups == 3 && atom.lonePairs == 1) dir = normal;          // trig-planar, single LP -> above plane
                dir = glm::normalize( dir );
                out[ &atom ].push_back( atom.position + dir * (atom.radius + 0.02f) );
            }
            continue;
        }

        /* ------------------------------------------------ tetrahedral family (groups ==4) */
        if (groups == 4)
        {
            /* mark which tetra direction already occupied by bonds */
            bool used[ 4 ] = { false,false,false,false };
            for (Atom *nb : atom.bondedAtoms)
            {
                glm::vec3 v = glm::normalize( nb->position - atom.position );
                float bestDot = -2; int best = -1;
                for (int i = 0; i < 4; ++i)
                {
                    float d = glm::dot( v, tetraDir[ i ] );
                    if (d > bestDot)
                    {
                        bestDot = d; best = i;
                    }
                }
                if (best >= 0) used[ best ] = true;
            }

            /* assign remaining dirs to LPs */
            int left = atom.lonePairs;
            for (int i = 0; i < 4 && left; ++i)
            {
                if (!used[ i ])
                {
                    out[ &atom ].push_back( atom.position + tetraDir[ i ] * (atom.radius + 0.02f) );
                    --left;
                }
            }
            continue;
        }

        for (int i = 0; i < atom.lonePairs; ++i)
        {
            float ang = i * glm::two_pi<float>() / atom.lonePairs;
            glm::vec3 dir( cos( ang ), sin( ang ), 0 );
            out[ &atom ].push_back( atom.position + dir * (atom.radius + 0.02f) );
        }
    }
}

float AtomSystem::getIdealBondAngle( const Atom &atom ) {
    int bonded = atom.bondedAtoms.size();
    int lonePairs = atom.lonePairs;
    int totalGroups = bonded + lonePairs;

    if (totalGroups == 2) return 180.0f;
    if (totalGroups == 3) return 120.0f;
    if (totalGroups == 4)
    {
        if (lonePairs == 0) return 109.5f;
        if (lonePairs == 1) return 107.0f;
        if (lonePairs == 2) return 104.5f;
    }
    return 109.5f;
}


std::string AtomSystem::determineGeometry( const Atom &a ) const {
    int bondedGroups = (int)a.bondedAtoms.size();
    int lp = a.lonePairs;
    int groups = bondedGroups + lp;

    if (groups == 2) return "linear";
    if (groups == 3) return (lp == 0 ? "trigonal planar" : "bent");
    if (groups == 4)
    {
        if (lp == 0) return "tetrahedral";
        if (lp == 1) return "trigonal pyramidal";
        if (lp == 2) return "bent";
    }
    return "unknown";
}
