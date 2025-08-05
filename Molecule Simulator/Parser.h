#pragma once
#include <string>
#include <vector>

struct ParsedFormula
{
    std::vector<std::string> atoms;   // expanded list of element symbols
    int  charge = 0;                  // algebraic charge (+2, -1, 0 …)
};

ParsedFormula           parseFormulaFull( const std::string & );

std::vector<std::string> parseFormula( const std::string & );
