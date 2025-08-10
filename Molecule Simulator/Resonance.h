#ifndef RESONANCE_H
#define RESONANCE_H

#include <vector>
#include <string>
#include <tuple>
#include <cstdint>

namespace Resonance {

    using Bond = std::tuple<int, int, int>;
    
    inline bool centralOnlyBonding = false;

    struct SearchCaps
    {
        enum Level
        {
            Fast, Balanced, Exhaustive
        } level = Balanced;
        int maxNodes = 50000;            // cap recursive nodes
        int maxStructures = 256;         // cap bag size
    };


    class Generator {
    public:

        explicit Generator(const std::vector<std::string>& atoms, int netCharge = 0);


        std::vector<int> new2old;   
        std::vector<int> old2new;   


        std::vector<std::vector<Bond>> generateStructures();
        std::vector<Bond>  bestStructure();
        bool hypervalent( const std::vector<Bond> & ) const;

    private:
        SearchCaps caps_;
        int nodesVisited_ = 0;
        std::vector<std::string> symbols;   // heavy atoms first, hydrogens last
        int heavyCnt{ 0 };
        int targetCharge = 0;
        std::vector<std::vector<uint8_t>> allowed_; 
        void buildSkeletonAndSeed( std::vector<uint8_t> &valLeft, std::vector<Bond> &seed );
        int netCharge( const std::vector<Bond> & ) const;

        static uint8_t typicalValence(std::string sym);

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
