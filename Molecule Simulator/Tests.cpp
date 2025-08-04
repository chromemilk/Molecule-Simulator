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
	int passes = 0, fails = 0;

    struct TestCase
    {
        std::string name;
        std::string formula;
        std::function<bool( const AtomSystem & )> validate;
        std::string expected;
    };

    std::cout << "Init Unit Tests For System: RESONANCE" << std::endl;

    const std::vector<TestCase> tests{
        {"Methanol (CH3OH)", "CH3OH",   
         []( const AtomSystem &sys )
         {
             const auto &bonds = sys.getBonds();
             if (bonds.size() != 5) return false;
             for (const auto &b : bonds)
                 if (b.type != BondType::SINGLE) return false;
             return true;
         },
         "5 single bonds"},

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
        if (pass) passes++;
		else fails++;
        std::cout << "Actual: " << actual << '\n';
        std::cout << "Result: " << (pass ? "PASS" : "FAIL") << "\n\n";
    }
    std::cout << "Pass Ratio: " << std::fixed << std::setprecision( 2 )
		<< (100.0f * passes / (passes + fails)) << "%\n";
    
}
