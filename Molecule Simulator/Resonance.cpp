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


    Generator::Generator( const vector<string> &atoms, int nc ) : targetCharge(nc) {
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
            int desired = (base == "H") ? 2 : (pt.Get( base ).atomicNumber > 10 ? 12 : 8);

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
            int desired = (base == "H") ? 2 : (pt.Get( base ).atomicNumber > 10 ? 12 : 8);

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
        if (next == heavyCnt)
        {
            int fc = formalCharge( current );
            if (fc < bestCharge)
            {
                bestCharge = fc; bag.clear(); bag.push_back( current );
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

                int maxOrder = std::min<int>( 3, std::min( valLeft[ prev ], valLeft[ next ] ) );
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
                && !hypervalent( m ))                 /* octet preserved  */
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
                // --- basic bond stats ---
                int dbl = 0, tri = 0, sum = 0;
                std::vector<int> bondSum( symbols.size(), 0 );
                for (auto [i, j, o] : mol)
                {
                    if (o == 2) ++dbl;
                    if (o == 3) ++tri;
                    sum += o;
                    bondSum[ i ] += o; bondSum[ j ] += o;
                }

                // typical-valence deviation
                int typDev = 0;
                for (std::size_t k = 0; k < symbols.size(); ++k)
                    typDev += std::abs( bondSum[ k ] - typicalValence( symbols[ k ] ) );

                // multiple-bond penalty (triples weigh more automatically)
                int over = 0; for (auto [i, j, o] : mol) over += (o - 1) * (o - 1);

                // --- heavy subgraph ---
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
                    std::string base = stripCharge( symbols[ idx ] );
                    int Z = pt.Get( base ).atomicNumber;
                    return (base == "H") ? 2 : (Z > 10 ? 12 : 8);
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

                int conn = (int)mol.size();

                int absFC = formalCharge( mol );

                return std::tuple{
                    comps,          // connected heavy graph
                    cyclePenalty,   // penalize non-aromatic cycles
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