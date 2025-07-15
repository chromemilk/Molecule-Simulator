#include "PeriodicTable.h"

// Entire periodic table with the following: Atomic symbol, atomic number, weight, valence electrions, and electronegativity

PeriodicTable::PeriodicTable() {
    table[ "H" ] = { "H",   1,   1.0080f, 1, 2.20f };
    table[ "He" ] = { "He",  2,   4.0026f, 2, 0.00f };

    table[ "Li" ] = { "Li",  3,   6.9400f, 1, 0.98f };
    table[ "Be" ] = { "Be",  4,   9.0122f, 2, 1.57f };
    table[ "B" ] = { "B",   5,  10.8100f, 3, 2.04f };
    table[ "C" ] = { "C",   6,  12.0110f, 4, 2.55f };
    table[ "N" ] = { "N",   7,  14.0070f, 5, 3.04f };
    table[ "O" ] = { "O",   8,  15.9990f, 6, 3.44f };
    table[ "F" ] = { "F",   9,  18.9980f, 7, 3.98f };
    table[ "Ne" ] = { "Ne", 10,  20.1800f, 8, 0.00f };

    table[ "Na" ] = { "Na", 11,  22.9900f, 1, 0.93f };
    table[ "Mg" ] = { "Mg", 12,  24.3050f, 2, 1.31f };
    table[ "Al" ] = { "Al", 13,  26.9820f, 3, 1.61f };
    table[ "Si" ] = { "Si", 14,  28.0850f, 4, 1.90f };
    table[ "P" ] = { "P",  15,  30.9740f, 5, 2.19f };
    table[ "S" ] = { "S",  16,  32.0600f, 6, 2.58f };
    table[ "Cl" ] = { "Cl", 17,  35.4500f, 7, 3.16f };
    table[ "Ar" ] = { "Ar", 18,  39.9480f, 8, 0.00f };

    table[ "K" ] = { "K",  19,  39.0980f, 1, 0.82f };
    table[ "Ca" ] = { "Ca", 20,  40.0780f, 2, 1.00f };

    table[ "Sc" ] = { "Sc", 21,  44.9560f, 3, 1.36f };
    table[ "Ti" ] = { "Ti", 22,  47.8670f, 2, 1.54f };
    table[ "V" ] = { "V",  23,  50.9420f, 2, 1.63f };
    table[ "Cr" ] = { "Cr", 24,  51.9960f, 2, 1.66f };
    table[ "Mn" ] = { "Mn", 25,  54.9380f, 2, 1.55f };
    table[ "Fe" ] = { "Fe", 26,  55.8450f, 2, 1.83f };
    table[ "Co" ] = { "Co", 27,  58.9330f, 2, 1.88f };
    table[ "Ni" ] = { "Ni", 28,  58.6930f, 2, 1.91f };
    table[ "Cu" ] = { "Cu", 29,  63.5460f, 1, 1.90f };
    table[ "Zn" ] = { "Zn", 30,  65.3800f, 2, 1.65f };

    table[ "Ga" ] = { "Ga", 31,  69.7230f, 3, 1.81f };
    table[ "Ge" ] = { "Ge", 32,  72.6300f, 4, 2.01f };
    table[ "As" ] = { "As", 33,  74.9220f, 5, 2.18f };
    table[ "Se" ] = { "Se", 34,  78.9710f, 6, 2.55f };
    table[ "Br" ] = { "Br", 35,  79.9040f, 7, 2.96f };
    table[ "Kr" ] = { "Kr", 36,  83.7980f, 8, 3.00f };

    table[ "Rb" ] = { "Rb", 37,  85.4680f, 1, 0.82f };
    table[ "Sr" ] = { "Sr", 38,  87.6200f, 2, 0.95f };
    table[ "Y" ] = { "Y",  39,  88.9060f, 3, 1.22f };
    table[ "Zr" ] = { "Zr", 40,  91.2240f, 2, 1.33f };
    table[ "Nb" ] = { "Nb", 41,  92.9060f, 2, 1.60f };
    table[ "Mo" ] = { "Mo", 42,  95.9500f, 2, 2.16f };
    table[ "Tc" ] = { "Tc", 43,  98.0000f, 2, 1.90f };
    table[ "Ru" ] = { "Ru", 44, 101.0700f, 2, 2.20f };
    table[ "Rh" ] = { "Rh", 45, 102.9100f, 2, 2.28f };
    table[ "Pd" ] = { "Pd", 46, 106.4200f, 2, 2.20f };
    table[ "Ag" ] = { "Ag", 47, 107.8700f, 1, 1.93f };
    table[ "Cd" ] = { "Cd", 48, 112.4100f, 2, 1.69f };
    table[ "In" ] = { "In", 49, 114.8200f, 3, 1.78f };
    table[ "Sn" ] = { "Sn", 50, 118.7100f, 4, 1.96f };
    table[ "Sb" ] = { "Sb", 51, 121.7600f, 5, 2.05f };
    table[ "Te" ] = { "Te", 52, 127.6000f, 6, 2.10f };
    table[ "I" ] = { "I",  53, 126.9000f, 7, 2.66f };
    table[ "Xe" ] = { "Xe", 54, 131.2900f, 8, 2.60f };

    table[ "Cs" ] = { "Cs", 55, 132.9100f, 1, 0.79f };
    table[ "Ba" ] = { "Ba", 56, 137.3300f, 2, 0.89f };

    table[ "La" ] = { "La", 57, 138.9100f, 3, 1.10f };
    table[ "Ce" ] = { "Ce", 58, 140.1200f, 4, 1.12f };
    table[ "Pr" ] = { "Pr", 59, 140.9100f, 5, 1.13f };
    table[ "Nd" ] = { "Nd", 60, 144.2400f, 6, 1.14f };
    table[ "Pm" ] = { "Pm", 61, 145.0000f, 7, 1.15f };
    table[ "Sm" ] = { "Sm", 62, 150.3600f, 8, 1.17f };
    table[ "Eu" ] = { "Eu", 63, 151.9600f, 2, 1.15f };
    table[ "Gd" ] = { "Gd", 64, 157.2500f, 3, 1.20f };
    table[ "Tb" ] = { "Tb", 65, 158.9300f, 4, 1.20f };
    table[ "Dy" ] = { "Dy", 66, 162.5000f, 5, 1.22f };
    table[ "Ho" ] = { "Ho", 67, 164.9300f, 6, 1.23f };
    table[ "Er" ] = { "Er", 68, 167.2600f, 7, 1.24f };
    table[ "Tm" ] = { "Tm", 69, 168.9300f, 8, 1.25f };
    table[ "Yb" ] = { "Yb", 70, 173.0500f, 2, 1.20f };
    table[ "Lu" ] = { "Lu", 71, 174.9700f, 3, 1.27f };

    table[ "Hf" ] = { "Hf", 72, 178.4900f, 2, 1.30f };
    table[ "Ta" ] = { "Ta", 73, 180.9500f, 2, 1.50f };
    table[ "W" ] = { "W",  74, 183.8400f, 2, 2.36f };
    table[ "Re" ] = { "Re", 75, 186.2100f, 2, 1.90f };
    table[ "Os" ] = { "Os", 76, 190.2300f, 2, 2.20f };
    table[ "Ir" ] = { "Ir", 77, 192.2200f, 2, 2.20f };
    table[ "Pt" ] = { "Pt", 78, 195.0800f, 2, 2.28f };
    table[ "Au" ] = { "Au", 79, 196.9700f, 1, 2.54f };
    table[ "Hg" ] = { "Hg", 80, 200.5900f, 2, 2.00f };
    table[ "Tl" ] = { "Tl", 81, 204.3800f, 3, 1.62f };
    table[ "Pb" ] = { "Pb", 82, 207.2000f, 4, 2.33f };
    table[ "Bi" ] = { "Bi", 83, 208.9800f, 5, 2.02f };
    table[ "Po" ] = { "Po", 84, 209.0000f, 6, 2.00f };
    table[ "At" ] = { "At", 85, 210.0000f, 7, 2.20f };
    table[ "Rn" ] = { "Rn", 86, 222.0000f, 8, 0.00f };

    table[ "Fr" ] = { "Fr", 87, 223.0000f, 1, 0.70f };
    table[ "Ra" ] = { "Ra", 88, 226.0000f, 2, 0.90f };

    table[ "Ac" ] = { "Ac", 89, 227.0000f, 3, 1.10f };
    table[ "Th" ] = { "Th", 90, 232.0400f, 4, 1.30f };
    table[ "Pa" ] = { "Pa", 91, 231.0400f, 5, 1.50f };
    table[ "U" ] = { "U",  92, 238.0300f, 6, 1.38f };
    table[ "Np" ] = { "Np", 93, 237.0000f, 7, 1.36f };
    table[ "Pu" ] = { "Pu", 94, 244.0000f, 8, 1.28f };
    table[ "Am" ] = { "Am", 95, 243.0000f, 2, 1.30f };
    table[ "Cm" ] = { "Cm", 96, 247.0000f, 3, 1.30f };
    table[ "Bk" ] = { "Bk", 97, 247.0000f, 4, 1.30f };
    table[ "Cf" ] = { "Cf", 98, 251.0000f, 5, 1.30f };
    table[ "Es" ] = { "Es", 99, 252.0000f, 6, 1.30f };
    table[ "Fm" ] = { "Fm",100, 257.0000f, 7, 1.30f };
    table[ "Md" ] = { "Md",101, 258.0000f, 8, 1.30f };
    table[ "No" ] = { "No",102, 259.0000f, 2, 1.30f };
    table[ "Lr" ] = { "Lr",103, 266.0000f, 3, 1.30f };

    // super heavies
    table[ "Rf" ] = { "Rf",104, 267.0000f, 2, 0.00f };
    table[ "Db" ] = { "Db",105, 268.0000f, 2, 0.00f };
    table[ "Sg" ] = { "Sg",106, 269.0000f, 2, 0.00f };
    table[ "Bh" ] = { "Bh",107, 270.0000f, 2, 0.00f };
    table[ "Hs" ] = { "Hs",108, 269.0000f, 2, 0.00f };
    table[ "Mt" ] = { "Mt",109, 278.0000f, 2, 0.00f };
    table[ "Ds" ] = { "Ds",110, 281.0000f, 2, 0.00f };
    table[ "Rg" ] = { "Rg",111, 282.0000f, 1, 0.00f };
    table[ "Cn" ] = { "Cn",112, 285.0000f, 2, 0.00f };
    table[ "Nh" ] = { "Nh",113, 286.0000f, 3, 0.00f };
    table[ "Fl" ] = { "Fl",114, 289.0000f, 4, 0.00f };
    table[ "Mc" ] = { "Mc",115, 290.0000f, 5, 0.00f };
    table[ "Lv" ] = { "Lv",116, 293.0000f, 6, 0.00f };
    table[ "Ts" ] = { "Ts",117, 294.0000f, 7, 0.00f };
    table[ "Og" ] = { "Og",118, 294.0000f, 8, 0.00f };
}

PeriodicTable &PeriodicTable::Instance() {
    static PeriodicTable inst;
    return inst;
}
const Element &PeriodicTable::Get( const std::string &sym ) const {
    return table.at( sym );
}