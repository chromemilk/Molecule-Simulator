#pragma once

#include <vector>
#include <unordered_map>
#include <tuple>
#include <string>
#include "Atom.h"
#include "Bond.h"
#include "TextRenderer.h"
#include "Camera.h"

class AtomSystem
{
public:
    AtomSystem( unsigned maxAtoms, TextRenderer &tr );

    void update( float dt );
    void render( int w, int h, Atom *selected, Atom *hovered, Atom *bondFirst, Atom *breakFirst, const Camera &camera );

    std::vector<Bond> &getBonds();

    void setDirtyLonePairs();

    void spawnAtom( const std::string &type );
    void spawnAtom( const std::string &type, const glm::vec3 &p );
    void addAtom( const Atom &atom );
    void createBond( int ia, int ib, BondType t = BondType::SINGLE );
    glm::vec3 getCenter() const;

    void updateLonePairs();
    void updateFormalCharges();
    float computeDipole();

    void updatePolarities();

    void computeLonePairDots();

    void drawTooltip( const Atom &at, int w, int h ) const;

    void build( const std::vector<std::string> &symbols,
        const std::vector<std::tuple<int, int, int>> &bonds );

    bool isPolar = false;
    float latestCorrectionStrength = 0.f;
    int singleBonds = 0;
    int doubleBonds = 0;
    int tripleBonds = 0;
    int sigmaBonds = 0;
    int piBonds = 0;

    bool betterStabilization = false;

	bool followCamera = false; // follow camera position

    bool fastCorrection = false;

    const std::vector<Atom> &getAtoms() const {
        return atoms;
    }
    const std::vector<Atom> &getLonePairs() const;
 

    std::vector<Bond> bonds;

    std::vector<Atom> lonePairDots;
    bool lonePairsDirty = true;

    std::string firstCentralGeometry;

    void clear();



private:
    void applyVSEPRForces( float dt );
    void applyVSEPRAngleFast( float dt );
    void renderBondAngles( int w, int h, const Camera &camera );
    float getIdealBondAngle( const Atom &a );
    void  computeLonePairPositions( std::unordered_map<Atom *, std::vector<glm::vec3>> &out );
    std::string determineGeometry( const Atom & ) const;

    unsigned maxAtoms{ 0 };
    std::vector<Atom> atoms;
    std::vector<Atom> lonePairAtoms;      // persistent store of LP-dots
    TextRenderer &textRenderer;

    glm::vec3 netDipole{ 0.f };
    float dipoleMag{ 0.f };
};
