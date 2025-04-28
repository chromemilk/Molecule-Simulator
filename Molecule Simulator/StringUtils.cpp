#include "StringUtils.h"

 std::string StringUtils::chargeString( int fc ) {
    if (fc == 0) return "";
    std::string s = "^";
    if (std::abs( fc ) != 1) s += std::to_string( std::abs( fc ) );
    s += (fc > 0 ? "+" : "-");
    return s;
}
