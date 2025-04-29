#define GLM_ENABLE_EXPERIMENTAL
#include "AtomSystem.h"
#include "Renderer.h"
#include "PeriodicTable.h"
#include "MathUtils.h"
#include "StringUtils.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <iostream>
#include <GLFW/glfw3.h>
#include <sstream>


extern Camera camera;
extern GLFWwindow *window;
extern Atom *selectedAtom;
extern Atom *hoveredAtom;


AtomSystem::AtomSystem( unsigned int maxAtoms, TextRenderer &tr )
    : maxAtoms( maxAtoms ), textRenderer( tr ) {
}

void AtomSystem::update( float dt ) {
    for (auto &bond : bonds)
        bond.applyForce();

    applyVSEPRForces( dt );

    for (auto &atom : atoms)
        atom.update( dt );

    updatePolarities();       

}


void AtomSystem::render( int w, int h ) {
    // determine first?central geometry once
    if (firstCentralGeometry.empty())
    {
        for (auto &a : atoms)
            if (a.bondedAtoms.size() >= 2 || a.lonePairs > 0)
            {
                firstCentralGeometry = determineGeometry( a );
                break;
            }
    }

    // draw bonds
    for (auto &b : bonds)
    {
        b.render( w, h );
    }

    for (auto &a : atoms)
    {
        bool isSel = (&a == selectedAtom) || (&a == hoveredAtom);
        Renderer::DrawAtom( a, w, h, isSel );
    }

    if (hoveredAtom)
    {
        const Element &e = PeriodicTable::Instance().Get( hoveredAtom->type );
        int s = 0, d = 0, t = 0;
        for (auto &b : bonds)
            if (b.atomA == hoveredAtom || b.atomB == hoveredAtom)
            {
                if (b.type == BondType::SINGLE) ++s;
                if (b.type == BondType::DOUBLE) ++d;
                if (b.type == BondType::TRIPLE) ++t;
            }
        float mu = glm::length( hoveredAtom->polarityDir );

        std::ostringstream oss;
        oss << e.symbol << " (" << e.atomicNumber << ") " << e.atomicMass << " u\n"
            << "EN: " << e.electronegativity << "\n"
            << "Dipole: " << mu << "\n"
            << "LonePairs: " << hoveredAtom->lonePairs << "\n"
            << "Bonds: " << hoveredAtom->bondedAtoms.size() << "\n";
        if (s) oss << " - Single: " << s << "\n";
        if (d) oss << " - Double: " << d << "\n";
        if (t) oss << " - Triple: " << t << "\n";

        float sx = 10, sy = 260, dy = 20; int line = 0;
        std::istringstream iss( oss.str() );
        std::string ln;
        while (std::getline( iss, ln ))
        {
            textRenderer.DrawScreenText( ln, sx, sy + line * dy, w, h );
            ++line;
        }
    }

    for (auto &a : atoms)
    {
        std::string lbl = a.type + StringUtils::chargeString( a.formalCharge );
        textRenderer.DrawText( lbl, a.position, w, h );
    }
    if (!firstCentralGeometry.empty())
    {
        textRenderer.DrawScreenText( "Geometry: " + firstCentralGeometry, 10, 90, w, h );
    }

    std::unordered_map<Atom *, std::vector<glm::vec3>> lpmap;
    computeLonePairPositions( lpmap );
    for (auto &kv : lpmap)
        for (auto &pos : kv.second)
        {
            Atom dot( "LP", pos, 0.1f );
            dot.radius = 0.08f; dot.color = glm::vec3( 0.7f, 0.7f, 1.f );
            glEnable( GL_BLEND );
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            glDepthMask( GL_FALSE );
            Renderer::DrawAtom( dot, w, h );
        }
    glDisable( GL_BLEND );
    glDepthMask( GL_TRUE );

    for (auto &a : atoms)
    {
        glm::vec3 st = a.position + glm::vec3( 0, a.radius + 0.1f, 0 );
        glm::vec3 en = st + a.polarityDir * 0.5f;
        Renderer::DrawArrow( st, en, glm::vec3( 1, 0, 0 ), w, h );
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

// TODO: expanded octet
void AtomSystem::updateLonePairs() {
    auto &pt = PeriodicTable::Instance();

    for (Atom &a : atoms)
    {
        int bondedElectrons = 0;
        for (Bond &b : bonds)
            if (b.atomA == &a || b.atomB == &a)
                bondedElectrons += 2 * b.bondOrder();           

        int desired = (a.type == "H") ? 2 : 8;

        int remaining = std::max( 0, desired - bondedElectrons );
        a.lonePairs = remaining / 2;
    }
}

float AtomSystem::computeDipole() {
    netDipole = glm::vec3( 0.0f );
    auto &pt = PeriodicTable::Instance();

    for (auto &bond : bonds)
    {
        float enA = pt.Get( bond.atomA->type ).electronegativity;
        float enB = pt.Get( bond.atomB->type ).electronegativity;
        float deltaEN = fabs( enA - enB );

        // use the full displacement vector
        glm::vec3 disp = bond.atomB->position - bond.atomA->position;
        // bond dipole -> (d) EN * (rB - rA)
        netDipole += disp * deltaEN;
    }

    dipoleMag = glm::length( netDipole );
    isPolar = (dipoleMag > 1e-2f);
    return dipoleMag;
}


void AtomSystem::computeLonePairPositions(
    std::unordered_map<Atom *, std::vector<glm::vec3>> &out ) {
    out.clear();
    lonePairAtoms.clear();

    static const glm::vec3 tetraDirs[ 4 ] = {
        glm::normalize( glm::vec3( 1,  1,  1 ) ),
        glm::normalize( glm::vec3( -1, -1,  1 ) ),
        glm::normalize( glm::vec3( 1, -1, -1 ) ),
        glm::normalize( glm::vec3( -1,  1, -1 ) )
    };

    const float centreOffset = 0.0f;   // raise LPs above the sphere surface
    const float spreadFactor = 0.9f;   // lateral offset for the “pair” dots

    for (Atom &atom : atoms)
    {
        int LP = atom.lonePairs;
        if (LP <= 0) continue;


        std::vector<glm::vec3> lpDirs;
        int bonded = (int)atom.bondedAtoms.size();
        int groups = bonded + LP;

        if (groups <= 3)      // trigonal planar or linear situations
        {
            for (int i = 0; i < LP; ++i)
            {
                float ang = i * glm::two_pi<float>() / LP;
                lpDirs.emplace_back( cos( ang ), sin( ang ), 0.0f );
            }
        }
        else                  // use remaining corners of a tetrahedron
        {
            bool used[ 4 ] = { false,false,false,false };
            for (Atom *nb : atom.bondedAtoms)
            {
                glm::vec3 v = glm::normalize( nb->position - atom.position );
                float best = -2.0f; int idx = -1;
                for (int j = 0; j < 4; ++j)
                {
                    float d = glm::dot( v, tetraDirs[ j ] );
                    if (d > best)
                    {
                        best = d; idx = j;
                    }
                }
                if (idx >= 0) used[ idx ] = true;
            }
            for (int j = 0; j < 4 && (int)lpDirs.size() < LP; ++j)
                if (!used[ j ]) lpDirs.push_back( tetraDirs[ j ] );
        }

        for (glm::vec3 dir : lpDirs)
        {
            dir = glm::normalize( dir );
            glm::vec3 ref = (fabs( dir.y ) < 0.99f) ? glm::vec3( 0, 1, 0 ) : glm::vec3( 1, 0, 0 );
            glm::vec3 perp = glm::normalize( glm::cross( dir, ref ) );

            float baseR = atom.radius + centreOffset;
            float spread = atom.radius * spreadFactor;

            glm::vec3 centre = atom.position + dir * baseR;

            glm::vec3 p1 = centre + perp * spread;
            glm::vec3 p2 = centre - perp * spread;

            out[ &atom ].push_back( p1 );
            out[ &atom ].push_back( p2 );

            Atom dot1( "LP", p1, 0.1f );
            dot1.radius = 0.08f;
            dot1.color = glm::vec3( 0.7f, 0.7f, 1.0f );
            lonePairAtoms.push_back( dot1 );

            Atom dot2 = dot1;
            dot2.position = p2;
            lonePairAtoms.push_back( dot2 );
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

    if (groups == 2) return "Linear";
    if (groups == 3) return (lp == 0 ? "Trigonal planar" : "Bent");
    if (groups == 4)
    {
        if (lp == 0) return "Tetrahedral";
        if (lp == 1) return "Trigonal pyramidal";
        if (lp == 2) return "Bent";
    }
    return "unknown";
}

void AtomSystem::updateFormalCharges() {
    for (Atom &atom : atoms)
    {
        /* count electron PAIRS in bonds to this atom */
        int bondPairs = 0;
        for (Bond &b : bonds)
            if (b.atomA == &atom || b.atomB == &atom)
                bondPairs += b.bondOrder();   // 1, 2 or 3

        int V = PeriodicTable::Instance().Get( atom.type ).valenceElectrons;
        int LP = atom.lonePairs;              // already known

        atom.formalCharge = V - 2 * LP - bondPairs;
    }
}

void AtomSystem::build( const std::vector<std::string> &symbols,
    const std::vector<std::tuple<int, int, int>> &bondList ) {
    // fresh start
    atoms.clear();
    bonds.clear();
    firstCentralGeometry.clear();
    singleBonds = doubleBonds = tripleBonds = sigmaBonds = piBonds = 0;

    for (const auto &s : symbols)
        spawnAtom( s );

    for (const auto &[a, b, order] : bondList)
    {
        BondType t = BondType::SINGLE;
        if (order == 2) t = BondType::DOUBLE;
        else if (order == 3) t = BondType::TRIPLE;

        // running counts
        if (order == 1)
        {
            singleBonds++;  sigmaBonds++;
        }
        if (order == 2)
        {
            doubleBonds++;  sigmaBonds++; piBonds++;
        }
        if (order == 3)
        {
            tripleBonds++;  sigmaBonds++; piBonds += 2;
        }

        createBond( a, b, t );
    }

    glm::vec3 com( 0.0f );
    float     totalM = 0.0f;
    for (const Atom &a : atoms)
    {
        com += a.mass * a.position; totalM += a.mass;
    }
    if (totalM > 0.0f) com /= totalM;

    for (Atom &a : atoms)
        a.position -= com;

    glm::vec3 P( 0.0f );                      // net linear momentum
    for (const Atom &a : atoms)
        P += a.mass * a.velocity;

    glm::vec3 vCM = (totalM > 0.0f) ? P / totalM : glm::vec3( 0.0f );
    for (Atom &a : atoms)
        a.velocity -= vCM;

    // kill residual angular velocity for a calm start
    for (Atom &a : atoms)
        a.velocity = glm::vec3( 0.0f );
}


void AtomSystem::updatePolarities() {
    auto &pt = PeriodicTable::Instance();
    for (Atom &a : atoms)
    {
        glm::vec3 sum( 0.0f );
        float enA = pt.Get( a.type ).electronegativity;

        for (Atom *nb : a.bondedAtoms)
        {
            float enB = pt.Get( nb->type ).electronegativity;
            float dEN = fabs( enA - enB );

            glm::vec3 disp = nb->position - a.position;
            // Only normalize after sum
            sum += glm::normalize( disp ) * dEN * glm::length( disp );
        }

        // Lower tolerance 
        if (glm::length( sum ) > 1e-6f)
        {
            a.polarityDir = glm::normalize( sum );
        }
        else
        {
            a.polarityDir = glm::vec3( 0, 1, 0 );
        }
    }
}
