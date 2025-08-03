#include "Resonance.h"
#include "PeriodicTable.h"
#include <algorithm>
#include <array>
#include <limits>
#include <numeric>
#include <unordered_map>
#include <stdexcept>

using std::vector;
using std::string;
using std::tuple;
using std::uint8_t;

namespace Resonance {


    static const std::unordered_map<string, uint8_t> kTypV = {
        // Hydrogen and common organic elements
        {"H",1},{"C",4},{"N",3},{"O",2},

        // Halogens
        {"F",1},{"Cl",3},{"Br",3},{"I",3},

        // Pnictogens (group 15)
        {"P",5},{"As",5},{"Sb",5},

        // Chalcogens (group 16)
        {"S",6},{"Se",6},{"Te",6},

        // Boron family
        {"B",3},{"Al",3},{"Ga",3},{"In",3},

        // Noble gases (allow hypervalent species)
        {"Xe",4},{"Kr",4},{"Ar",2}, // ArF2 possible
    };

    uint8_t Generator::typicalValence( std::string sym ) {
        int formalCharge = 0;
        if (!sym.empty() && (sym.back() == '+' || sym.back() == '-'))
        {
            char sign = sym.back(); sym.pop_back();
            int mag = 1;
            if (!sym.empty() && isdigit( sym.back() ))
            {
                mag = sym.back() - '0'; sym.pop_back();
            }
            formalCharge = (sign == '+' ? -mag : +mag);
        }

        auto it = kTypV.find( sym );
        if (it != kTypV.end()) return std::max( uint8_t( 1 ), uint8_t( it->second - formalCharge ) );

        const auto &pt = PeriodicTable::Instance();
        int ve = pt.Get( sym ).valenceElectrons - formalCharge;

        uint8_t val = std::clamp<uint8_t>( (8 - ve) / 2, 1, 4 );
        if (pt.Get( sym ).atomicNumber >= 15)            // period 3
            val = std::min<uint8_t>( 6, std::max<uint8_t>( val, 4 ) );  // up to d-expansion
        return val;
    }


    Generator::Generator(const std::vector<std::string>& atoms)
    {
        const int n = atoms.size();
        new2old.reserve(n);
        symbols.reserve(n);

        // heavy atoms first
        for (int i = 0;i < n;++i) if (atoms[i] != "H") {
            new2old.push_back(i); symbols.push_back(atoms[i]);
        }
        heavyCnt = symbols.size();

        // hydrogens after that
        for (int i = 0;i < n;++i) if (atoms[i] == "H") {
            new2old.push_back(i); symbols.push_back("H");
        }

        old2new.resize(n);
        for (int k = 0;k < n;++k) old2new[new2old[k]] = k;
    }


    vector<Bond> Generator::attachHydrogens( vector<uint8_t> &valenceLeft ) {
        const int totalH = symbols.size() - heavyCnt;
        vector<Bond> out;
        int hIndex = heavyCnt;                 // first H in symbols

        for (int h = 0; h < totalH; ++h, ++hIndex)
        {
            // Find max valence left but skip noble gases
            int i = -1;
            int maxVal = -1;
            for (int idx = 0; idx < heavyCnt; ++idx)
            {
                const string &sym = symbols[ idx ];
                // Skip noble gases for H attachment
                if (sym == "Xe" || sym == "Kr" || sym == "Ar")
                    continue;
                if (valenceLeft[ idx ] > maxVal)
                {
                    maxVal = valenceLeft[ idx ];
                    i = idx;
                }
            }

            if (i == -1 || maxVal == 0)
                throw std::runtime_error( "Ran out of valence while attaching H" );

            valenceLeft[ i ]--;
            out.emplace_back( i, hIndex, 1 );    // single bond
        }
        return out;
    }


    int Generator::formalCharge(const vector<Bond>& bonds) const
    {
        const int n = symbols.size();
        auto& pt = PeriodicTable::Instance();

        vector<int> bondOrderSum(n, 0);
        for (auto [i, j, o] : bonds) {
            bondOrderSum[i] += o;
            bondOrderSum[j] += o;
        }

        int total = 0;
        for (int i = 0;i < n;++i) {
            int desiredE = (symbols[ i ] == "H" ? 2 : 8);
            if ((symbols[ i ] == "Xe" || symbols[ i ] == "Kr" || symbols[ i ] == "Cl" ||
                symbols[ i ] == "Br" || symbols[ i ] == "I" || symbols[ i ] == "P" || symbols[ i ] == "S")
                && pt.Get( symbols[ i ] ).atomicNumber > 10)
            {
                desiredE = 12;
            }
            int bondingE = bondOrderSum[i] * 2;
            int loneE = std::max( 0, desiredE - bondingE );
            int lonePairs = loneE / 2;

            int fc = pt.Get( symbols[ i ] ).valenceElectrons - (2 * lonePairs) - (bondOrderSum[ i ]);

            total += std::abs( fc );
        }
        return total;
    }

    void Generator::dfs( int next,
        vector<uint8_t> &valenceLeft,
        vector<Bond> &current,
        int &bestCharge,
        vector<vector<Bond>> &out ) {
        // we no longer bail out on leftover valence — that leftover will become


        // If all heavy atoms have been considered:
        if (next == heavyCnt)
        {
            // don’t discard just because some valence remains!
            int qc = formalCharge( current );
            if (qc < bestCharge)
            {
                bestCharge = qc;
                out.clear();
                out.push_back( current );
            }
            else if (qc == bestCharge)
            {
                out.push_back( current );
            }
            return;
        }

        for (int prev = 0; prev < next; ++prev)
        {
       
            if (prev != 0 && next != 0) continue; // only bond to central
            for (int order = 1; order <= 3; ++order)
            {
                if (valenceLeft[ prev ] < order || valenceLeft[ next ] < order)
                    break;
                valenceLeft[ prev ] -= order;
                valenceLeft[ next ] -= order;
                current.emplace_back( prev, next, order );

                dfs( next + 1, valenceLeft, current, bestCharge, out );

                current.pop_back();
                valenceLeft[ prev ] += order;
                valenceLeft[ next ] += order;
            }
        }

    }


    vector<vector<Bond>> Generator::generateStructures()
    {
        vector<uint8_t> valenceLeft(heavyCnt);
        for (int i = 0;i < heavyCnt;++i)
            valenceLeft[i] = typicalValence(symbols[i]);

        vector<Bond> seed = attachHydrogens(valenceLeft);

        vector<Bond>               current = seed;
        vector<vector<Bond>>       out;
        int                        best = std::numeric_limits<int>::max();

        dfs(1, valenceLeft, current, best, out);      // heavy atom 0 = “root”
        return out;
    }

    std::vector<Bond> Generator::bestStructure()
    {
        auto internal = generateStructures();
        if (internal.empty()) return {};

        std::vector<Bond> out;
        for (auto [i, j, o] : internal.front()) {
            out.emplace_back(new2old[i], new2old[j], o);
        }
        return out;
    }


} // namespace Resonance
