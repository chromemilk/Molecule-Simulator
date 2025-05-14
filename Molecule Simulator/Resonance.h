#ifndef RESONANCE_H
#define RESONANCE_H

#include <vector>
#include <string>
#include <tuple>

namespace Resonance {

    using Bond = std::tuple<int, int, int>;

    class Generator {
    public:
        Generator(const std::vector<std::string>& atoms);

        void addAtom(const std::string& symbol);

        std::vector<std::vector<Bond>> generateStructures();

        std::vector<Bond> bestStructure();

    private:
        std::vector<std::string> symbols;
        std::vector<std::pair<int, int>> pairs;  // All unique atom index pairs

        int totalElectrons() const;

        static int desiredElectrons(const std::string& sym);

        void backtrack(int pairIndex,
            int electronsLeft,
            std::vector<Bond>& current,
            std::vector<std::vector<Bond>>& out) const;

        int score(const std::vector<Bond>& bonds) const;
    };

} 

#endif // RESONANCE_H