#pragma once
#include <string>
#include <unordered_map>

struct Element
{
    std::string symbol;
    int atomicNumber;
    float atomicMass;
    int valenceElectrons;
    float electronegativity;
};

class PeriodicTable
{
public:
    static PeriodicTable &Instance();
    const Element &Get( const std::string &symbol ) const;
private:
    PeriodicTable();
    std::unordered_map<std::string, Element> table;
};