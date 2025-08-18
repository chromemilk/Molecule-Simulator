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
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/gtx/quaternion.hpp>
#include "Resonance.h" 



AtomSystem::AtomSystem( unsigned int maxAtoms, TextRenderer &tr )
    : maxAtoms( maxAtoms ), textRenderer( tr ) {
}

void AtomSystem::update( float dt ) {
    // Apply forces based on bonds/bond type
    for (Bond& b : bonds) {
        b.applyForce();
    }

    if (applyVESPR)
    {
        if (fastCorrection)
        {
            applyVSEPRAngleFast( dt );
        }
        else
        {
            applyVSEPRForces( dt );
        }
    }
    
    if (betterStabilization)
    {
        const float drag = 0.95f;   
        for (Atom &a : atoms)
        {
            a.velocity *= drag;
        }
    }
    // Update atoms with respect to delta time
    for (Atom& a : atoms) {
        a.update(dt);
    }


    // Set the individual polarities for atoms
    updatePolarities();

    // Get positions for lone pairs based on geometry
    computeLonePairDots();         
}


void AtomSystem::render( int w, int h, Atom *selectedAtom, Atom *hoveredAtom, Atom *bondFirst, Atom *breakFirst, const Camera &camera ) {


    if (firstCentralGeometry.empty())
    {
        int bestIndex = -1;
        int bestBonds = -1;
        for (int i = 0; i < (int)atoms.size(); ++i)
        {
            int bcount = (int)atoms[ i ].bondedAtoms.size();
            if (bcount > bestBonds)
            {
                bestBonds = bcount;
                bestIndex = i;
            }
            else if (bcount == bestBonds && bestIndex >= 0)
            {
                // tie-break: prefer fewer lone pairs
                if (atoms[ i ].lonePairs < atoms[ bestIndex ].lonePairs)
                {
                    bestIndex = i;
                }
            }
        }
        if (bestIndex >= 0)
        {
            firstCentralGeometry = determineGeometry( atoms[ bestIndex ] );
        }
    }


    for (Bond &b : bonds)
    {
        // Render the bonds 
        b.render( w, h );
    }

    for (Atom &a : atoms)
    {
        // Draw the atoms, and highlight if conditions are met
        Renderer::DrawAtom( a, w, h, (&a == selectedAtom) ||
            (&a == hoveredAtom) ||
            (&a == bondFirst) ||
            (&a == breakFirst) );
    }

    // Lone pair transparency 
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glDepthMask( GL_FALSE );

    // Draw lone pair dots 
    for (Atom &dot : lonePairDots)
    {
        Renderer::DrawAtom( dot, w, h, &dot == hoveredAtom );
    }

    // Disable transparency
    glDisable( GL_BLEND );
    glDepthMask( GL_TRUE );

    // Show calculated formal charges
 /*   for (Atom &a : atoms) {
        textRenderer.DrawText(a.type + StringUtils::chargeString(a.formalCharge),
            a.position, w, h);
    }*/

    // build projection & view once
    glm::mat4 proj = glm::perspective(
        glm::radians( camera.Zoom ),
        float( w ) / float( h ),
        0.1f, 100.0f
    );
    glm::mat4 view = camera.GetViewMatrix();

    for (Atom &a : atoms)
    {
        glm::vec4 clip = proj * view * glm::vec4( a.position, 1.0f );
        if (clip.w <= 0.0f) continue;         // behind camera
        glm::vec3 ndc = glm::vec3( clip ) / clip.w;
        float sx = (ndc.x * 0.5f + 0.5f) * w;
        float sy = (1.0f - (ndc.y * 0.5f + 0.5f)) * h;

        textRenderer.DrawScreenText(
            a.type + StringUtils::chargeString( a.formalCharge ),
            sx, sy, w, h
        );
    }



    // Render the computed bond angles
    renderBondAngles( w, h, camera, hoveredAtom );

    // Render computed geometry
    if (!firstCentralGeometry.empty()) {
        textRenderer.DrawScreenText("Geometry: " + firstCentralGeometry, (w / 2) - 30, 30, w, h);
    }

    // Draw the relevant info for the active atom
    if (hoveredAtom) {
        drawTooltip(*hoveredAtom, w, h);
    }

    for (Atom &a : atoms)
    {
        // Draw 3D dipole arrows 
        glm::vec3 s = a.position + glm::vec3( 0, a.radius + .1f, 0 );
        Renderer::DrawArrow( s, s + a.polarityDir * .5f,
            glm::vec3( 1, 0, 0 ), w, h );
    }
}

const std::vector<Atom> &AtomSystem::getLonePairs() const {
    return lonePairDots;
}
std::vector<Bond> &AtomSystem::getBonds() {
    return bonds;
}

const std::vector<Bond> &AtomSystem::getBonds() const {
    return bonds;
}

void AtomSystem::setDirtyLonePairs() {
    lonePairsDirty = true;
    firstCentralGeometry.clear();
}

// Create a specified atom in a random place in space
void AtomSystem::spawnAtom( const std::string &sym ) {
    if (atoms.size() >= maxAtoms) {
        return;
    }
    atoms.emplace_back( sym, MathUtils::GetRandomVector3( -2, 2 ), 1.f );
    setDirtyLonePairs();
}

// Add a specified atom
void AtomSystem::addAtom( const Atom &atom ) {
    if (atoms.size() < maxAtoms) {
        atoms.push_back(atom);
    }
}

// Create bonds between atoms 
void AtomSystem::createBond( int ia, int ib, BondType t ) {
    if (ia < 0 || ib < 0 || ia >= atoms.size() || ib >= atoms.size())
    {
        std::cerr << "ERR: BOND INVALID!\n"; return;
    }

    switch (t)
    {
    case BondType::SINGLE:  ++singleBonds;  ++sigmaBonds;            break;
    case BondType::DOUBLE:  ++doubleBonds; ++sigmaBonds; ++piBonds;  break;
    case BondType::TRIPLE:  ++tripleBonds; ++sigmaBonds; piBonds += 2; break;
    }

    bonds.emplace_back( &atoms[ ia ], &atoms[ ib ], t );
    atoms[ ia ].bondedAtoms.push_back( &atoms[ ib ] );
    atoms[ ib ].bondedAtoms.push_back( &atoms[ ia ] );

    setDirtyLonePairs();        
}


static glm::vec3 safeNormalize( const glm::vec3 &v, float eps = 1e-6f ) {
    float len = glm::length( v );
    return (len > eps) ? v / len : glm::vec3( 0.0f );
}


void AtomSystem::applyVSEPRForces( float dt ) {
    float k = correctionProportion;
    float maxStep = glm::radians( maxCorrectionPerStep ); // cap per frame
    latestCorrectionStrength = 0.0f;

    for (auto &atom : atoms)
    {
        const size_t n = atom.bondedAtoms.size();
        if (n < 2) continue;

        // steric number for pairwise targets (AX5/AX6)
        const int totalDomains =
            int( atom.bondedAtoms.size() ) + atom.lonePairs + (atom.hasRadical ? 1 : 0);

        for (size_t i = 0; i < n; ++i)
        {
            for (size_t j = i + 1; j < n; ++j)
            {
                Atom *A = atom.bondedAtoms[ i ];
                Atom *B = atom.bondedAtoms[ j ];

                glm::vec3 vA = A->position - atom.position;
                glm::vec3 vB = B->position - atom.position;
                if (glm::length2( vA ) < 1e-8f || glm::length2( vB ) < 1e-8f) continue;

                glm::vec3 a = glm::normalize( vA );
                glm::vec3 b = glm::normalize( vB );

                float current = glm::degrees( acos( glm::clamp( glm::dot( a, b ), -1.0f, 1.0f ) ) );

                // Pairwise angle targets for AX5/AX6 (others use a single target)
                float ideal = getIdealBondAngle( atom );
                if (totalDomains == 5 || totalDomains == 6)
                {
                    if (current > 150.0f)      ideal = 180.0f; // axial–axial
                    else if (current > 105.0f) ideal = 120.0f; // equatorial–equatorial
                    else                       ideal = 90.0f;  // axial–equatorial (or octahedral equivalents)
                }

                float delta = ideal - current; // + means "increase angle"
                if (fabsf( delta ) <= 0.5f) continue;

                glm::vec3 axis = glm::cross( a, b );
                float axisLen2 = glm::dot( axis, axis );
                if (axisLen2 < 1e-10f) continue;
                axis = glm::normalize( axis );

                // dt-scaled, clamped step; sign chosen so +delta expands angle
                float step = glm::clamp( glm::radians( delta ) * k * dt, -maxStep, +maxStep );

                auto rotateHalf = [&]( Atom *nb, float signedHalf ) {
                    if (!nb || nb->fixed) return;
                    glm::vec3 fromC = nb->position - atom.position;
                    float r = glm::length( fromC );
                    if (r < 1e-6f) return;

                    glm::vec3 dir = fromC / r;
                    // Opposite rotations around the same axis change the pair angle.
                    // To increase the angle when delta>0, rotate A by -step/2 and B by +step/2.
                    glm::mat4 R = glm::rotate( glm::mat4( 1.0f ), signedHalf, axis );
                    glm::vec3 newDir = glm::normalize( glm::vec3( R * glm::vec4( dir, 0 ) ) );
                    nb->position = atom.position + newDir * r;
                    nb->velocity *= 0.90f; // mild damping prevents ping-pong
                    };

                rotateHalf( A, -0.5f * step );
                rotateHalf( B, +0.5f * step );

                latestCorrectionStrength = std::max( latestCorrectionStrength, fabsf( delta ) );
            }
        }
    }
}

struct Dir
{
    glm::vec3 v; float r; int idx; bool movable;
};


static void addVirtualLonePairs( const Atom &C, std::vector<Dir> &out ) {
    static const glm::vec3 TET[ 4 ] = {                       // tetra corners
        glm::normalize( glm::vec3( 1,  1,  1 ) ),
        glm::normalize( glm::vec3( -1, -1,  1 ) ),
        glm::normalize( glm::vec3( 1, -1, -1 ) ),
        glm::normalize( glm::vec3( -1,  1, -1 ) ) };

    const int need = C.lonePairs;
    if (need == 0) return;

    std::vector<glm::vec3> cand( TET, TET + 4 );
    for (Atom *nb : C.bondedAtoms)           // mark used corners
    {
        glm::vec3 v = glm::normalize( nb->position - C.position );
        auto best = std::max_element( cand.begin(), cand.end(),
            [&]( auto &a, auto &b ) { return glm::dot( v, a ) > glm::dot( v, b ); } );
        cand.erase( best );
    }
    for (int i = 0; i < need && i < (int)cand.size(); ++i)
        out.push_back( { cand[ i ], 1.0f, -1, false } );
}


void AtomSystem::applyVSEPRAngleFast( float dt ) {
    const float kRepBase = 6.0f;
    const float maxStep = glm::radians( 6.0f );
    const float velocityDamp = 0.90f;

    auto isHeavy = []( const Atom *a ) {
        return a && a->type != "H" && a->type != "LP";
        };

    std::vector<glm::vec3> torque( atoms.size(), glm::vec3( 0 ) );

    for (Atom &C : atoms)
    {
       
        std::vector<Dir> dom;

        for (Atom *B : C.bondedAtoms)
        {
            if (!isHeavy( B )) continue;                   
            glm::vec3 d = B->position - C.position;
            float r = glm::length( d ); if (r < 1e-4f) r = 1e-4f;
            int idx = int( &B[ 0 ] - &atoms[ 0 ] );
            dom.push_back( { d / r, r, idx, !B->fixed } );
        }

        addVirtualLonePairs( C,  dom ); // or copy the helper’s push_back logic

        if (dom.size() < 2) continue;

        // Scale repulsion by domain count so big coordinations don’t blow up
        const float kRep = kRepBase / float( std::max<int>( 1, (int)dom.size() - 1 ) );

        for (int i = 0; i < (int)dom.size(); ++i)
        {
            for (int j = i + 1; j < (int)dom.size(); ++j)
            {
                glm::vec3 diff = dom[ i ].v - dom[ j ].v;
                float d2 = glm::dot( diff, diff ) + 1e-4f;
                glm::vec3 f = kRep * diff / (d2 * std::sqrt( d2 ));  // inverse-cube, softened

                if (dom[ i ].movable) torque[ dom[ i ].idx ] += f;
                if (dom[ j ].movable) torque[ dom[ j ].idx ] -= f;
            }
        }
    }

    // Choose a stable pivot for B: prefer heavy neighbor with highest bond order; avoid H pivots
    auto choosePivotFor = [&]( Atom &B ) -> Atom *{
        Atom *best = nullptr; int bestOrder = -1;
        for (Atom *N : B.bondedAtoms)
        {
            if (!isHeavy( N )) continue;                    
            int order = 0;
            for (const Bond &bd : bonds)
            {
                bool match = (bd.atomA == &B && bd.atomB == N) || (bd.atomB == &B && bd.atomA == N);
                if (match)
                {
                    order = bd.bondOrder(); break;
                }
            }
            if (order > bestOrder)
            {
                bestOrder = order; best = N;
            }
        }
        if (!best && !B.bondedAtoms.empty()) best = B.bondedAtoms.front();
        return best;
        };

    // Integrate: rotate each movable atom around its pivot using only tangential torque
    for (size_t idx = 0; idx < atoms.size(); ++idx)
    {
        Atom &B = atoms[ idx ];
        if (B.fixed) continue;

        glm::vec3 T = torque[ idx ];
        if (glm::length2( T ) < 1e-10f) continue;

        Atom *C = choosePivotFor( B );
        if (!C) continue;

        glm::vec3 dir = B.position - C->position;
        float r = glm::length( dir ); if (r < 1e-6f) continue;
        dir /= r;

        glm::vec3 tang = T - glm::dot( T, dir ) * dir;             // strip radial component
        float ang = glm::clamp( glm::length( tang ) * dt, 0.0f, maxStep );
        if (ang < 1e-5f) continue;

        glm::vec3 axis = glm::cross( dir, tang );
        float axisLen2 = glm::dot( axis, axis );
        if (axisLen2 < 1e-12f) continue;
        axis = glm::normalize( axis );

        glm::mat4 R = glm::rotate( glm::mat4( 1.0f ), ang, axis );
        glm::vec3 newDir = glm::normalize( glm::vec3( R * glm::vec4( dir, 0 ) ) );
        B.position = C->position + newDir * r;

        B.velocity *= velocityDamp;
    }
}


static inline bool angleInWindow( float deg, float minDeg = 15.0f, float maxDeg = 181.0f ) {
    return deg >= minDeg && deg <= maxDeg;
}

static inline bool sepOK( const glm::vec3 &a, const glm::vec3 &b, float minSin = 0.26f ) {
    return glm::length( glm::cross( a, b ) ) >= minSin;
}

bool isAngleVertex( const Atom &C ) {
    std::vector<glm::vec3> dirs;
    dirs.reserve( C.bondedAtoms.size() );
    for (Atom *n : C.bondedAtoms)
    {
        glm::vec3 d = n->position - C.position;
        float L2 = glm::dot( d, d );
        if (L2 > 1e-8f) dirs.push_back( d / glm::sqrt( L2 ) );
    }
    if (dirs.size() < 2) return false;

    for (size_t i = 0; i < dirs.size(); ++i)
    {
        for (size_t j = i + 1; j < dirs.size(); ++j)
        {
            float c = glm::clamp( glm::dot( dirs[ i ], dirs[ j ] ), -1.0f, 1.0f );
            float deg = glm::degrees( std::acos( c ) );
            if (deg > 1e-4f && deg < 180.0f - 1e-4f) return true;
            if (fabsf( deg ) <= 1e-4f || fabsf( deg - 180.0f ) <= 1e-4f) return true; // explicit 0°/180° ok
        }
    }
    return false;
}


void AtomSystem::renderBondAngles( int windowWidth, int windowHeight, const Camera &camera, const Atom *hovered ) {
    if (!hovered) return;

    const Atom *center = nullptr;
    for (const Atom &a : atoms) if (&a == hovered)
    {
        center = &a; break;
    }
    if (!center) return;

    if (!isAngleVertex( *center )) return;  

    // Collect normalized neighbor directions
    std::vector<Atom *> nbrs( center->bondedAtoms.begin(), center->bondedAtoms.end() );
    if (nbrs.size() < 2) return;

    auto safeNorm = []( const glm::vec3 &v ) {
        float L2 = glm::dot( v, v ); return (L2 > 1e-8f) ? v / glm::sqrt( L2 ) : glm::vec3( 0 );
        };

    glm::mat4 proj = glm::perspective( glm::radians( camera.Zoom ),
        float( windowWidth ) / float( windowHeight ),
        0.1f, 100.0f );
    glm::mat4 view = camera.GetViewMatrix();
    glm::vec4 viewport( 0.f, 0.f, float( windowWidth ), float( windowHeight ) );

    float bestDeg = 1e9f;
    Atom *Astar = nullptr, *Bstar = nullptr;
    glm::vec3 vA{}, vB{};
    for (size_t i = 0; i < nbrs.size(); ++i)
    {
        for (size_t j = i + 1; j < nbrs.size(); ++j)
        {
            glm::vec3 a = safeNorm( nbrs[ i ]->position - center->position );
            glm::vec3 b = safeNorm( nbrs[ j ]->position - center->position );
            float c = glm::clamp( glm::dot( a, b ), -1.0f, 1.0f );
            float deg = glm::degrees( std::acos( c ) );
            if (deg < bestDeg)
            {
                bestDeg = deg; Astar = nbrs[ i ]; Bstar = nbrs[ j ]; vA = a; vB = b;
            }
        }
    }
    if (!Astar || !Bstar) return;

    float ideal = idealAnglePair( *center, vA, vB );

    glm::vec3 bis = safeNorm( vA + vB );
    if (glm::length2( bis ) < 1e-8f)
    {
        glm::vec3 n = safeNorm( glm::cross( vA, vB ) );
        glm::vec3 alt = safeNorm( glm::cross( n, vA ) );
        bis = (glm::length2( alt ) > 1e-8f) ? alt : vA;
    }

    glm::vec3 labelWorld = center->position + bis * 0.35f;
    glm::vec3 win = glm::project( labelWorld, view, proj, viewport );
    if (win.z < 0.0f || win.z > 1.0f) return;

    char buf[ 96 ];
    std::snprintf( buf, sizeof( buf ), "[ANGLE] %.0f | (ideal %.0f)", bestDeg, ideal );
    textRenderer.DrawScreenText( buf, win.x, windowHeight - win.y, windowWidth, windowHeight );
}





void AtomSystem::updateLonePairs() {
    auto &pt = PeriodicTable::Instance();
    bool anyChange = false;

    for (Atom &a : atoms)
    {
        // count bonding electrons
        int bondedPairs = 0;
        for (const Bond &b : bonds)
            if (b.atomA == &a || b.atomB == &a)
                bondedPairs += b.bondOrder();
        int bondedElectrons = bondedPairs * 2;

        // decide valence target
        const Element &e = pt.Get( a.type );
        int an = e.atomicNumber;
        int valence = e.valenceElectrons;
        int desired;
        if (an == 1)  desired = 2;  // H duet
        else if (an == 4)  desired = 4;  // Be incomplete
        else if (an == 5 || an == 13 || an == 31 || an == 49)
            desired = 6;  // B, Al, Ga, In
        else if (an > 10)  desired = 12; // period-3+ expanded
        else desired = 8;  // C, N, O, F, Ne, etc.

        int newLP = std::max( 0, desired - bondedElectrons ) / 2;

        int usedElectrons = bondedElectrons + newLP * 2;
        int remaining = valence - usedElectrons;
        if (remaining < 0)  remaining = 0;          // overshoot 
        bool newRadical = (remaining == 1);     // exactly one
        if (newLP != a.lonePairs ||
            newRadical != a.hasRadical)
        {
            a.lonePairs = newLP;
            a.hasRadical = newRadical;
            anyChange = true;
        }
    }
    if (anyChange) setDirtyLonePairs();
}
float AtomSystem::computeDipole() {
    netDipole = glm::vec3(0.0f);

    auto& pt = PeriodicTable::Instance();

    for (auto& bond : bonds) {
        // direction only
        glm::vec3 disp = bond.atomB->position - bond.atomA->position;
        glm::vec3 dir = glm::normalize(disp);

        // electronegativities
        float enA = pt.Get(bond.atomA->type).electronegativity;
        float enB = pt.Get(bond.atomB->type).electronegativity;
        float deltaEN = fabs(enA - enB);

        // fractional charge (0..1)
        float q = deltaEN / (enA + enB);

        netDipole += dir * q;
    }

    // DIP scale 
    netDipole *= 0.04;

    // verify magnitude 
    dipoleMag = glm::length(netDipole);
    if (dipoleMag < 1e-3f)
    {
        netDipole = glm::vec3( 0.0f );
        isPolar = false;
    }
    else
    {
        isPolar = true;
    }
    return dipoleMag;
}


void AtomSystem::spawnAtom( const std::string & sym, const glm::vec3 & pos )   // NEW
 {
    // Specifc place 
    if (atoms.size() >= maxAtoms) return;
    atoms.emplace_back( sym, pos, 1.f );
    setDirtyLonePairs();
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
    auto &pt = PeriodicTable::Instance();
    int bonded = (int)atom.bondedAtoms.size();
    int lp = atom.lonePairs;
    int radical = atom.hasRadical ? 1 : 0;
    int total = bonded + lp + radical;

    switch (total)
    {
    case 0: case 1:
        return 0.0f;
    case 2:
        return 180.0f;
    case 3:
        return 120.0f;
    case 4:
        return (lp == 0 ? 109.5f : lp == 1 ? 107.0f : 104.5f);
    case 5:
        // pentagonal bipyramid 
        return 90.0f;
    case 6:
        // octahedral 
        return 90.0f;
    case 7:
        // pentagonal bipyramidal 
        return 72.0f;
    case 8:
        // square antiprism 
        return 70.5f;
    default:
        // fallback
        return 90.0f;
    }
}

std::string AtomSystem::determineGeometry( const Atom &a ) const {
    int bonded = (int)a.bondedAtoms.size();
    int lp = a.lonePairs;
    int radical = a.hasRadical ? 1 : 0;
    int total = bonded + lp + radical;

    switch (total)
    {
    case 0:  return "No electron domains";
    case 1:  return "Single-domain";
    case 2:  return "Linear";
    case 3:  return (lp == 0 ? "Trigonal planar" : "Bent");
    case 4:
        if (lp == 0) return "Tetrahedral";
        else if (lp == 1) return "Trigonal pyramidal";
        else             return "Bent";
    case 5:
        if (lp == 0) return "Trigonal bipyramidal";
        else if (lp == 1) return "Seesaw (disphenoidal)";
        else if (lp == 2) return "T-shaped";
        else             return "Linear";
    case 6:
        if (lp == 0) return "Octahedral";
        else if (lp == 1) return "Square pyramidal";
        else if (lp == 2) return "Square planar";
        else if (lp == 3) return "T-shaped";
        else             return "Linear";
    case 7:
        return "Pentagonal bipyramidal";
    case 8:
        return "Square antiprismatic";
    default:
        return "Coordination " + std::to_string( total );
    }
}

void AtomSystem::updateFormalCharges() {
    for (Atom &atom : atoms)
    {
        // Formal charges = valence - nonbonding electrons - 1/2 * bonding electrons
        int bondPairs = 0;
        for (Bond& b : bonds) {
            if (b.atomA == &atom || b.atomB == &atom) {
                bondPairs += b.bondOrder();   // 1, 2 or 3
                // Account for single, double, or triple bonds 
            }
        }

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

    for (const auto& s : symbols) {
        spawnAtom(s);
    }

    for (const auto &[a, b, order] : bondList)
    {
        BondType t = BondType::SINGLE;
        if (order == 2) t = BondType::DOUBLE;
        else if (order == 3) t = BondType::TRIPLE;



        createBond( a, b, t );
    }


    // Electrion stability system 
    glm::vec3 com( 0.0f );
    float     totalM = 0.0f;
    for (const Atom &a : atoms)
    {
        com += a.mass * a.position; totalM += a.mass;
    }

    if (totalM > 0.0f) {
        com /= totalM;
    }

    for (Atom& a : atoms) {
        a.position -= com;
    }

    glm::vec3 P( 0.0f );                      // net linear momentum
    for (const Atom& a : atoms) {
        P += a.mass * a.velocity;
    }

    glm::vec3 vCM = (totalM > 0.0f) ? P / totalM : glm::vec3( 0.0f );
    for (Atom& a : atoms) {
        a.velocity -= vCM;
    }

    // kill residual angular velocity for a calm start
    for (Atom& a : atoms) {
        a.velocity = glm::vec3(0.0f);
    }
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

void AtomSystem::computeLonePairDots() {
   // if (!lonePairsDirty) return;
  //  lonePairsDirty = false;

    lonePairDots.clear();

    static const glm::vec3 tetra[ 4 ] =
    {
        glm::normalize( glm::vec3( 1, 1, 1 ) ),
        glm::normalize( glm::vec3( -1,-1, 1 ) ),
        glm::normalize( glm::vec3( 1,-1,-1 ) ),
        glm::normalize( glm::vec3( -1, 1,-1 ) )
    };

    for (Atom &a : atoms)
    {
        if (a.lonePairs == 0) continue;

        std::vector<glm::vec3> dirs;

        int groups = (int)a.bondedAtoms.size() + a.lonePairs;
        if (groups <= 3)                             // planar / linear
        {
            for (int i = 0; i < a.lonePairs; ++i)
            {
                float ang = i * glm::two_pi<float>() / a.lonePairs;
                dirs.emplace_back( cosf( ang ), sinf( ang ), 0 );
            }
        }
        else                                       // tetrahedral slots
        {
            bool used[ 4 ] = { false,false,false,false };
            for (Atom *nb : a.bondedAtoms)
            {
                glm::vec3 v = glm::normalize( nb->position - a.position );
                float best = -2; int idx = -1;
                for (int j = 0; j < 4; ++j)
                {
                    float d = glm::dot( v, tetra[ j ] );
                    if (d > best)
                    {
                        best = d; idx = j;
                    }
                }
                used[ idx ] = true;
            }
            for (int j = 0; j < 4 && dirs.size() < a.lonePairs; ++j)
                if (!used[ j ]) dirs.push_back( tetra[ j ] );
        }


        const float baseR = a.radius;          // on surface
        const float spread = a.radius * 0.9f;     // pair separation

        for (glm::vec3 d : dirs)
        {
            d = glm::normalize( d );
            glm::vec3 ref = fabs( d.y ) < .99f ? glm::vec3( 0, 1, 0 ) : glm::vec3( 1, 0, 0 );
            glm::vec3 perp = glm::normalize( glm::cross( d, ref ) );

            glm::vec3 centre = a.position + d * baseR;

            for (int sgn : { +1, -1 })
            {
                const glm::vec3 pos = centre + perp * (spread * static_cast<float>(sgn));
                Atom dot( "LP", pos, 0.1f );
                dot.radius = 0.08f;
                dot.color = glm::vec3( 0.7f, 0.7f, 1.0f );
                lonePairDots.push_back( dot );
            }

        }
    }
}

void AtomSystem::drawTooltip( const Atom &at, int w, int h ) const {
    const Element &e = PeriodicTable::Instance().Get( at.type );

    int ns = 0, nd = 0, nt = 0;
	int piBonds = 0, sigmaBonds = 0;
    for (const Bond &b : bonds)
        if (b.atomA == &at || b.atomB == &at)
        {
            if (b.type == BondType::SINGLE)
            {
                ++ns;
				++sigmaBonds;
            }
            if (b.type == BondType::DOUBLE)
            {
                ++nd;
				++sigmaBonds;
				++piBonds;
            }
            if (b.type == BondType::TRIPLE)
            {
                ++nt;
				++sigmaBonds;
				piBonds += 2;
            }
        }

    std::ostringstream os;
    os << e.symbol << " (" << e.atomicNumber << ")  " << e.atomicMass << " u\n"
        << "Electronegativity: " << e.electronegativity << "\n"
        << "Dipole: " << glm::length( at.polarityDir ) << "\n"
        << "Lone Pairs: " << at.lonePairs << "\n"
        << "Bonds: " << at.bondedAtoms.size() << "\n";
    if (ns) os << "  Single: " << ns << "\n";
    if (nd) os << "  Double: " << nd << "\n";
    if (nt) os << "  Triple: " << nt << "\n";
	if (sigmaBonds) os << "  Sigma: " << sigmaBonds << "\n";
	if (piBonds) os << "  Pi: " << piBonds << "\n";

    float x = 10, y = 410, dy = 20;  int i = 0;  std::string ln;
    std::istringstream iss( os.str() );
    while (std::getline(iss, ln)) {
        textRenderer.DrawScreenText(ln, x, y + i * dy, w, h), ++i;
    }
}

void AtomSystem::clear() {
    atoms.clear();
    lonePairAtoms.clear();
    lonePairDots.clear();

    bonds.clear();

    singleBonds = doubleBonds = tripleBonds = 0;
    sigmaBonds = piBonds = 0;

    isPolar = false;
    latestCorrectionStrength = 0.f;
    netDipole = glm::vec3( 0.f );
    dipoleMag = 0.f;

    firstCentralGeometry.clear();
    lonePairsDirty = true;
}

glm::vec3 AtomSystem::getCenter() const {
    if (atoms.empty()) return glm::vec3( 0.0f );
    glm::vec3 sum( 0.0f );
    for (const Atom &a : atoms)
        sum += a.position;
    return sum / float( atoms.size() );
}

void AtomSystem::build( const std::vector<std::string> &symbols ) {
    Resonance::Generator gen( symbols );
    std::vector<std::tuple<int, int, int>> bonds;
    try
    {
        bonds = gen.bestStructure();
    }
    catch (const std::exception &e)
    {
        std::cerr << "[Resonance] " << e.what() << "\n";
        bonds.clear(); // atoms-only fallback
    }
    build( symbols, bonds );
}

float AtomSystem::idealAnglePair( const Atom &C, const glm::vec3 &vA, const glm::vec3 &vB ) {
    float ang = glm::degrees( acos( glm::clamp( glm::dot( vA, vB ), -1.f, 1.f ) ) );
    int total = int( C.bondedAtoms.size() ) + C.lonePairs + (C.hasRadical ? 1 : 0);
    if (total == 5 || total == 6)
    {
        if (ang > 150.f) return 180.f;
        if (ang > 105.f) return 120.f;
        return 90.f;
    }
    return getIdealBondAngle( C );
}

bool AtomSystem::hasHyrdogenBonds() {
    // Hydrogen bonding: look for N, O, or F bonded to H
    for (const Bond& bond : bonds) {
        Atom* a = bond.atomA;
        Atom* b = bond.atomB;
        // Check if one atom is hydrogen and the other is N, O, or F
        if ((a->type == "H" && (b->type == "N" || b->type == "O" || b->type == "F")) ||
            (b->type == "H" && (a->type == "N" || a->type == "O" || a->type == "F"))) {
            return true;
        }
    }
    return false;
}