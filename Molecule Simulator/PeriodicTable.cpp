#include "PeriodicTable.h"
PeriodicTable::PeriodicTable() {
    table[ "H" ] = { "H", 1,   1.008f, 1, 2.20f };
    table[ "C" ] = { "C", 6,  12.011f, 4, 2.55f };
    table[ "N" ] = { "N", 7,  14.007f, 5, 3.04f };
    table[ "O" ] = { "O", 8,  15.999f, 6, 3.44f };
}
PeriodicTable &PeriodicTable::Instance() {
    static PeriodicTable inst;
    return inst;
}
const Element &PeriodicTable::Get( const std::string &sym ) const {
    return table.at( sym );
}