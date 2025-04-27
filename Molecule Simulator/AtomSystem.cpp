#include "AtomSystem.h"
#include "Renderer.h"
#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <iostream>
#include "MathUtils.h"

extern Camera camera;
extern GLFWwindow *window;

AtomSystem::AtomSystem( unsigned int maxAtoms, TextRenderer &textRenderer )
    : maxAtoms( maxAtoms ), textRenderer( textRenderer ) {
}

void AtomSystem::update( float dt ) {
    for (auto &bond : bonds)
    {
        bond.applyForce();
    }

    applyVSEPRForces( dt );

    for (auto &atom : atoms)
    {
        atom.update( dt );
    }
}

void AtomSystem::render( int windowWidth, int windowHeight ) {
    // Save OpenGL state
    GLboolean wasDepthTestEnabled = glIsEnabled( GL_DEPTH_TEST );
    GLboolean wasBlendEnabled = glIsEnabled( GL_BLEND );

    // Draw all bonds
    for (auto &bond : bonds)
    {
        bond.render();
    }

    // Draw all atoms
    for (auto &atom : atoms)
    {
        Renderer::DrawParticle( atom.position, atom.color, atom.radius * 60.0f );
    }

    // Draw atom labels above each atom
    for (auto &atom : atoms)
    {
        textRenderer.DrawText( atom.type, atom.position, windowWidth, windowHeight );
    }

	renderBondAngles( windowWidth, windowHeight );


    // Draw lone pairs
    std::unordered_map<Atom *, std::vector<glm::vec3>> lonePairPositions;
    computeLonePairPositions( lonePairPositions );

    for (auto &kv : lonePairPositions)
    {
        for (auto &lpPos : kv.second)
        {
            Renderer::DrawParticle( lpPos, glm::vec3( 0.8f, 0.8f, 0.8f ), 5.0f ); // light gray dot
        }

     //   if (!kv.second.empty())
       // {
      //     glm::vec3 firstLpPos = kv.second.front();
      //      textRenderer.DrawText( "LP", firstLpPos, windowWidth, windowHeight );
      //  }
    }

    // Restore OpenGL state
    if (!wasDepthTestEnabled) glDisable( GL_DEPTH_TEST );
    else glEnable( GL_DEPTH_TEST );

    if (!wasBlendEnabled) glDisable( GL_BLEND );
    else glEnable( GL_BLEND );
}


void AtomSystem::spawnAtom( const std::string &type ) {
    if (atoms.size() < maxAtoms)
    {
        glm::vec3 pos = MathUtils::GetRandomVector3( -2.0f, 2.0f );
        float mass = 1.0f;
        atoms.emplace_back( type, pos, mass );
    }
}

void AtomSystem::addAtom( const Atom &atom ) {
    if (atoms.size() < maxAtoms)
    {
        atoms.push_back( atom );
    }
}

void AtomSystem::createBond( int indexA, int indexB, BondType bondType ) {
    if (indexA >= 0 && indexB >= 0 && indexA < atoms.size() && indexB < atoms.size())
    {
        bonds.emplace_back( &atoms[ indexA ], &atoms[ indexB ], bondType );
        atoms[ indexA ].bondedAtoms.push_back( &atoms[ indexB ] );
        atoms[ indexB ].bondedAtoms.push_back( &atoms[ indexA ] );
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
                    glm::vec3 correction = glm::normalize( vecA + vecB ) * (angleError * 0.15f);

                    latestCorrectionStrength = glm::length( correction ); 

                    // Just clamp until I make some PID or something 
                    if (glm::length( correction ) > 5.f)
                    {
						correction = glm::normalize( correction ) * 5.f; // cap the correction
                    }

                    if (!neighborA->fixed)
                        neighborA->velocity += correction;
                    if (!neighborB->fixed)
                        neighborB->velocity += correction;
                }
            }
        }
    }
}

static int valenceFor( const std::string &t ) {
    if (t == "H") return 1;
    if (t == "O") return 6;
    if (t == "N") return 5;
    if (t == "C") return 4;
    return 4;
}

float AtomSystem::getIdealBondAngle( const Atom &atom ) {
    int bondedGroups = 0;           // counts each neighbor once
    int bondElectronPairs = 0;      // counts total electron pairs

    for (auto &bond : bonds)
    {
        if (bond.atomA == &atom || bond.atomB == &atom)
        {
            bondedGroups += 1;
            bondElectronPairs += bond.bondOrder();
        }
    }

    int valence = valenceFor( atom.type );
    int lonePairElectrons = std::max( 0, valence - bondElectronPairs );
    int lonePairs = lonePairElectrons / 2;

    int totalGroups = bondedGroups + lonePairs;  // VSEPR groups

    if (totalGroups == 2) return 180.0f;          // linear
    if (totalGroups == 3) return 120.0f;          // trigonal planar
    if (totalGroups == 4)
    {                       // tetrahedral family
        if (lonePairs == 0) return 109.5f;        
        if (lonePairs == 1) return 107.0f;       
        if (lonePairs == 2) return 104.5f;        
    }
    // fallback
    return 109.5f;
}




void AtomSystem::updateLonePairs() {
    for (auto &atom : atoms)
    {
        int bondOrderSum = 0;
        for (auto &bond : bonds)
        {
            if (bond.atomA == &atom || bond.atomB == &atom)
            {
                bondOrderSum += bond.bondOrder(); // bondOrder = 1, 2, or 3
            }
        }

        int valence = valenceFor( atom.type );

        int lonePairElectrons = std::max( 0, valence - bondOrderSum );
        atom.lonePairs = lonePairElectrons / 2; // each lone pair = 2 electrons
    }
}





void AtomSystem::renderBondAngles( int windowWidth, int windowHeight ) {
    for (auto &atom : atoms)
    {
        // Only atoms that are bonded to 2+ neighbors (true angle centers)
        if (atom.bondedAtoms.size() < 2) continue;
        if (atom.type == "H") continue; // skip hydrogens

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

                std::string label = "Angle: " + std::to_string( (int)angle ) + " (Ideal: " + std::to_string( (int)ideal ) + ")";
                textRenderer.DrawText( label, labelPos, windowWidth, windowHeight );
            }
        }
    }

    // Draw correction aggression once at the top left corner
    std::string corrText = "Correction Aggression: " + std::to_string( latestCorrectionStrength );
    textRenderer.DrawScreenText( corrText, 10.0f, 30.0f, windowWidth, windowHeight );
}

static std::vector<glm::vec3> tetrDirs{
    { 1, 1, 1}, {-1,-1, 1},
    { 1,-1,-1}, {-1, 1,-1}
};
void AtomSystem::computeLonePairPositions( std::unordered_map<Atom *, std::vector<glm::vec3>> &out ) {
    out.clear();

    static const std::vector<glm::vec3> tetrahedralDirs = {
        glm::normalize( glm::vec3( 1, 1, 1 ) ),
        glm::normalize( glm::vec3( -1, -1, 1 ) ),
        glm::normalize( glm::vec3( 1, -1, -1 ) ),
        glm::normalize( glm::vec3( -1, 1, -1 ) )
    };

    for (auto &atom : atoms)
    {
        if (atom.lonePairs == 0) continue;

        int bonded = int( atom.bondedAtoms.size() );
        int groups = bonded + atom.lonePairs;

        if (groups == 4)
        {
            // tetrahedral
            std::vector<bool> used( 4, false );
            for (auto *neighbor : atom.bondedAtoms)
            {
                glm::vec3 bondVec = glm::normalize( neighbor->position - atom.position );
                float bestDot = -2.0f;
                int best = -1;
                for (int i = 0; i < 4; ++i)
                {
                    float dot = glm::dot( bondVec, tetrahedralDirs[ i ] );
                    if (dot > bestDot)
                    {
                        bestDot = dot;
                        best = i;
                    }
                }
                if (best >= 0) used[ best ] = true;
            }

            int remaining = atom.lonePairs;
            for (int i = 0; i < 4 && remaining > 0; ++i)
            {
                if (!used[ i ])
                {
                    glm::vec3 dir = tetrahedralDirs[ i ];
                    out[ &atom ].push_back( atom.position + dir * atom.radius * 1.8f );
                    out[ &atom ].push_back( atom.position + dir * atom.radius * 2.2f ); // slightly offset second electron
                    --remaining;
                }
            }
        }
        else
        {
            // fallback if not tetrahedral
            for (int i = 0; i < atom.lonePairs; ++i)
            {
                float angle = i * glm::two_pi<float>() / atom.lonePairs;
                glm::vec3 dir( cos( angle ), sin( angle ), 0 );
                out[ &atom ].push_back( atom.position + dir * atom.radius * 2.0f );
                out[ &atom ].push_back( atom.position + dir * atom.radius * 2.2f );
            }
        }
    }
}

