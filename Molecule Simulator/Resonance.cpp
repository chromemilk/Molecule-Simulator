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
        {"H",1},{"C",4},{"N",3},{"O",2},{"F",1},{"Cl",1},{"Br",1},{"I",1},
        {"S",2},{"P",3},{"B",3}
    };

    uint8_t Generator::typicalValence(const string& s) {
        auto it = kTypV.find(s);
        if (it != kTypV.end()) return it->second;
        auto& pt = PeriodicTable::Instance();
        int ve = pt.Get(s).valenceElectrons;
        return std::clamp<uint8_t>((8 - ve) / 2, 1, 4);
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


    vector<Bond> Generator::attachHydrogens(vector<uint8_t>& valenceLeft)
    {
        const int totalH = symbols.size() - heavyCnt;
        vector<Bond> out;
        int hIndex = heavyCnt;                 // first H in `symbols`

   
        for (int h = 0; h < totalH; ++h, ++hIndex) {
            auto it = std::max_element(valenceLeft.begin(), valenceLeft.end());
            int i = std::distance(valenceLeft.begin(), it);
            if (*it == 0)
                throw std::runtime_error("Ran out of valence while attaching H");
            (*it)--;
            out.emplace_back(i, hIndex, 1);    // single bond
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
            int desiredE = (symbols[i] == "H" ? 2 : 8);
            int bondingE = bondOrderSum[i] * 2;
            int loneE = std::max(0, desiredE - bondingE);
            int lonePairs = loneE / 2;

            int fc = pt.Get(symbols[i]).valenceElectrons - (bondOrderSum[i] + lonePairs);
            total += std::abs(fc);
        }
        return total;
    }

    void Generator::dfs(int next,
        vector<uint8_t>& valenceLeft,
        vector<Bond>& current,
        int& bestCharge,
        vector<vector<Bond>>& out)
    {
        // AFTER — check the *whole* list or drop the test
        int remAll = std::accumulate(valenceLeft.begin(), valenceLeft.end(), 0);
        if (remAll & 1) return;          // total remaining valence must be even

        if (next == heavyCnt) {
            if (remAll != 0) return;                        // unsaturated
            int qc = formalCharge(current);
            if (qc < bestCharge) {
                bestCharge = qc;
                out.clear();
                out.push_back(current);
            }
            else if (qc == bestCharge) {
                out.push_back(current);
            }
            return;
        }

        for (int prev = 0; prev < next; ++prev) {
            for (int order = 1; order <= 3; ++order) {
                if (valenceLeft[prev] < order || valenceLeft[next] < order) break;
                valenceLeft[prev] -= order;
                valenceLeft[next] -= order;
                current.emplace_back(prev, next, order);

                dfs(next + 1, valenceLeft, current, bestCharge, out);

                current.pop_back();
                valenceLeft[prev] += order;
                valenceLeft[next] += order;
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
