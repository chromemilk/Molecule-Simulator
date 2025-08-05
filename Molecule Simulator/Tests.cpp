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
        std::string                             name;
        std::string                             formula;
        std::function<bool( const AtomSystem & )>  validate;
        std::string                             expected;
    };

    std::cout << "Init Unit Tests For System: RESONANCE\n"
        << "-------------------------------------\n";

    const std::vector<TestCase> tests{
        {"Methanol (CH3OH)", "CH3OH",
            []( const AtomSystem &sys )
            {
                if (sys.getBonds().size() != 5) return false;
                for (auto &b : sys.getBonds())
                    if (b.type != BondType::SINGLE) return false;
                return true;
            },
            "5 single bonds"},

        {"Nitrate Ion (NO3-)", "NO3-",
            []( const AtomSystem &sys )
            {
                int dbl = 0, sing = 0;
                for (auto &b : sys.getBonds())
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
        for (auto &b : sys.getBonds())
            if (b.type == BondType::DOUBLE) ++dbl;
        return dbl == 2;
    },
    "2 double bonds"},

{"Ozone (O3)", "O3",
    []( const AtomSystem &sys )
    {
        int dbl = 0, sing = 0;
        for (auto &b : sys.getBonds())
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

{"Benzene (C6H6)", "C6H6",
    []( const AtomSystem &sys )
    {
        int dbl = 0, sing = 0;
        for (auto &b : sys.getBonds())
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
        for (auto &b : sys.getBonds())
            if (b.atomA->type == "S" || b.atomB->type == "S") ++cnt;
        return cnt == 6;
    },
    "6 S-F bonds"},
    };

    for (const auto &test : tests)
    {
        std::cout << "---- " << test.name << " ----\n";
        std::cout << "Input: " << test.formula << '\n';

        bool   pass = false;
        std::string  actual = "RUNTIME EXCEPTION";

        try
        {
            ParsedFormula pf = parseFormulaFull( test.formula );

            Resonance::Generator gen( pf.atoms, pf.charge );
            const auto bonds = gen.bestStructure();

            AtomSystem sys( 100, textRenderer );
            sys.build( pf.atoms, bonds );

            pass = test.validate( sys );

            int singles = 0, doubles = 0, triples = 0;
            for (auto &b : sys.getBonds())
            {
                if (b.type == BondType::SINGLE)  ++singles;
                if (b.type == BondType::DOUBLE)  ++doubles;
                if (b.type == BondType::TRIPLE)  ++triples;
            }
            actual = std::to_string( doubles ) + " double, " +
                std::to_string( singles ) + " single, " +
                std::to_string( triples ) + " triple bonds";
        }
        catch (const std::exception &e)
        {
            std::cout << "Exception: " << e.what() << '\n';
        }

        std::cout << "Expected: " << test.expected << '\n'
            << "Actual:   " << actual << '\n'
            << "Result:   " << (pass ? "PASS" : "FAIL") << "\n\n";

        (pass ? ++passes : ++fails);
    }

    double ratio = passes + fails ? 100.0 * passes / (passes + fails) : 0.0;
    std::cout << "Pass Ratio: " << std::fixed << std::setprecision( 2 )
        << ratio << "%\n"
        << "-------------------------------------\n"
        << "Verdict: " << (ratio > 85.0 ? "PASS" : "FAIL") << std::endl;
}
