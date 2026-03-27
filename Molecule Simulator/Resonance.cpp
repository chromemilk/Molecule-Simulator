#include "Resonance.h"
#include "PeriodicTable.h"

#include <algorithm>
#include <array>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <unordered_map>
#include <functional>
#include <unordered_set>

using std::vector;
using std::string;
using std::tuple;
using std::uint8_t;

namespace Resonance
{
    static bool heavyConnected( const std::vector<std::tuple<int, int, int>> &mol, int heavyCnt ) {
        if (heavyCnt <= 1) return true;
        std::vector<std::vector<int>> adj( heavyCnt );
        for (auto [i, j, o] : mol)
        {
            if (o <= 0) continue;
            if (i < heavyCnt && j < heavyCnt)
            {
                adj[ i ].push_back( j );
                adj[ j ].push_back( i );
            }
        }
        std::vector<char> seen( heavyCnt, 0 );
        std::vector<int> st = { 0 };
        seen[ 0 ] = 1;
        while (!st.empty())
        {
            int u = st.back(); st.pop_back();
            for (int v : adj[ u ]) if (!seen[ v ])
            {
                seen[ v ] = 1;
                st.push_back( v );
            }
        }
        return std::all_of( seen.begin(), seen.end(), []( char f ) { return f != 0; } );
    }

    static std::string canonicalBondKey( const std::vector<std::tuple<int, int, int>> &bonds ) {
        std::vector<std::tuple<int, int, int>> v;
        v.reserve( bonds.size() );
        for (auto [a, b, o] : bonds)
        {
            if (a > b) std::swap( a, b );
            v.emplace_back( a, b, o );
        }
        std::sort( v.begin(), v.end() );

        std::string key;
        key.reserve( v.size() * 8 );
        for (auto [a, b, o] : v)
        {
            key += std::to_string( a );
            key.push_back( '-' );
            key += std::to_string( b );
            key.push_back( ':' );
            key += std::to_string( o );
            key.push_back( ';' );
        }
        return key;
    }


    static inline std::vector<std::tuple<int, int, int>> MapBondsToOld( const std::vector<std::tuple<int, int, int>> &b,
            const std::vector<int> &n2o ) {
        std::vector<std::tuple<int, int, int>> out; out.reserve( b.size() );
        for (auto t : b)
        {
            int a, b2, o; std::tie( a, b2, o ) = t;
            out.emplace_back( n2o[ a ], n2o[ b2 ], o );
        }
        return out;
    }

    const std::vector<int> &Generator::new2oldMap() const {
        return new2old;
    }

    void Generator::setSearchCaps( const SearchCaps &caps ) {
        caps_ = caps;
    }

    SearchCaps Generator::searchCaps() const {
        return caps_;
    }

    int Generator::nodesVisited() const {
        return nodesVisited_;
    }

    std::vector<std::vector<std::tuple<int, int, int>>> Generator::generateStructuresOriginal() {
        auto internal = generateStructures();                // current internal list
        std::vector<std::vector<std::tuple<int, int, int>>> out; out.reserve( internal.size() );
        for (auto &v : internal) out.push_back( MapBondsToOld( v, new2old ) );
        return out;
    }

    static string stripCharge( string s ) {
        if (!s.empty() && (s.back() == '+' || s.back() == '-'))
        {
            s.pop_back();
            if (!s.empty() && ::isdigit( s.back() ))
                s.pop_back();
        }
        return s;
    }

    static const std::unordered_map<std::string, std::vector<uint8_t>> kValenceOpts = {
        // 1st row
        {"H",{1}}, {"He",{0}},
        // 2nd row
        {"Li",{1}}, {"Be",{2,4}}, {"B",{3,4}}, {"C",{4}},
        {"N",{3,4,5}}, {"O",{2,3}}, {"F",{1}}, {"Ne",{0}},
        // 3rd row
        {"Na",{1}}, {"Mg",{2}}, {"Al",{3,4}}, {"Si",{4}},
        {"P",{3,5,6}}, {"S",{2,4,6}}, {"Cl",{1,3,5}}, {"Ar",{0,2}},
        // 4th row (add transition metals)
        {"K",{1}}, {"Ca",{2}},
        {"Sc",{2,3}}, {"Ti",{2,3,4}}, {"V",{3,4,5}}, {"Cr",{2,3,6}}, {"Mn",{2,4,7}},
        {"Fe",{2,3}}, {"Co",{2,3}}, {"Ni",{2,3}}, {"Cu",{1,2}}, {"Zn",{2}},
        {"Ga",{3,4}}, {"Ge",{4}},
        {"As",{3,5}}, {"Se",{2,4,6}}, {"Br",{1,3,5}}, {"Kr",{0,2,4}},
        // 5th row
        {"Rb",{1}}, {"Sr",{2}},
        {"Y",{3}}, {"Zr",{2,3,4}}, {"Nb",{3,4,5}}, {"Mo",{2,4,6}}, {"Tc",{4,5,7}},
        {"Ru",{3,4,8}}, {"Rh",{1,3,4}}, {"Pd",{2,3,4}}, {"Ag",{1,2,3}}, {"Cd",{2}},
        {"In",{1,3}}, {"Sn",{2,4}},
        {"Sb",{3,5}}, {"Te",{2,4,6}}, {"I",{1,3,5}}, {"Xe",{0,2,4}},
        // 6th row
        {"Cs",{1}}, {"Ba",{2}},
        // Lanthanides
        {"La",{3}}, {"Ce",{3,4}}, {"Pr",{3,4}}, {"Nd",{2,3,4}}, {"Pm",{3}},
        {"Sm",{2,3}}, {"Eu",{2,3}}, {"Gd",{3}}, {"Tb",{3,4}}, {"Dy",{3}},
        {"Ho",{3}}, {"Er",{3}}, {"Tm",{2,3}}, {"Yb",{2,3}}, {"Lu",{3}},
        // 6th row transition metals cont.
        {"Hf",{2,3,4}}, {"Ta",{3,4,5}}, {"W",{4,5,6}}, {"Re",{4,6,7}},
        {"Os",{4,6,8}}, {"Ir",{3,4,6}}, {"Pt",{2,4,6}}, {"Au",{1,3}}, {"Hg",{1,2}},
        {"Tl",{1,3}}, {"Pb",{2,4}},
        {"Bi",{3,5}}, {"Po",{2,4,6}}, {"At",{1,3,5}}, {"Rn",{0,2}},
        // 7th row
        {"Fr",{1}}, {"Ra",{2}},
        // Actinides
        {"Ac",{3}}, {"Th",{3,4}}, {"Pa",{3,4,5}}, {"U",{4,5,6}}, {"Np",{4,5,6}},
        {"Pu",{3,4,6}}, {"Am",{3,4,6}}, {"Cm",{3}}, {"Bk",{3,4}}, {"Cf",{2,3,4}},
        {"Es",{3}}, {"Fm",{2,3}}, {"Md",{2,3}}, {"No",{2}}, {"Lr",{3}},
        // Post-actinide/superheavy p-block
        {"Rf",{4}}, {"Db",{5}}, {"Sg",{6}}, {"Bh",{5,7}}, {"Hs",{6}},
        {"Mt",{1,3}}, {"Ds",{2,4}}, {"Rg",{1,3}}, {"Cn",{2}},
        {"Nh",{1,3}}, {"Fl",{2,4}}, {"Mc",{1,3}}, {"Lv",{2,4}}, {"Ts",{1,3,5}}, {"Og",{0,2}}
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

    static int desiredElectronTarget( const std::string &base ) {
        if (base == "H") return 2;
        if (base == "B" || base == "Al" || base == "Ga" || base == "In" || base == "Tl") return 6;
        if (base == "P" || base == "S" || base == "Cl" || base == "Br" || base == "I"
            || base == "Se" || base == "Te" || base == "As" || base == "Sb" || base == "Xe") return 12;
        return 8;
    }

    static int maxBondOrderForPair( const std::string &a, const std::string &b ) {
        static const std::unordered_set<std::string> kMayUseQuad = {
            "Cr","Mo","W","Re","Ru","Rh","Os","Ir","Fe","Co","Ni"
        };

        const std::string A = stripCharge( a );
        const std::string B = stripCharge( b );
        if (kMayUseQuad.count( A ) || kMayUseQuad.count( B )) return 4;
        return 3;
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
    static const std::unordered_map<std::string, uint8_t> kTypV = {
        // 1st row
        {"H",1}, {"He",0},
        // 2nd row
        {"Li",1}, {"Be",2}, {"B",3}, {"C",4},
        {"N",4}, {"O",2}, {"F",1}, {"Ne",0},
        // 3rd row
        {"Na",1}, {"Mg",2}, {"Al",3}, {"Si",4},
        {"P",5}, {"S",6}, {"Cl",3}, {"Ar",2},
        // 4th row
        {"K",1}, {"Ca",2},
        {"Sc",3}, {"Ti",4}, {"V",5}, {"Cr",3}, {"Mn",2},
        {"Fe",3}, {"Co",2}, {"Ni",2}, {"Cu",2}, {"Zn",2},
        {"Ga",3}, {"Ge",4},
        {"As",5}, {"Se",6}, {"Br",3}, {"Kr",4},
        // 5th row
        {"Rb",1}, {"Sr",2},
        {"Y",3}, {"Zr",4}, {"Nb",5}, {"Mo",6}, {"Tc",7},
        {"Ru",3}, {"Rh",3}, {"Pd",2}, {"Ag",1}, {"Cd",2},
        {"In",3}, {"Sn",4},
        {"Sb",5}, {"Te",6}, {"I",3}, {"Xe",4},
        // 6th row
        {"Cs",1}, {"Ba",2},
        // Lanthanides
        {"La",3}, {"Ce",3}, {"Pr",3}, {"Nd",3}, {"Pm",3},
        {"Sm",3}, {"Eu",3}, {"Gd",3}, {"Tb",3}, {"Dy",3},
        {"Ho",3}, {"Er",3}, {"Tm",3}, {"Yb",3}, {"Lu",3},
        // 6th row transition metals cont.
        {"Hf",4}, {"Ta",5}, {"W",6}, {"Re",6},
        {"Os",4}, {"Ir",3}, {"Pt",2}, {"Au",3}, {"Hg",2},
        {"Tl",3}, {"Pb",4},
        {"Bi",5}, {"Po",6}, {"At",3}, {"Rn",2},
        // 7th row
        {"Fr",1}, {"Ra",2},
        // Actinides
        {"Ac",3}, {"Th",4}, {"Pa",5}, {"U",6}, {"Np",5},
        {"Pu",4}, {"Am",3}, {"Cm",3}, {"Bk",3}, {"Cf",3},
        {"Es",3}, {"Fm",3}, {"Md",3}, {"No",2}, {"Lr",3},
        // Post-actinide/superheavy p-block
        {"Rf",4}, {"Db",5}, {"Sg",6}, {"Bh",7}, {"Hs",6},
        {"Mt",3}, {"Ds",4}, {"Rg",3}, {"Cn",2},
        {"Nh",1}, {"Fl",2}, {"Mc",1}, {"Lv",2}, {"Ts",1}, {"Og",0}
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


    Generator::Generator( const std::vector<std::string> &atoms, int nc )
        : targetCharge( nc ) {
        const int n = (int)atoms.size();
        new2old.reserve( n );
        symbols.reserve( n );

        for (int i = 0; i < n; ++i) if (atoms[ i ] != "H")
        {
            new2old.push_back( i );
            symbols.push_back( atoms[ i ] );
        }
        heavyCnt = (int)symbols.size();
        for (int i = 0; i < n; ++i) if (atoms[ i ] == "H")
        {
            new2old.push_back( i );
            symbols.push_back( "H" );
        }

    
        if (heavyCnt >= 3)
        {
            auto &pt = PeriodicTable::Instance();

            auto countBase = [&]( const std::string &base ) {
                int c = 0;
                for (int i = 0; i < heavyCnt; ++i)
                    if (stripCharge( symbols[ i ] ) == base) ++c;
                return c;
                };

            auto score = [&]( int i ) {
                const std::string base = stripCharge( symbols[ i ] );
                const uint8_t tv = typicalValence( symbols[ i ] );   // includes charge effects
                const uint8_t mv = maxValenceFor( base );
                const float en = pt.Get( base ).electronegativity;

                int s = 0;
                s += 3 * int( tv );
                s += int( mv );
                if (countBase( base ) == 1) s += 2;                 
                if (base == "C") s += 1;                           // carbon often central
                if (base == "O") s -= 2;                           // O tends to be terminal
                if (base == "F" || base == "Cl" || base == "Br" || base == "I") s -= 3; // halogens terminal
                s -= int( en * 10.0f );                              // prefer less EN
                return s;
                };

            int best = 0;
            for (int i = 1; i < heavyCnt; ++i)
                if (score( i ) > score( best )) best = i;

            if (best != 0)
            {
                std::swap( symbols[ 0 ], symbols[ best ] );
                std::swap( new2old[ 0 ], new2old[ best ] );
            }
        }

        old2new.resize( n );
        for (int k = 0; k < n; ++k)
            old2new[ new2old[ k ] ] = k;

        auto &pt = PeriodicTable::Instance();
        for (const auto &sym : symbols)
        {
            std::string base = sym;
            if (!base.empty() && (base.back() == '+' || base.back() == '-'))
            {
                base.pop_back();
                if (!base.empty() && ::isdigit( base.back() )) base.pop_back();
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

        if (heavyCnt >= 10)
        {
            caps_.level = SearchCaps::Fast;
            caps_.maxNodes = 25000;
            caps_.maxStructures = 96;
        }
        else if (heavyCnt >= 7)
        {
            caps_.level = SearchCaps::Balanced;
            caps_.maxNodes = 75000;
            caps_.maxStructures = 192;
        }
        else
        {
            caps_.level = SearchCaps::Exhaustive;
            caps_.maxNodes = 220000;
            caps_.maxStructures = 512;
        }
    }

    int Generator::netCharge( const std::vector<Bond> &bonds ) const {
        const int n = symbols.size();
        std::vector<int> bondSum( n, 0 );
        for (auto [i, j, o] : bonds)
        {
            bondSum[ i ] += o;  bondSum[ j ] += o;
        }

        const auto &pt = PeriodicTable::Instance();
        int q = 0;

        for (int i = 0; i < n; ++i)
        {
            std::string base = stripCharge( symbols[ i ] );
            int desired = desiredElectronTarget( base );

            int ve = pt.Get( base ).valenceElectrons;
            int bondsOwn = bondSum[ i ];
            int loneE = std::max( 0, desired - 2 * bondSum[ i ] );

            q += ve - (bondsOwn + loneE);          // signed FC this time
        }
        return q;                                  // algebraic ?FC  (= total charge)
    }


    vector<Bond> Generator::attachHydrogens( vector<uint8_t> &valLeft ) {
        const int totalH = static_cast<int>(symbols.size()) - heavyCnt;
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

    int Generator::formalCharge( const std::vector<Bond> &bonds ) const {
        const int n = symbols.size();
        std::vector<int> bondSum( n, 0 );          // ? bond orders around each atom
        for (auto [i, j, o] : bonds)
        {
            bondSum[ i ] += o;  bondSum[ j ] += o;
        }

        const auto &pt = PeriodicTable::Instance();
        int total = 0;

        for (int i = 0; i < n; ++i)
        {
            std::string base = stripCharge( symbols[ i ] );
            int desired = desiredElectronTarget( base );

            int ve = pt.Get( base ).valenceElectrons;          // valence electrons
            int bondsOwn = bondSum[ i ];                             // B/2  (one e- per bond)
            int loneE = std::max( 0, desired - 2 * bondSum[ i ] );  // L    (electrons in lone pairs)

            int fc = ve - (bondsOwn + loneE);      // signed formal charge
            total += std::abs( fc );
        }
        return total;                              // ?|FC|
    }

    void Generator::dfs( int next,
        std::vector<std::uint8_t> &valLeft,
        std::vector<Bond> &current,
        int &bestCharge,
        std::vector<std::vector<Bond>> &bag ) {
        if (++nodesVisited_ > caps_.maxNodes)
            return;

        if (next == heavyCnt)
        {
            if (netCharge( current ) != targetCharge || hypervalent( current )) return;
            if (!heavyConnected( current, heavyCnt )) return;

            int fc = formalCharge( current );
            if (fc < bestCharge)
            {
                bestCharge = fc; bag.clear();
            }
            if (fc == bestCharge && static_cast<int>( bag.size() ) < caps_.maxStructures)
                bag.push_back( current );
            return;
        }

        auto exceedsLimit = [&]( int atom, int extra ) {
            int already = typicalValence( symbols[ atom ] ) - valLeft[ atom ];
            return (already + extra) > maxValenceFor( symbols[ atom ] );
            };

        // Decide bonds from `next` to ALL previous heavy atoms
        std::function<void( int )> linkPrev = [&]( int prev )
            {
                if (prev == next)
                {
                    // done wiring `next`; move on
                    dfs( next + 1, valLeft, current, bestCharge, bag );
                    return;
                }

                if (centralOnlyBonding && heavyCnt > 2 && prev != 0 && next != 0)
                {
                    linkPrev( prev + 1 );         // skip if using “central only” mode
                    return;
                }

                int maxOrder = std::min<int>( maxBondOrderForPair( symbols[ prev ], symbols[ next ] ), std::min( valLeft[ prev ], valLeft[ next ] ) );
                while (maxOrder >= 1 && (exceedsLimit( prev, maxOrder ) || exceedsLimit( next, maxOrder )))
                    --maxOrder;

                // Try all possibilities, including 0 (no bond to this prev)
                for (int o = maxOrder; o >= 0; --o)
                {
                    if (o > 0)
                    {
                        valLeft[ prev ] -= o;  valLeft[ next ] -= o;
                        current.emplace_back( prev, next, o );
                    }

                    linkPrev( prev + 1 );

                    if (o > 0)
                    {
                        current.pop_back();
                        valLeft[ prev ] += o;  valLeft[ next ] += o;
                    }
                }
            };

        linkPrev( 0 );
    }


    vector<vector<Bond>> Generator::generateStructures() {
        vector<vector<Bond>> all;
        nodesVisited_ = 0;

        if (heavyCnt == 0)
        {
            if (symbols.size() % 2 == 0)
            {
                vector<Bond> hb;
                for (int i = 0; i < static_cast<int>( symbols.size() ); i += 2)
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

        std::unordered_set<std::string> seen;
        vector<vector<Bond>> dedup;
        dedup.reserve( all.size() );
        for (const auto &m : all)
        {
            std::string key = canonicalBondKey( m );
            if (seen.insert( key ).second)
                dedup.push_back( m );
        }
        all.swap( dedup );
        return all;
    }


    static int overbondPenalty( const vector<Bond> &bs ) {
        int p = 0; for (auto [i, j, o] : bs) p += (o - 1) * (o - 1); return p;
    }
    static int connectivity( const vector<Bond> &bs ) {
        return (int)bs.size();
    }


    bool Generator::hypervalent( const std::vector<Bond> &mol ) const {
        std::vector<int> bondSum( symbols.size(), 0 );
        for (auto [i, j, o] : mol)
        {
            bondSum[ i ] += o; bondSum[ j ] += o;
        }

        const auto &pt = PeriodicTable::Instance();
        for (std::size_t k = 0; k < symbols.size(); ++k)
        {
            int Z = pt.Get( symbols[ k ] ).atomicNumber;
            if (Z <= 10 && bondSum[ k ] > 4)       // B–Ne may not exceed octet
                return true;
        }
        return false;
    }

    std::vector<Bond> Generator::bestStructure() {
 
        const auto all = generateStructures();        
        if (all.empty())
            throw std::runtime_error( "No resonance structures generated" );


        std::vector<const std::vector<Bond> *> legal;
        for (const auto &m : all)
            if (netCharge( m ) == targetCharge      /* exact charge     */
                && !hypervalent( m )
                && heavyConnected( m, heavyCnt ))
                legal.push_back( &m );

        if (legal.empty())
            throw std::runtime_error(
                "No structure matches the required charge "
                "without exceeding the octet on 2nd-row atoms" );


        auto fcSum = [&]( const std::vector<Bond> &mol ) {
            return formalCharge( mol );              // one call, no loop
        };

        auto fcDelta = [&]( const std::vector<Bond> &mol ) {
            return std::abs( formalCharge( mol ) - targetCharge );
        };

        int bestDelta = std::numeric_limits<int>::max();
        for (auto m : legal) bestDelta = std::min( bestDelta, fcDelta( *m ) );

      
        auto score = [&]( const std::vector<Bond> &mol )
            {
                int dbl = 0, tri = 0, sum = 0;
                std::vector<int> bondSum( symbols.size(), 0 );
                for (auto [i, j, o] : mol)
                {
                    if (o == 2) ++dbl;
                    if (o == 3) ++tri;
                    sum += o;
                    bondSum[ i ] += o; bondSum[ j ] += o;
                }

                int typDev = 0;
                for (std::size_t k = 0; k < symbols.size(); ++k)
                    typDev += std::abs( bondSum[ k ] - typicalValence( symbols[ k ] ) );

                int octetPenalty = 0;
                int chargePlacementPenalty = 0;
                for (std::size_t k = 0; k < symbols.size(); ++k)
                {
                    const std::string base = stripCharge( symbols[ k ] );
                    const auto &elem = PeriodicTable::Instance().Get( base );
                    const int desired = desiredElectronTarget( base );
                    const int lone = std::max( 0, desired - 2 * bondSum[ k ] );
                    const int owned = bondSum[ k ] + lone;
                    const int fcSigned = elem.valenceElectrons - owned;

                    if (elem.atomicNumber <= 10)
                    {
                        const int targetOwned = (base == "H") ? 2 : 8;
                        octetPenalty += std::abs( owned - targetOwned );
                    }

                    const int enScaled = int( elem.electronegativity * 10.0f );
                    if (fcSigned < 0) chargePlacementPenalty += std::max( 0, 32 - enScaled );
                    if (fcSigned > 0) chargePlacementPenalty += std::max( 0, enScaled - 20 );
                }

                int over = 0; for (auto [i, j, o] : mol) over += (o - 1) * (o - 1);

                std::vector<std::vector<int>> adj( heavyCnt );
                int heavyEdges = 0;
                for (auto [i, j, o] : mol)
                {
                    if (i < heavyCnt && j < heavyCnt)
                    {
                        ++heavyEdges;
                        adj[ i ].push_back( j );
                        adj[ j ].push_back( i );
                    }
                }

                // connected components among heavy atoms
                int comps = 0;
                std::vector<char> seen( heavyCnt, 0 );
                for (int v = 0; v < heavyCnt; ++v) if (!seen[ v ])
                {
                    ++comps;
                    std::vector<int> st = { v }; seen[ v ] = 1;
                    while (!st.empty())
                    {
                        int u = st.back(); st.pop_back();
                        for (int w : adj[ u ]) if (!seen[ w ])
                        {
                            seen[ w ] = 1; st.push_back( w );
                        }
                    }
                }

                // cyclomatic number on heavy subgraph
                int cycl = std::max( 0, heavyEdges - heavyCnt + comps );

       
                std::unordered_map<long long, int> edgeOrder;
                auto key = []( int a, int b )->long long { if (a > b) std::swap( a, b ); return ((long long)a << 32) | b; };
                for (auto [i, j, o] : mol) if (i < heavyCnt && j < heavyCnt) edgeOrder[ key( i, j ) ] = o;

                // desired electron count (octet/expanded) for estimating available lone pairs
                auto &pt = PeriodicTable::Instance();
                auto desiredE = [&]( int idx ) {
                    return desiredElectronTarget( stripCharge( symbols[ idx ] ) );
                    };

                std::vector<int> loneE( symbols.size(), 0 );
                for (int i = 0; i < (int)symbols.size(); ++i)
                    loneE[ i ] = std::max( 0, desiredE( i ) - 2 * bondSum[ i ] );

                auto isHetero = [&]( int a ) {
                    const std::string s = stripCharge( symbols[ a ] );
                    return (s == "N" || s == "O" || s == "S" || s == "P");
                    };

                std::vector<int> compId( heavyCnt, -1 );
                int compCount = 0;
                for (int v = 0; v < heavyCnt; ++v) if (compId[ v ] < 0)
                {
                    std::vector<int> st = { v }; compId[ v ] = compCount;
                    while (!st.empty())
                    {
                        int u = st.back(); st.pop_back();
                        for (int w : adj[ u ]) if (compId[ w ] < 0)
                        {
                            compId[ w ] = compCount; st.push_back( w );
                        }
                    }
                    ++compCount;
                }

                int aromaticAllowed = 0;
                for (int c = 0; c < compCount; ++c)
                {
                    std::vector<int> verts;
                    for (int v = 0; v < heavyCnt; ++v) if (compId[ v ] == c) verts.push_back( v );
                    if (verts.empty()) continue;

                    int Ecomp = 0; bool allDeg2 = true;
                    for (int v : verts)
                    {
                        int deg = 0;
                        for (int w : adj[ v ]) if (compId[ w ] == c) ++deg;
                        allDeg2 &= (deg == 2);
                        Ecomp += deg;
                    }
                    Ecomp /= 2;
                    if (!(allDeg2 && (Ecomp == (int)verts.size()))) continue;

                    int pi = 0, dblEdges = 0;

                    for (int u : verts) for (int w : adj[ u ]) if (compId[ w ] == c && u < w)
                    {
                        int o = edgeOrder[ key( u, w ) ];
                        if (o >= 2)
                        {
                            pi += 2; ++dblEdges;
                        }
                    }

                    for (int u : verts)
                    {
                        bool hasRingDouble = false;
                        for (int w : adj[ u ]) if (compId[ w ] == c)
                        {
                            if (edgeOrder[ key( u, w ) ] >= 2)
                            {
                                hasRingDouble = true; break;
                            }
                        }
                        if (!hasRingDouble && isHetero( u ) && loneE[ u ] >= 2)
                            pi += 2; // donate ONE lone pair
                    }

                    if (dblEdges == 0) continue;

                    // Hückel 4n+2
                    if (pi >= 2 && ((pi - 2) % 4 == 0))
                        ++aromaticAllowed;
                }

                int cyclePenalty = std::max( 0, cycl - aromaticAllowed );
                if (cycl > 0 && cyclePenalty > 0 && cycl <= 2)
                    --cyclePenalty; // keep simple non-aromatic rings viable

                int conn = (int)mol.size();

                int absFC = formalCharge( mol );

                return std::tuple{
                    comps,          // connected heavy graph
                    cyclePenalty,   // penalize non-aromatic cycles
                    octetPenalty,   // enforce octet quality on 2nd-row atoms
                    chargePlacementPenalty, // prefer chemically plausible charge location
                    absFC,          // minimize total |formal charge|
                    typDev,         // near-typical valences
                    over,           // avoid unnecessary multiple bonds
                    tri,            // fewer triples
                    -heavyEdges,    // encourage justified heavy connectivity
                    conn            // avoid gratuitous extra edges
                };
            };


        const auto *best = *std::min_element(
            legal.begin(), legal.end(),
            [&]( auto a, auto b ) { return score( *a ) < score( *b ); }
        );

        std::vector<Bond> out;
        for (auto [i, j, o] : *best)
            out.emplace_back( new2old[ i ], new2old[ j ], o );
        return out;
    }

} // namespace Resonance