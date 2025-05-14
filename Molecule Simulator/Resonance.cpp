
#include "resonance.h"
#include "PeriodicTable.h"
#include <limits>

namespace Resonance {

    Generator::Generator(const std::vector<std::string>& atoms)
        : symbols(atoms) {
        int n = symbols.size();
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                pairs.emplace_back(i, j);
            }
        }
    }

    void Generator::addAtom(const std::string& symbol) {
        int idx = symbols.size();
        symbols.push_back(symbol);
        for (int i = 0; i < idx; ++i) {
            pairs.emplace_back(i, idx);
        }
    }

    int Generator::desiredElectrons(const std::string& sym) {
        return (sym == "H" ? 2 : 8);
    }

    int Generator::totalElectrons() const {
        int sum = 0;
        auto& pt = PeriodicTable::Instance();
        for (const auto& s : symbols) {
            sum += pt.Get(s).valenceElectrons;
        }
        return sum;
    }

    std::vector<std::vector<Bond>> Generator::generateStructures() {
        std::vector<std::vector<Bond>> result;
        std::vector<Bond> current;
        int electrons = totalElectrons();
        backtrack(0, electrons, current, result);
        return result;
    }

    void Generator::backtrack(int pairIndex,
        int electronsLeft,
        std::vector<Bond>& current,
        std::vector<std::vector<Bond>>& out) const {
        int numPairs = pairs.size();
        // If all pairs assigned
        if (pairIndex == numPairs) {
            if (electronsLeft < 0 || (electronsLeft % 2) != 0) return;
            int n = symbols.size();
            std::vector<int> bondElectrons(n, 0);
            for (const auto& b : current) {
                int i, j, o;
                std::tie(i, j, o) = b;
                bondElectrons[i] += o * 2;
                bondElectrons[j] += o * 2;
            }
            int left = electronsLeft;
            for (int i = 0; i < n; ++i) {
                int cap = desiredElectrons(symbols[i]) - bondElectrons[i];
                int use = std::min(cap, left);
                if (use % 2) --use;
                left -= use;
            }
            if (left != 0) return;
            out.push_back(current);
            return;
        }

        int i = pairs[pairIndex].first;
        int j = pairs[pairIndex].second;
        auto calcBE = [&](int idx) {
            int sum = 0;
            for (const auto& b : current) {
                int a1, a2, o;
                std::tie(a1, a2, o) = b;
                if (a1 == idx || a2 == idx) sum += o * 2;
            }
            return sum;
            };

        // Try possible bond orders
        for (int order = 0; order <= 3; ++order) {
            int used = order * 2;
            if (electronsLeft - used < 0) break;
            int be_i = calcBE(i);
            int be_j = calcBE(j);
            if (be_i + used > desiredElectrons(symbols[i])) break;
            if (be_j + used > desiredElectrons(symbols[j])) continue;

            if (order > 0) current.emplace_back(i, j, order);
            backtrack(pairIndex + 1, electronsLeft - used, current, out);
            if (order > 0) current.pop_back();
        }
    }

    int Generator::score(const std::vector<Bond>& bonds) const {
        int n = symbols.size();
        auto& pt = PeriodicTable::Instance();
        std::vector<int> bondPairs(n, 0);
        for (const auto& b : bonds) {
            int i, j, o;
            std::tie(i, j, o) = b;
            bondPairs[i] += o;
            bondPairs[j] += o;
        }
        int totalE = totalElectrons();
        int usedE = 0;
        for (const auto& b : bonds) usedE += std::get<2>(b) * 2;
        int left = totalE - usedE;
        std::vector<int> loneE(n, 0);
        for (int i = 0; i < n; ++i) {
            int cap = desiredElectrons(symbols[i]) - bondPairs[i] * 2;
            int use = std::min(cap, left);
            if (use % 2) --use;
            loneE[i] = use;
            left -= use;
        }
        int score = 0;
        for (int i = 0; i < n; ++i) {
            int V = pt.Get(symbols[i]).valenceElectrons;
            int nonbond = loneE[i];
            int bonding = bondPairs[i];
            int fc = V - (nonbond + bonding);
            score += std::abs(fc);
        }
        return score;
    }

    std::vector<Bond> Generator::bestStructure() {
        auto structures = generateStructures();
        int bestScore = std::numeric_limits<int>::max();
        std::vector<Bond> best;
        for (const auto& s : structures) {
            int sc = score(s);
            if (sc < bestScore) {
                bestScore = sc;
                best = s;
            }
        }
        return best;
    }

} 
