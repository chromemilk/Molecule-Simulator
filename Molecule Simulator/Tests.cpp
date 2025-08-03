#include "Tests.h"
#include "Parser.h"
#include "resonance.h"
#include <cassert>
#include <iostream>

void runMoleculeTests( AtomSystem &atoms, TextRenderer &textRenderer ) {
    auto build = [&]( const std::string &f ) {
        auto sym = parseFormula( f );
        Resonance::Generator gen( sym );
        auto bonds = gen.bestStructure();
        AtomSystem sys( 100, textRenderer );
        sys.build( sym, bonds );
        return sys;
        };
    {
        auto sys = build( "CH3OH" );
        assert( sys.getBonds().size() == 4 );
        for (auto &b : sys.getBonds()) assert( b.type == BondType::SINGLE );
    }
    std::cout << "[Tests] All passed!\n";
}
