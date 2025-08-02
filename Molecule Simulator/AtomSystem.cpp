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

extern Atom *bondFirst;
extern Atom *breakFirst;   


AtomSystem::AtomSystem( unsigned int maxAtoms, TextRenderer &tr )
    : maxAtoms( maxAtoms ), textRenderer( tr ) {
}

void AtomSystem::update( float dt ) {
    // Apply forces based on bonds/bond type
    for (Bond& b : bonds) {
        b.applyForce();
    }

    // Use valence shel electron repulsion theory to find and set bond angles dynamically
    applyVSEPRForces( dt );
    
    if (betterStabilization)
    {
        const float drag = 0.98f;   
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


void AtomSystem::render( int w, int h ) {
  

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


    for (Bond& b : bonds) {
        // Render the bonds 
        b.render(w, h);
    }

    for (Atom& a : atoms) {
        // Draw the atoms, and highlight if conditions are met
        Renderer::DrawAtom(a, w, h, (&a == selectedAtom) ||
            (&a == hoveredAtom) ||
            (&a == bondFirst) ||
            (&a == breakFirst));
    }

    // Lone pair transparency 
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glDepthMask( GL_FALSE );

    // Draw lone pair dots 
    for (Atom& dot : lonePairDots) {
        Renderer::DrawAtom(dot, w, h, &dot == hoveredAtom);
    }

    // Disable transparency
    glDisable( GL_BLEND );
    glDepthMask( GL_TRUE );

    // Show calculated formal charges
    for (Atom& a : atoms) {
        textRenderer.DrawText(a.type + StringUtils::chargeString(a.formalCharge),
            a.position, w, h);
    }

    // Render the computed bond angles
    renderBondAngles( w, h );

    // Render computed geometry
    if (!firstCentralGeometry.empty()) {
        textRenderer.DrawScreenText("Geometry: " + firstCentralGeometry, 10, 90, w, h);
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
    auto &pt = PeriodicTable::Instance();
    constexpr float ionicThresh = 2.0f;    
    constexpr float repelStrength = 30.0f;   // lowered from 100
    constexpr float maxCorr = 0.1f;    // clamp correction magnitude
    constexpr int   PASSES = 3;       // micro-iterations per frame

    for (int pass = 0; pass < PASSES; ++pass)
    {
        for (auto &center : atoms)
        {
            size_t n = center.bondedAtoms.size();
            if (n < 2) continue;
            float enC = pt.Get( center.type ).electronegativity;

            for (size_t i = 0; i < n; ++i)
            {
                Atom *A = center.bondedAtoms[ i ];
                float enA = pt.Get( A->type ).electronegativity;
                if (fabs( enC - enA ) > ionicThresh) continue;

                for (size_t j = i + 1; j < n; ++j)
                {
                    Atom *B = center.bondedAtoms[ j ];
                    float enB = pt.Get( B->type ).electronegativity;
                    if (fabs( enC - enB ) > ionicThresh) continue;

                    // direct pairwise repulsion
                    glm::vec3 delta = A->position - B->position;
                    float dist = glm::length( delta );
                    if (dist < 1e-4f) continue;
                    glm::vec3 dir = delta / dist;

                    // compute and clamp
                    float mag = repelStrength * dt / (dist * dist);
                    mag = glm::clamp( mag, 0.0f, maxCorr );
                    glm::vec3 correction = dir * mag;

                    if (!A->fixed) A->velocity += correction;
                    if (!B->fixed) B->velocity -= correction;

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
                // Angle vertex
                Atom *neighborA = atom.bondedAtoms[ i ];
                Atom *neighborB = atom.bondedAtoms[ j ];

                glm::vec3 vecA = neighborA->position - atom.position;
                glm::vec3 vecB = neighborB->position - atom.position;

                float angle = glm::degrees( acos( glm::clamp( glm::dot( glm::normalize( vecA ), glm::normalize( vecB ) ), -1.0f, 1.0f ) ) );

                glm::vec3 labelPos = (atom.position + neighborA->position + neighborB->position) / 3.0f;

                float ideal = getIdealBondAngle( atom );
                // Show actual vs ideal angle
                std::string label = "Real Angle: " + std::to_string( angle ) + " (Ideal: " + std::to_string( ideal  ) + ")";

                textRenderer.DrawText( label, labelPos, windowWidth, windowHeight );
            }
        }
    }
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