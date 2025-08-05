#include "Resonance.h"
#include "PeriodicTable.h"

#include <algorithm>
#include <array>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <unordered_map>
#include <functional>

using std::vector;
using std::string;
using std::tuple;
using std::uint8_t;

namespace Resonance
{


    // Preferred valence options for elements where chemistry strongly restricts
    // the number of bonds that can be formed.  Elements not present in this map
    // will derive their options algorithmically from the periodic table data.
    static string stripCharge( string s ) {
        if (!s.empty() && (s.back() == '+' || s.back() == '-'))
        {
            s.pop_back();
            if (!s.empty() && ::isdigit( s.back() ))
                s.pop_back();
        }
        return s;
    }

    static const std::unordered_map<string, vector<uint8_t>> kValenceOpts = {
        {"H",{1}},                       {"C",{4}},
        {"N",{3,4,5}},                   {"O",{2,3}},
        {"P",{3,5,6}},                   {"S",{2,4,6}},
        {"F",{1}},   {"Cl",{1,3,5,7}},   {"Br",{1,3,5,7}}, {"I",{1,3,5,7}},
        {"Ar",{0,2}}, {"Kr",{0,2,4,6}},   {"Xe",{0,2,4,6}}
    };

    static uint8_t maxValenceFor( const string &sym ) {
        string base = stripCharge( sym );
        if (auto it = kValenceOpts.find( base ); it != kValenceOpts.end())
            return *std::max_element( it->second.begin(), it->second.end() );

        const auto &pt = PeriodicTable::Instance();
        const auto &e = pt.Get( base );

        if (base == "H") return 1;
        if (e.atomicNumber <= 10) return 4; // second period obeys octet strictly
        if (base == "Cl" || base == "Br" || base == "I" || base == "At" || base == "Ts")
            return 7;                       // halogens may expand up to seven bonds
        return 6;                            // general upper bound for others
    }

    static vector<uint8_t> valenceOptions( const string &sym ) {
        string base = stripCharge( sym );
        if (auto it = kValenceOpts.find( base ); it != kValenceOpts.end())
            return it->second;

        uint8_t m = maxValenceFor( base );
        vector<uint8_t> opts;
        for (uint8_t v = 1; v <= m; ++v) opts.push_back( v );
        return opts;
    }

    static const std::unordered_map<string, uint8_t> kTypV = {
        {"H",1},{"C",4},{"N",4},{"O",2},
        {"F",1},{"Cl",3},{"Br",3},{"I",3},
        {"P",5},{"As",5},{"Sb",5},
        {"S",6},{"Se",6},{"Te",6},
        {"B",3},{"Al",3},{"Ga",3},{"In",3},
        {"Xe",4},{"Kr",4},{"Ar",2},
    };


    uint8_t Generator::typicalValence( string sym ) {
        int formalCharge = 0;
        if (!sym.empty() && (sym.back() == '+' || sym.back() == '-'))
        {
            char sign = sym.back(); sym.pop_back();
            int mag = 1;
            if (!sym.empty() && ::isdigit( sym.back() ))
            {
                mag = sym.back() - '0'; sym.pop_back();
            }
            formalCharge = (sign == '+') ? -mag : +mag;
        }

        if (auto it = kTypV.find( sym ); it != kTypV.end())
            return std::max<uint8_t>( 1, uint8_t( it->second - formalCharge ) );

        try
        {
            const auto &pt = PeriodicTable::Instance();
            const auto &elem = pt.Get( sym );
            int ve = elem.valenceElectrons - formalCharge;
            uint8_t v = std::clamp<uint8_t>( (8 - ve) / 2, 1, 4 );
            if (elem.atomicNumber >= 15)
                v = std::min<uint8_t>( 6, std::max<uint8_t>( v, 4 ) );
            return v;
        }
        catch (const std::out_of_range &)
        {
            throw std::invalid_argument( "Unknown element symbol: " + sym );
        }
    }

 
    Generator::Generator( const vector<string> &atoms ) {
        const int n = atoms.size();
        new2old.reserve( n );   symbols.reserve( n );

        for (int i = 0; i < n; ++i) if (atoms[ i ] != "H")
        {
            new2old.push_back( i ); symbols.push_back( atoms[ i ] );
        }
        heavyCnt = symbols.size();
        for (int i = 0; i < n; ++i) if (atoms[ i ] == "H")
        {
            new2old.push_back( i ); symbols.push_back( "H" );
        }

        old2new.resize( n );
        for (int k = 0; k < n; ++k) old2new[ new2old[ k ] ] = k;

        auto &pt = PeriodicTable::Instance();
        for (const auto &sym : symbols)
        {
            string base = sym;
            if (!base.empty() && (base.back() == '+' || base.back() == '-'))
            {
                base.pop_back();
                if (!base.empty() && ::isdigit( base.back() ))
                    base.pop_back();
            }
            try
            {
                pt.Get( base );
            }
            catch (const std::out_of_range &)
            {
                throw std::invalid_argument( "Unknown element symbol: " + sym );
            }
        }
    }

    vector<Bond> Generator::attachHydrogens( vector<uint8_t> &valLeft ) {
        const int totalH = static_cast<int>( symbols.size() ) - heavyCnt;
        int        hIndex = heavyCnt;               // first H in reordered list
        int        hLeft = totalH;

        auto needReserve = [&]( int idx ) {
            return (symbols[ idx ] == "O") && (valLeft[ idx ] > 0);
            };
        int reserved = 0;
        for (int i = 0; i < heavyCnt; ++i)
            if (needReserve( i )) ++reserved;

        vector<Bond> out;
        for (int h = 0; h < totalH; ++h, ++hIndex, --hLeft)
        {
            bool mustGiveToO = (reserved > 0 && hLeft <= reserved);

            int best = -1, bestVal = -1;

            for (int i = 0; i < heavyCnt; ++i)
            {
                if (valLeft[ i ] == 0) continue;

                if (mustGiveToO)
                {
                    if (!needReserve( i )) continue;
                    best = i; break;                  // take the first suitable O
                }

                if (symbols[ i ] == "O") continue;
                if (valLeft[ i ] > bestVal)
                {
                    bestVal = valLeft[ i ]; best = i;
                }
            }

            if (best == -1)
            {
                for (int i = 0; i < heavyCnt; ++i)
                    if (valLeft[ i ] > 0)
                    {
                        best = i; break;
                    }
            }

            if (best == -1) continue;

            --valLeft[ best ];
            if (needReserve( best ) && valLeft[ best ] == 0) --reserved;
            out.emplace_back( best, hIndex, 1 );
        }
        return out;
    }

    int Generator::formalCharge( const vector<Bond> &bonds ) const {
        const int n = symbols.size();
        vector<int> bondSum( n, 0 );
        for (auto [i, j, o] : bonds)
        {
            bondSum[ i ] += o; bondSum[ j ] += o;
        }

        auto &pt = PeriodicTable::Instance();
        int total = 0;
        for (int i = 0; i < n; ++i)
        {
            string base = stripCharge( symbols[ i ] );
            int desired = (base == "H") ? 2 : 8;
            if (pt.Get( base ).atomicNumber > 10) desired = 12; // hypervalent allowed

            int bondingE = bondSum[ i ] * 2;
            int loneE = std::max( 0, desired - bondingE );
            int fc = pt.Get( base ).valenceElectrons - (bondSum[ i ] + loneE / 2 * 2);
            total += std::abs( fc );
        }
        return total;
    }
    void Generator::dfs( int next,
        std::vector<std::uint8_t> &valLeft,
        std::vector<Bond> &current,
        int &bestCharge,
        std::vector<std::vector<Bond>> &bag ) {
        if (next == heavyCnt)
        {
            int fc = formalCharge( current );
            if (fc < bestCharge)
            {
                bestCharge = fc;
                bag.clear();
                bag.push_back( current );
            }
            else if (fc == bestCharge)
            {
                bag.push_back( current );
            }
            return;
        }

        auto exceedsLimit = [&]( int atom, int extra ) {
            int already = typicalValence( symbols[ atom ] ) - valLeft[ atom ];
            return (already + extra) > maxValenceFor( symbols[ atom ] );
            };

        for (int prev = 0; prev < next; ++prev)
        {
            if (centralOnlyBonding && heavyCnt > 2 && prev != 0 && next != 0)
                continue;

            int maxOrder = std::min<int>( 3, std::min( valLeft[ prev ], valLeft[ next ] ) );

            while (maxOrder >= 1 &&
                (exceedsLimit( prev, maxOrder ) || exceedsLimit( next, maxOrder )))
                --maxOrder;

            for (int o = maxOrder; o >= 1; --o)
            {
                valLeft[ prev ] -= o;  valLeft[ next ] -= o;
                current.emplace_back( prev, next, o );

                dfs( next + 1, valLeft, current, bestCharge, bag );

                current.pop_back();
                valLeft[ prev ] += o;  valLeft[ next ] += o;
            }
        }

        dfs( next + 1, valLeft, current, bestCharge, bag );
    }


    vector<vector<Bond>> Generator::generateStructures() {
        vector<vector<Bond>> all;

        if (heavyCnt == 0)
        {
            if (symbols.size() % 2 == 0)
            {
                vector<Bond> hb;
                for (int i = 0; i < static_cast<int>(symbols.size()); i += 2)
                    hb.emplace_back( i, i + 1, 1 );
                all.push_back( std::move( hb ) );
            }
            return all;
        }

        vector<vector<uint8_t>> opts( heavyCnt );
        for (int i = 0; i < heavyCnt; ++i)
            opts[ i ] = valenceOptions( symbols[ i ] );

        vector<uint8_t> valSel( heavyCnt );

        std::function<void( int )> back = [&]( int idx )
            {
                if (idx == heavyCnt)
                {
                    vector<uint8_t> valLeft = valSel;
                    vector<Bond> seed = attachHydrogens( valLeft );
                    int sum = std::accumulate( valLeft.begin(), valLeft.end(), 0 );
                    if (sum % 2 != 0) return;

                    vector<Bond> cur( seed );
                    int best = std::numeric_limits<int>::max();
                    vector<vector<Bond>> bag;
                    dfs( 1, valLeft, cur, best, bag );

                    all.insert( all.end(), bag.begin(), bag.end() );
                    return;
                }

                for (uint8_t v : opts[ idx ])
                {
                    valSel[ idx ] = v; back( idx + 1 );
                }
            };
        back( 0 );
        return all;
    }


    static int overbondPenalty( const vector<Bond> &bs ) {
        int p = 0; for (auto [i, j, o] : bs) p += (o - 1) * (o - 1); return p;
    }
    static int connectivity( const vector<Bond> &bs ) {
        return (int)bs.size();
    }

    std::vector<Bond> Generator::bestStructure() {
        auto candidates = generateStructures();
        if (candidates.empty())
            throw std::runtime_error( "No valid resonance structures found" );

        int minFC = std::numeric_limits<int>::max();
        for (auto &m : candidates)
            minFC = std::min( minFC, formalCharge( m ) );

        std::vector<std::vector<Bond>> filtered;
        for (auto &m : candidates)
            if (formalCharge( m ) == minFC)
                filtered.push_back( m );

        auto score = [&]( const std::vector<Bond> &mol ) {
            int dbl = 0, tri = 0, sum = 0;
            for (auto [i, j, o] : mol)
            {
                if (o == 2) ++dbl;
                if (o == 3) ++tri;
                sum += o;
            }

            std::vector<int> bondCnt( symbols.size(), 0 );
            for (auto [i, j, o] : mol)
            {
                bondCnt[ i ] += o; bondCnt[ j ] += o;
            }
            int typDev = 0;
            for (int i = 0; i < static_cast<int>(symbols.size()); ++i)
                typDev += std::abs( bondCnt[ i ] - typicalValence( symbols[ i ] ) );

            int over = overbondPenalty( mol );
            int conn = connectivity( mol );

            return std::tuple{ dbl, -typDev, -over, conn, -tri, sum };
            };

        auto bestIt = std::max_element(
            filtered.begin(), filtered.end(),
            [&]( const auto &a, const auto &b ) { return score( a ) < score( b ); } );

        std::vector<Bond> result;
        for (auto [i, j, o] : *bestIt)
            result.emplace_back( new2old[ i ], new2old[ j ], o );

        return result;
    }

} // namespace Resonance
