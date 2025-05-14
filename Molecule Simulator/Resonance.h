#ifndef RESONANCE_H
#define RESONANCE_H

#include <vector>
#include <string>
#include <tuple>
#include <cstdint>

namespace Resonance {

    using Bond = std::tuple<int, int, int>;

    class Generator {
    public:

        explicit Generator(const std::vector<std::string>& atoms);

        std::vector<int> new2old;   
        std::vector<int> old2new;   


        std::vector<std::vector<Bond>> generateStructures();
        std::vector<Bond>              bestStructure();

    private:
        std::vector<std::string> symbols;   // heavy atoms first, hydrogens last
        int                      heavyCnt{ 0 };

        static uint8_t typicalValence(const std::string& sym);

        std::vector<Bond> attachHydrogens(std::vector<uint8_t>& valenceLeft);

        void dfs(int next,
            std::vector<uint8_t>& valenceLeft,
            std::vector<Bond>& current,
            int& bestCharge,
            std::vector<std::vector<Bond>>& out);

        int formalCharge(const std::vector<Bond>& bonds) const;
    };

} // namespace Resonance
#endif
