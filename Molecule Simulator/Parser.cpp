// Parser.cpp  – robust formula ? list<string> atoms
#include "Parser.h"
#include "PeriodicTable.h"
#include <cctype>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

using std::string;
using std::vector;


static std::size_t readNumber( const string &s, std::size_t &i ) {
    std::size_t start = i;
    while (i < s.size() && std::isdigit( s[ i ] )) ++i;
    return (i == start) ? 1 : std::stoul( s.substr( start, i - start ) );
}


static void parseInto( const string &s, std::size_t &i, char endTok,
    std::unordered_map<string, int> &counts,
    std::size_t scopeMult ) {
    while (i < s.size())
    {
        char c = s[ i ];

        if (endTok && c == endTok)
        {
            ++i; return;
        }

        if (std::isupper( c ))
        {
            string sym;  sym.push_back( c );  ++i;
            if (i < s.size() && std::islower( s[ i ] )) sym.push_back( s[ i++ ] );

            std::size_t n = readNumber( s, i );
            counts[ sym ] += static_cast<int>( scopeMult * n );
            continue;
        }

        if (c == '(' || c == '[' || c == '{')
        {
            char open = c, close = (c == '(' ? ')' : c == '[' ? ']' : '}');
            ++i;                                       // skip open
            std::unordered_map<string, int> sub;
            parseInto( s, i, close, sub, 1 );            // recurse
            std::size_t grpMult = readNumber( s, i );    // multiplier after bracket
            for (auto &kv : sub)
                counts[ kv.first ] += kv.second * static_cast<int>(scopeMult * grpMult);
            continue;
        }

        if (c == '+' || c == '-' || c == '^') return;

        throw std::runtime_error( "Unexpected character in formula: '" + string( 1, c ) + "'" );
    }
}


vector<string> parseFormula( const string &formula ) {
    vector<string> segments;
    {
        string buf;
        for (char ch : formula)
        {
            if (ch == '.' || ch == '·')
            {
                if (!buf.empty())
                {
                    segments.push_back( buf ); buf.clear();
                }
            }
            else                     buf.push_back( ch );
        }
        if (!buf.empty()) segments.push_back( buf );
    }

    std::unordered_map<string, int> total;

    for (std::size_t idx = 0; idx < segments.size(); ++idx)
    {
        const string &seg = segments[ idx ];
        std::size_t i = 0;
        std::size_t leading = (idx == 0) ? 1 : readNumber( seg, i );   
        parseInto( seg, i, 0, total, leading );
    }

    vector<string> atoms;
    atoms.reserve( total.size() * 2 );                 // rough guess
    for (auto &kv : total)
        atoms.insert( atoms.end(), kv.second, kv.first );

    return atoms;
}
