#include "Parser.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <cctype>
#include <stdexcept>

namespace
{

  
    inline std::size_t readNumber( const std::string &s, std::size_t &i ) {
        std::size_t start = i;
        while (i < s.size() && std::isdigit( s[ i ] )) ++i;
        return (i == start) ? 1 : std::stoul( s.substr( start, i - start ) );
    }

    struct Frame
    {
        std::unordered_map<std::string, int> mult;  
        std::size_t leading = 1;                   
    };

} 


std::vector<std::string> parseFormula( const std::string &f ) {
    std::vector<Frame> st{ Frame{} };            // root frame
    std::size_t i = 0;

    std::size_t pending = 0;                   

    auto flushPending = [&]() {               // reset after we have consumed it
        std::size_t v = pending ? pending : 1;
        pending = 0;
        return v;
        };

    while (i < f.size())
    {
        char c = f[ i ];

        if (std::isdigit( c ))
        {
            pending = readNumber( f, i );        
            continue;
        }

        if (c == '(' || c == '[' || c == '{')
        {
            st.push_back( Frame{} );             
            st.back().leading = flushPending();
            ++i;
            continue;
        }

        if (c == ')' || c == ']' || c == '}')
        {
            ++i;
            std::size_t trailing = readNumber( f, i );     // number after the ')', default 1
            std::size_t factor = st.back().leading * trailing;

            auto grp = std::move( st.back().mult );
            st.pop_back();
            for (auto &kv : grp)
                st.back().mult[ kv.first ] += kv.second * static_cast<int>(factor);
            continue;
        }

        if (std::isupper( c ))
        {
            std::string sym;
            sym.push_back( c ); ++i;
            if (i < f.size() && std::islower( f[ i ] )) sym.push_back( f[ i++ ] );

            std::size_t count = readNumber( f, i );
            std::size_t factor = flushPending();

            st.back().mult[ sym ] += static_cast<int>( factor * count );
            continue;
        }

        if (c == '·' || c == '.')
        {
            ++i;
            continue;                          
        }

      
        if (c == '+' || c == '-' || c == '^')
            break;

        throw std::runtime_error( std::string( "Unexpected character in formula: '" ) + c + "'" );
    }

    std::vector<std::string> out;
    for (auto &kv : st.front().mult)
        out.insert( out.end(), kv.second, kv.first );

    return out;
}