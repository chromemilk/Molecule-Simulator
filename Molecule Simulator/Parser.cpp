#include "Parser.h"
#include <cctype>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>

using std::string;
using std::vector;

static std::size_t readNumber( const string &s, std::size_t &i );
static void parseInto( const string &s, std::size_t &i, char endTok,
    std::unordered_map<string, int> &counts,
    std::size_t scopeMult );
static int parseChargeSuffix( const string &seg, std::size_t i );

static string trimCopy( string s ) {
    auto notWs = []( unsigned char ch ) { return !std::isspace( ch ); };
    s.erase( s.begin(), std::find_if( s.begin(), s.end(), notWs ) );
    s.erase( std::find_if( s.rbegin(), s.rend(), notWs ).base(), s.end() );
    return s;
}

static string extractPrimarySpecies( const string &input ) {
    string s = trimCopy( input );
    if (s.empty()) return s;

    auto findArrow = [&]( const string &tok ) {
        return s.find( tok );
    };

    std::size_t arrowPos = string::npos;
    std::size_t arrowLen = 0;
    for (const string tok : { string( "<=>" ), string( "<->" ), string( "->" ), string( "=>" ), string( "=" ) })
    {
        std::size_t p = findArrow( tok );
        if (p != string::npos && (arrowPos == string::npos || p < arrowPos))
        {
            arrowPos = p;
            arrowLen = tok.size();
        }
    }

    string side = (arrowPos == string::npos) ? s : s.substr( 0, arrowPos );
    side = trimCopy( side );
    if (side.empty() && arrowPos != string::npos)
        side = trimCopy( s.substr( arrowPos + arrowLen ) );

    std::size_t plus = side.find( '+' );
    if (plus != string::npos)
        side = trimCopy( side.substr( 0, plus ) );

    return side;
}


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
        const string seg = trimCopy( segments[ idx ] );
        if (seg.empty()) continue;

        std::size_t i = 0;
        std::size_t leading = readNumber( seg, i );  // stoichiometric multiplier (supports first segment too)
        parseInto( seg, i, 0, total, leading );

        total[ "__CHARGE__" ] += static_cast<int>( leading ) * parseChargeSuffix( seg, i );
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
    while (i < s.size() && std::isspace( static_cast<unsigned char>( s[ i ] ) )) ++i;
    std::size_t start = i;
    while (i < s.size() && std::isdigit( static_cast<unsigned char>( s[ i ] ) )) ++i;
    return (i == start) ? 1 : std::stoul( s.substr( start, i - start ) );
}

static void parseInto( const string &s, std::size_t &i, char endTok,
    std::unordered_map<string, int> &counts,
    std::size_t scopeMult ) {
    while (i < s.size())
    {
        while (i < s.size() && std::isspace( static_cast<unsigned char>( s[ i ] ) )) ++i;
        if (i >= s.size()) break;

        char c = s[ i ];
        if (endTok && c == endTok)
        {
            ++i; return;
        }

        if (std::isupper( static_cast<unsigned char>( c ) ))
        {
            string sym; sym.push_back( c ); ++i;
            if (i < s.size() && std::islower( static_cast<unsigned char>( s[ i ] ) )) sym.push_back( s[ i++ ] );
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
        if (c == ')' || c == ']' || c == '}')
            throw std::runtime_error( "Unmatched closing bracket in formula" );
        if (c == '+' || c == '-' || c == '^') return;  // charge starts here
        throw std::runtime_error( "Unexpected character in formula: '" + string( 1, c ) + "'" );
    }

    if (endTok)
        throw std::runtime_error( "Unclosed bracket group in formula" );
}

static int parseChargeSuffix( const string &seg, std::size_t i ) {
    while (i < seg.size() && std::isspace( static_cast<unsigned char>( seg[ i ] ) )) ++i;
    if (i >= seg.size()) return 0;

    string rest = trimCopy( seg.substr( i ) );
    if (rest.empty()) return 0;
    if (rest.front() == '^')
    {
        rest.erase( rest.begin() );
        rest = trimCopy( rest );
    }
    if (rest.empty()) return 0;

    auto parseDigits = [&]( std::size_t &k ) {
        std::size_t st = k;
        while (k < rest.size() && std::isdigit( static_cast<unsigned char>( rest[ k ] ) )) ++k;
        return (k > st) ? std::stoi( rest.substr( st, k - st ) ) : 0;
    };

    std::size_t k = 0;
    if (rest[ 0 ] == '+' || rest[ 0 ] == '-')
    {
        char sign = rest[ 0 ];
        ++k;
        int mag = parseDigits( k );
        if (k != rest.size()) return 0;
        if (mag == 0) mag = 1;
        return sign == '-' ? -mag : mag;
    }

    int mag = parseDigits( k );
    if (mag > 0 && k < rest.size() && (rest[ k ] == '+' || rest[ k ] == '-') && k + 1 == rest.size())
        return rest[ k ] == '-' ? -mag : mag;

    return 0;
}
