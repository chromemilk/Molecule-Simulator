#include "Parser.h"
#include <unordered_map>
#include <cctype>

std::vector<std::string> parseFormula( const std::string &f ) {
    struct Frame
    {
        std::unordered_map<std::string, int> mult;
    };
    std::vector<Frame> st( 1 );
    auto mul_into_parent = [&]( Frame &frm, int k ) {
        for (auto &p : frm.mult) st.back().mult[ p.first ] += p.second * k;
        };
    for (size_t i = 0; i < f.size();)
    {
        char c = f[ i ];
        if (c == '(' || c == '[')
        {
            st.emplace_back(); ++i; continue;
        }
        if (c == ')' || c == ']')
        {
            ++i; std::string num;
            while (i < f.size() && isdigit( f[ i ] )) num += f[ i++ ];
            mul_into_parent( st.back(), num.empty() ? 1 : std::stoi( num ) );
            st.pop_back(); continue;
        }
        if (isupper( c ))
        {
            std::string elm{ c }; ++i;
            if (i < f.size() && islower( f[ i ] )) elm.push_back( f[ i++ ] );
            std::string num;
            while (i < f.size() && isdigit( f[ i ] )) num.push_back( f[ i++ ] );
            st.back().mult[ elm ] += num.empty() ? 1 : std::stoi( num );
            continue;
        }
        if (c == '·' || c == '.')
        {
            ++i; continue;
        }
        if (c == '+' || c == '-' || c == '^') break;
        ++i;
    }
    std::vector<std::string> out;
    for (auto &p : st.front().mult)
        out.insert( out.end(), p.second, p.first );
    return out;
}
