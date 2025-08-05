#include <cassert>
#include <vector>
#include <string>
#include <tuple>
#include <stdexcept>

#include "../Molecule Simulator/Resonance.h"

int main() {
    using namespace Resonance;

    {
        Generator gen({"C", "O", "O"});
        auto bonds = gen.bestStructure();
        int dbl = 0;
        for (auto [i, j, o] : bonds)
            if (o == 2) ++dbl;
        assert(dbl == 2); // CO2 should have two double bonds
    }

    {
        bool thrown = false;
        try {
            Generator gen({"Xx"});
            gen.bestStructure();
        } catch (const std::invalid_argument &) {
            thrown = true;
        }
        assert(thrown); // unknown element should throw
    }

    {
        Generator gen({"H", "H"});
        auto bonds = gen.bestStructure();
        assert(bonds.size() == 1 && std::get<2>(bonds[0]) == 1); // H2 pair
    }

    {
        bool err = false;
        try {
            Generator gen({"H", "H", "H"});
            gen.bestStructure();
        } catch (const std::runtime_error &) {
            err = true;
        }
        assert(err); // odd number of hydrogens cannot form stable structure
    }

    {
        Generator gen({"S","F","F","F","F","F","F"});
        auto bonds = gen.bestStructure();
        int count = 0;
        for (auto [i,j,o] : bonds)
            if (i==0 || j==0) { assert(o==1); ++count; }
        assert(count == 6); // SF6 has six S-F single bonds
    }

    {
        Generator gen({"Xe","F","F","F","F"});
        auto bonds = gen.bestStructure();
        int count = 0;
        for (auto [i,j,o] : bonds)
            if (i==0 || j==0) { assert(o==1); ++count; }
        assert(count == 4); // XeF4 has four Xe-F single bonds
    }

    {
        Generator gen({"N+","H","H","H","H"});
        auto bonds = gen.bestStructure();
        int count = 0;
        for (auto [i,j,o] : bonds)
            if (i==0 || j==0) { assert(o==1); ++count; }
        assert(count == 4); // ammonium cation
    }

    return 0;
}
