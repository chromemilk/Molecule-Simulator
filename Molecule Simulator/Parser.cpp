#include "Parser.h"
#include <cctype>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

using std::string;
using std::vector;

static std::size_t readNumber( const string &s, std::size_t &i );
static void parseInto( const string &s, std::size_t &i, char endTok,
    std::unordered_map<string, int> &counts,
    std::size_t scopeMult );


ParsedFormula parseFormulaFull( const string &formula ) {

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
            else                         buf.push_back( ch );
        }
        if (!buf.empty()) segments.push_back( buf );
    }

    std::unordered_map<string, int> total;
    for (std::size_t idx = 0; idx < segments.size(); ++idx)
    {
        const string &seg = segments[ idx ];
        std::size_t i = 0;
        std::size_t leading = (idx == 0) ? 1 : readNumber( seg, i );  // stoichiometric multiplier
        parseInto( seg, i, 0, total, leading );                     

        while (i < seg.size() && seg[ i ] == '^') ++i;               // skip optional caret
        if (i < seg.size() && (seg[ i ] == '+' || seg[ i ] == '-'))
        {
            char sign = seg[ i++ ];
            std::size_t j = i;
            while (j < seg.size() && std::isdigit( seg[ j ] )) ++j;
            int mag = (j > i) ? std::stoi( seg.substr( i, j - i ) ) : 1;
            if (sign == '-') mag = -mag;
            total[ "__CHARGE__" ] += static_cast<int>( leading ) * mag;
        }
    }

    ParsedFormula out;
    out.charge = total[ "__CHARGE__" ];
    total.erase( "__CHARGE__" );

    out.atoms.reserve( total.size() * 2 );
    for (auto &kv : total)
        out.atoms.insert( out.atoms.end(), kv.second, kv.first );

    return out;
}

vector<string> parseFormula( const string &s ) {
    return parseFormulaFull( s ).atoms;
}


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
            string sym; sym.push_back( c ); ++i;
            if (i < s.size() && std::islower( s[ i ] )) sym.push_back( s[ i++ ] );
            std::size_t n = readNumber( s, i );
            counts[ sym ] += static_cast<int>( scopeMult * n );
            continue;
        }
        if (c == '(' || c == '[' || c == '{')
        {
            char close = (c == '(' ? ')' : c == '[' ? ']' : '}');
            ++i;
            std::unordered_map<string, int> sub;
            parseInto( s, i, close, sub, 1 );
            std::size_t grpMult = readNumber( s, i );
            for (auto &kv : sub)
                counts[ kv.first ] += kv.second * static_cast<int>(scopeMult * grpMult);
            continue;
        }
        if (c == '+' || c == '-' || c == '^') return;  // charge starts here
        throw std::runtime_error( "Unexpected character in formula: '" + string( 1, c ) + "'" );
    }
}
