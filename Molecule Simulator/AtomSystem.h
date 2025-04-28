#pragma once
#include <vector>
#include <unordered_map>
#include "Atom.h"
#include "Bond.h"
#include "TextRenderer.h"

class AtomSystem
{
public:
    AtomSystem( unsigned maxAtoms, TextRenderer &tr );

    void update( float dt );
    void render( int w, int h );

    void spawnAtom( const std::string &type );
    void addAtom( const Atom &atom );
    void createBond( int ia, int ib, BondType t = BondType::SINGLE );

    void updateLonePairs();  
    void updateFormalCharges();
    float computeDipole();   

    void updatePolarities();

    void build( const std::vector<std::string> &symbols, const std::vector<std::tuple<int, int, int>> &bonds );

    bool isPolar = false;
    float latestCorrectionStrength = 0.0f;

private:
    void applyVSEPRForces( float dt );
    void renderBondAngles( int w, int h );
    float getIdealBondAngle( const Atom &a );
    void  computeLonePairPositions( std::unordered_map<Atom *, std::vector<glm::vec3>> & );
    std::string determineGeometry( const Atom & ) const;


    unsigned                maxAtoms{};
    std::vector<Atom>       atoms;
    std::vector<Bond>       bonds;
    TextRenderer &textRenderer;
    std::string             firstCentralGeometry{};

    glm::vec3 netDipole{ 0.0f };   
    float     dipoleMag{ 0.0f };   


};
