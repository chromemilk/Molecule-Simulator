#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <functional>

#include "Parser.h"
#include "UI.h"
#include "InputHandler.h"
#include "Tests.h"
#include "AtomSystem.h"
#include "Resonance.h"

void runMoleculeTests( [[maybe_unused]] AtomSystem & /*unused*/, TextRenderer &textRenderer ) {
    struct TestCase
    {
        std::string name;
        std::string formula;
        std::function<bool( const AtomSystem & )> validate;
        std::string expected;
    };

    const std::vector<TestCase> tests{
        {"Methanol (CH3OH)", "CH3OH",   
         []( const AtomSystem &sys )
         {
             const auto &bonds = sys.getBonds();
             if (bonds.size() != 4) return false;
             for (const auto &b : bonds)
                 if (b.type != BondType::SINGLE) return false;
             return true;
         },
         "4 single bonds"},

        {"Nitrate Ion (NO3-)", "NO3-",
         []( const AtomSystem &sys )
         {
             int dbl = 0, sing = 0;
             for (const auto &b : sys.getBonds())
             {
                 if (b.type == BondType::DOUBLE) ++dbl;
                 if (b.type == BondType::SINGLE) ++sing;
             }
             return dbl == 1 && sing == 2;
         },
         "1 double & 2 single bonds"},

        {"Carbon Dioxide (CO2)", "CO2",
         []( const AtomSystem &sys )
         {
             int dbl = 0;
             for (const auto &b : sys.getBonds())
                 if (b.type == BondType::DOUBLE) ++dbl;
             return dbl == 2;
         },
         "2 double bonds"},

        {"Ozone (O3)", "O3",
         []( const AtomSystem &sys )
         {
             int dbl = 0, sing = 0;
             for (const auto &b : sys.getBonds())
             {
                 if (b.type == BondType::DOUBLE) ++dbl;
                 if (b.type == BondType::SINGLE) ++sing;
             }
             return dbl == 1 && sing == 1;
         },
         "1 double & 1 single bond"},

        {"Cyanide Ion (CN-)", "CN-",
         []( const AtomSystem &sys )
         {
             const auto &bonds = sys.getBonds();
             return bonds.size() == 1 && bonds[ 0 ].type == BondType::TRIPLE;
         },
         "1 triple bond"},
		// C6H6 is a special case with resonance structures, fix this
        {"Benzene (C6H6)", "C6H6",
         []( const AtomSystem &sys )
         {
             int dbl = 0, sing = 0;
             for (const auto &b : sys.getBonds())
             {
                 if (b.type == BondType::DOUBLE) ++dbl;
                 if (b.type == BondType::SINGLE) ++sing;
             }
             return dbl == 3 && sing == 9;
         },
         "3 double & 9 single bonds"},

        {"Sulfur Hexafluoride (SF6)", "SF6",
         []( const AtomSystem &sys )
         {
             int cnt = 0;
             for (const auto &b : sys.getBonds())
             {
                 if (b.atomA->type == "S" || b.atomB->type == "S") ++cnt;
             }
             return cnt == 6;
         },
         "6 S-F bonds"},

        {"Hydrated Copper Sulfate (CuSO4.5H2O)", "CuSO4.5H2O",
         []( const AtomSystem &sys )
         {
             std::map<std::string, int> cnt;
             for (const auto &a : sys.getAtoms()) ++cnt[ a.type ];
             return cnt[ "Cu" ] == 1 && cnt[ "S" ] == 1 && cnt[ "O" ] == 9 && cnt[ "H" ] == 10;
         },
         "composition CuH10O9S"},

        {"Magnesium Hydroxide (Mg(OH)2)", "Mg(OH)2",
         []( const AtomSystem &sys )
         {
             std::map<std::string, int> cnt;
             for (const auto &a : sys.getAtoms()) ++cnt[ a.type ];
             return cnt[ "Mg" ] == 1 && cnt[ "O" ] == 2 && cnt[ "H" ] == 2;
         },
         "composition Mg1O2H2"},

        {"Complex Ion (K4[ON(SO3)2]2)", "K4[ON(SO3)2]2",
         []( const AtomSystem &sys )
         {
             std::map<std::string, int> cnt;
             for (const auto &a : sys.getAtoms()) ++cnt[ a.type ];
             return cnt[ "K" ] == 4 && cnt[ "N" ] == 2 && cnt[ "O" ] == 14 && cnt[ "S" ] == 4;
         },
         "composition K4N2O14S4"}
    };

    for (const auto &test : tests)
    {
        std::cout << "---- " << test.name << " ----\n";
        std::cout << "Input: " << test.formula << '\n';

        const auto syms = parseFormula( test.formula );
        Resonance::Generator gen( syms );
        const auto bonds = gen.bestStructure();

        AtomSystem sys( 100, textRenderer );
        sys.build( syms, bonds );

        const bool pass = test.validate( sys );
        std::cout << "Expected: " << test.expected << '\n';

        std::string actual;
        if (test.expected.find( "composition" ) != std::string::npos)
        {
            std::map<std::string, int> cnt;
            for (const auto &a : sys.getAtoms()) ++cnt[ a.type ];
            for (const auto &p : cnt)
                actual += p.first + ':' + std::to_string( p.second ) + ' ';
        }
        else
        {
            int singles = 0, doubles = 0, triples = 0;
            for (const auto &b : sys.getBonds())
            {
                if (b.type == BondType::SINGLE) ++singles;
                if (b.type == BondType::DOUBLE) ++doubles;
                if (b.type == BondType::TRIPLE) ++triples;
            }
            actual = std::to_string( doubles ) + " double, " +
                std::to_string( singles ) + " single, " +
                std::to_string( triples ) + " triple bonds";
        }

        std::cout << "Actual: " << actual << '\n';
        std::cout << "Result: " << (pass ? "PASS" : "FAIL") << "\n\n";
    }
}
