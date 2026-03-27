#include "UI.h"
#include <glm/glm.hpp>
#include "Parser.h"
#include "resonance.h"
#include "PeriodicTable.h"
#include "tinyfiledialogs.h"
#include "Tests.h"
#include "ImGuiHelpers.h"
#include <algorithm>
#include <queue>
using namespace CustomMenu;

namespace
{
    void buildHCN( InputContext &c ) {
        auto &s = *c.atoms; s.build( { "H","C","N" }, { {0,1,1},{1,2,3} } ); c.currentPrebuiltAtom = "HCN";
    }
    void buildNO2m( InputContext &c ) {
        auto &s = *c.atoms; s.build( { "O","N","O" }, { {1,0,2},{1,2,1} } ); c.currentPrebuiltAtom = "NO2-";
    }
    void buildCO2( InputContext &c ) {
        auto &s = *c.atoms; s.build( { "O","C","O" }, { {1,0,2},{1,2,2} } ); c.currentPrebuiltAtom = "CO2";
    }
    void buildH2O( InputContext &c ) {
        auto &s = *c.atoms; s.build( { "H","O","H" }, { {1,0,1},{1,2,1} } ); c.currentPrebuiltAtom = "H2O";
    }
    void buildNH3( InputContext &c ) {
        auto &s = *c.atoms; s.build( { "N","H","H","H" }, { {0,1,1},{0,2,1},{0,3,1} } ); c.currentPrebuiltAtom = "NH3";
    }
    void buildCH4( InputContext &c ) {
        auto &s = *c.atoms; s.build( { "C","H","H","H","H" }, { {0,1,1},{0,2,1},{0,3,1},{0,4,1} } ); c.currentPrebuiltAtom = "CH4";
    }
    void buildO3( InputContext &c ) {
        auto &s = *c.atoms; s.build( { "O","O","O" }, { {0,1,2},{1,2,1} } ); c.currentPrebuiltAtom = "O3";
    }
    void buildCNminus( InputContext &c ) {
        auto &s = *c.atoms; s.build( { "C","N" }, { {0,1,3} } ); c.currentPrebuiltAtom = "CN-";
    }
    void buildO2( InputContext &c ) {
        auto &s = *c.atoms; s.build( { "O","O" }, { {0,1,2} } ); c.currentPrebuiltAtom = "O2";
    }
    void buildN2( InputContext &c ) {
        auto &s = *c.atoms; s.build( { "N","N" }, { {0,1,3} } ); c.currentPrebuiltAtom = "N2";
    }
    void buildH2( InputContext &c ) {
        auto &s = *c.atoms; s.build( { "H","H" }, { {0,1,1} } ); c.currentPrebuiltAtom = "H2";
    }
    void buildCO( InputContext &c ) {
        auto &s = *c.atoms; s.build( { "C","O" }, { {0,1,3} } ); c.currentPrebuiltAtom = "CO";
    }
    void buildHCl( InputContext &c ) {
        auto &s = *c.atoms; s.build( { "H","Cl" }, { {0,1,1} } ); c.currentPrebuiltAtom = "HCl";
    }
    void buildHF( InputContext &c ) {
        auto &s = *c.atoms; s.build( { "H","F" }, { {0,1,1} } ); c.currentPrebuiltAtom = "HF";
    }
    void buildSO2( InputContext &c ) {
        auto &s = *c.atoms; s.build( { "O","S","O" }, { {1,0,2},{1,2,2} } ); c.currentPrebuiltAtom = "SO2";
    }
    void buildSO3( InputContext &c ) {
        auto &s = *c.atoms; s.build( { "O","S","O","O" }, { {1,0,2},{1,2,2},{1,3,2} } ); c.currentPrebuiltAtom = "SO3";
    }
    void buildH2S( InputContext &c ) {
        auto &s = *c.atoms; s.build( { "H","S","H" }, { {1,0,1},{1,2,1} } ); c.currentPrebuiltAtom = "H2S";
    }
    void buildNH4p( InputContext &c ) { // ammonium
        auto &s = *c.atoms; s.build( { "N","H","H","H","H" }, { {0,1,1},{0,2,1},{0,3,1},{0,4,1} } ); c.currentPrebuiltAtom = "NH4+";
    }
    void buildNO3m( InputContext &c ) { // nitrate
        auto &s = *c.atoms; s.build( { "O","N","O","O" }, { {1,0,2},{1,2,1},{1,3,1} } ); c.currentPrebuiltAtom = "NO3-";
    }
    void buildSO4m2( InputContext &c ) { // sulfate
        auto &s = *c.atoms; s.build( { "O","S","O","O","O" }, { {1,0,2},{1,2,2},{1,3,1},{1,4,1} } ); c.currentPrebuiltAtom = "SO4^2-";
    }
    void buildHCO3m( InputContext &c ) { // bicarbonate
        // Atoms: H(0), C(1), O(2)= (carbonyl), O(3)-H(4) (hydroxyl), O(5) (single)
        auto &s = *c.atoms; s.build( { "H","C","O","O","H","O" },
            { {1,2,2}, {1,3,1}, {3,4,1}, {1,5,1} } ); c.currentPrebuiltAtom = "HCO3-";
    }
    void buildCH3OH( InputContext &c ) { // methanol
        // C(0)-O(1); C-H:2,3,4; O-H:5
        auto &s = *c.atoms; s.build( { "C","O","H","H","H","H" },
            { {0,1,1},{0,2,1},{0,3,1},{0,4,1},{1,5,1} } ); c.currentPrebuiltAtom = "CH3OH";
    }
    void buildC2H6( InputContext &c ) { // ethane
        auto &s = *c.atoms; s.build( { "C","C","H","H","H","H","H","H" },
            { {0,1,1},{0,2,1},{0,3,1},{0,4,1},{1,5,1},{1,6,1},{1,7,1} } ); c.currentPrebuiltAtom = "C2H6";
    }
    void buildC2H4( InputContext &c ) { // ethene
        auto &s = *c.atoms; s.build( { "C","C","H","H","H","H" },
            { {0,1,2},{0,2,1},{0,3,1},{1,4,1},{1,5,1} } ); c.currentPrebuiltAtom = "C2H4";
    }
    void buildC2H2( InputContext &c ) { // ethyne
        auto &s = *c.atoms; s.build( { "C","C","H","H" },
            { {0,1,3},{0,2,1},{1,3,1} } ); c.currentPrebuiltAtom = "C2H2";
    }
    void buildH2CO( InputContext &c ) { // formaldehyde
        auto &s = *c.atoms; s.build( { "C","O","H","H" },
            { {0,1,2},{0,2,1},{0,3,1} } ); c.currentPrebuiltAtom = "H2CO";
    }
    void buildHCOOH( InputContext &c ) { // formic acid (H-COOH)
        // Atoms: H(0)-C(1)=O(2); C(1)-O(3)-H(4)
        auto &s = *c.atoms; s.build( { "H","C","O","O","H" },
            { {1,0,1},{1,2,2},{1,3,1},{3,4,1} } ); c.currentPrebuiltAtom = "HCOOH";
    }
    void buildC6H6( InputContext &c ) { // benzene (alternating double bonds)
        // C ring 0..5, H 6..11
        auto &s = *c.atoms; s.build(
            { "C","C","C","C","C","C","H","H","H","H","H","H" },
            {
                {0,1,2},{1,2,1},{2,3,2},{3,4,1},{4,5,2},{5,0,1},
                {0,6,1},{1,7,1},{2,8,1},{3,9,1},{4,10,1},{5,11,1}
            }
        );
        c.currentPrebuiltAtom = "C6H6";
    }
}

static std::vector<int> OldToHeavyFirst( const std::vector<std::string> &syms ) {
    const int n = (int)syms.size();
    int heavyCount = 0; for (auto &s : syms) if (s != "H") ++heavyCount;
    std::vector<int> old2new( n, -1 );
    int h = 0, k = heavyCount;
    for (int i = 0; i < n; ++i)
    {
        if (syms[ i ] != "H") old2new[ i ] = h++;
        else old2new[ i ] = k++;
    }
    return old2new;
}

static std::vector<std::tuple<int, int, int>> RemapBonds( const std::vector<std::tuple<int, int, int>> &bonds, const std::vector<int> &old2new ) {
    std::vector<std::tuple<int, int, int>> out; out.reserve( bonds.size() );
    for (auto [a, b, o] : bonds) out.emplace_back( old2new[ a ], old2new[ b ], o );
    return out;
}

static std::vector<std::tuple<int, int, int>> Canon( const std::vector<std::tuple<int, int, int>> &bonds ) {
    std::vector<std::tuple<int, int, int>> v; v.reserve( bonds.size() );
    for (auto [a, b, o] : bonds)
    {
        if (a > b) std::swap( a, b );
        v.emplace_back( a, b, o );
    }
    std::sort( v.begin(), v.end() );
    return v;
}


static int ParseTrailingCharge( const std::string &s ) {
    if (s.empty()) return 0;
    int n = (int)s.size();
    int sign = 0;
    if (s[ n - 1 ] == '+') sign = +1; else if (s[ n - 1 ] == '-') sign = -1; else return 0;
    int mag = 1;
    if (n >= 3 && std::isdigit( (unsigned char)s[ n - 2 ] )) mag = s[ n - 2 ] - '0';
    return sign * mag;
}

static std::vector<std::string> CollectSymbolsFromScene( const AtomSystem &atoms ) {
    std::vector<std::string> syms; syms.reserve( atoms.getAtoms().size() );
    for (const Atom &a : atoms.getAtoms()) syms.push_back( a.type );
    return syms;
}

static std::vector<std::string> HeavyFirst( const std::vector<std::string> &syms ) {
    std::vector<std::string> out; out.reserve( syms.size() );
    for (const auto &s : syms) if (s != "H") out.push_back( s );
    for (const auto &s : syms) if (s == "H") out.push_back( s );
    return out;
}

static void DrawLewisMini( const std::vector<std::string> &symsHF,
    const std::vector<std::tuple<int, int, int>> &bonds,
    const ImVec2 &canvasSize,
    float fontScale = 0.9f ) {
    ImDrawList *dl = ImGui::GetWindowDrawList();
    ImVec2 p0 = ImGui::GetCursorScreenPos();
    ImVec2 p1 = ImVec2( p0.x + canvasSize.x, p0.y + canvasSize.y );
    dl->AddRectFilled( p0, p1, IM_COL32( 30, 30, 38, 255 ), 8.0f );
    dl->AddRect( p0, p1, IM_COL32( 60, 60, 70, 255 ), 8.0f );

    const int N = (int)symsHF.size();
    if (N == 0)
    {
        ImGui::Dummy( canvasSize ); return;
    }

    int heavyCnt = 0; for (auto &s : symsHF) if (s != "H") ++heavyCnt;

    const float pad = 10.0f;
    ImVec2 c = ImVec2( p0.x + canvasSize.x * 0.5f, p0.y + canvasSize.y * 0.52f );
    float R = std::min( (canvasSize.x - 2 * pad), (canvasSize.y - 2 * pad) ) * 0.38f;
    R = std::max( 26.0f, R );
    if (heavyCnt <= 2) R *= 0.75f;

    std::vector<ImVec2> pos( N );

    float a0 = -IM_PI * 0.5f;
    for (int i = 0, k = 0; i < N; ++i)
    {
        if (symsHF[ i ] == "H") continue;
        float ang = a0 + (heavyCnt ? (2.0f * IM_PI * (float)k / std::max( 1, heavyCnt )) : 0.0f);
        pos[ i ] = ImVec2( c.x + R * cosf( ang ), c.y + R * sinf( ang ) );
        ++k;
    }

    for (int hi = 0; hi < N; ++hi) if (symsHF[ hi ] == "H")
    {
        int neighbor = -1;
        for (auto [a, b, o] : bonds)
        {
            if (a == hi && symsHF[ b ] != "H")
            {
                neighbor = b; break;
            }
            if (b == hi && symsHF[ a ] != "H")
            {
                neighbor = a; break;
            }
        }
        if (neighbor >= 0)
        {
            ImVec2 rc = ImVec2( pos[ neighbor ].x - c.x, pos[ neighbor ].y - c.y );
            float len = sqrtf( rc.x * rc.x + rc.y * rc.y );
            ImVec2 dir = (len > 1e-3f) ? ImVec2( rc.x / len, rc.y / len ) : ImVec2( 0, -1 );
            float rH = std::max( 10.0f, R * 0.45f );
            pos[ hi ] = ImVec2( pos[ neighbor ].x + dir.x * rH, pos[ neighbor ].y + dir.y * rH );
        }
        else
        {
            pos[ hi ] = ImVec2( c.x, c.y - R * 0.6f );
        }
    }

    auto drawBond = [&]( int a, int b, int order ) {
        ImVec2 A = pos[ a ], B = pos[ b ];
        ImVec2 v = ImVec2( B.x - A.x, B.y - A.y );
        float L = sqrtf( v.x * v.x + v.y * v.y ); if (L < 1e-3f) return;
        ImVec2 u = ImVec2( v.x / L, v.y / L );
        ImVec2 n = ImVec2( -u.y, u.x );
        float gapA = (symsHF[ a ] == "H") ? 6.0f : 9.0f;
        float gapB = (symsHF[ b ] == "H") ? 6.0f : 9.0f;
        ImVec2 A2 = ImVec2( A.x + u.x * gapA, A.y + u.y * gapA );
        ImVec2 B2 = ImVec2( B.x - u.x * gapB, B.y - u.y * gapB );
        float off = 3.0f, thick = 2.0f;
        if (order == 1)
        {
            dl->AddLine( A2, B2, IM_COL32( 220, 220, 220, 255 ), thick );
        }
        else if (order == 2)
        {
            dl->AddLine( ImVec2( A2.x + n.x * off, A2.y + n.y * off ), ImVec2( B2.x + n.x * off, B2.y + n.y * off ), IM_COL32( 220, 220, 220, 255 ), thick );
            dl->AddLine( ImVec2( A2.x - n.x * off, A2.y - n.y * off ), ImVec2( B2.x - n.x * off, B2.y - n.y * off ), IM_COL32( 220, 220, 220, 255 ), thick );
        }
        else if (order == 3)
        {
            dl->AddLine( A2, B2, IM_COL32( 220, 220, 220, 255 ), thick );
            dl->AddLine( ImVec2( A2.x + n.x * 2.0f * off, A2.y + n.y * 2.0f * off ), ImVec2( B2.x + n.x * 2.0f * off, B2.y + n.y * 2.0f * off ), IM_COL32( 220, 220, 220, 255 ), thick );
            dl->AddLine( ImVec2( A2.x - n.x * 2.0f * off, A2.y - n.y * 2.0f * off ), ImVec2( B2.x - n.x * 2.0f * off, B2.y - n.y * 2.0f * off ), IM_COL32( 220, 220, 220, 255 ), thick );
        }
        };
    for (auto [a, b, o] : bonds) drawBond( a, b, o );

    ImGui::PushClipRect( p0, p1, true );
    float fs = ImGui::GetFontSize() * fontScale;
    for (int i = 0; i < N; ++i)
    {
        ImVec2 t = pos[ i ];
        std::string lab = symsHF[ i ];
        ImVec2 ts = ImGui::CalcTextSize( lab.c_str() );
        dl->AddText( ImVec2( t.x - ts.x * 0.5f, t.y - ts.y * 0.5f ), IM_COL32( 255, 255, 255, 255 ), lab.c_str() );
    }
    ImGui::PopClipRect();

    ImGui::Dummy( canvasSize );
}

struct ResonanceDiagnostics
{
    int absFormalCharge = 0;
    int components = 0;
    int heavyComponents = 0;
    int heavyRings = 0;
    int octetPenalty = 0;
    int chargePlacementPenalty = 0;
    int multibondPenalty = 0;
};

static int desiredElectronsForUI( const std::string &s ) {
    if (s == "H") return 2;
    if (s == "B" || s == "Al" || s == "Ga" || s == "In" || s == "Tl") return 6;
    if (s == "P" || s == "S" || s == "Cl" || s == "Br" || s == "I"
        || s == "Se" || s == "Te" || s == "As" || s == "Sb" || s == "Xe") return 12;
    return 8;
}

static ResonanceDiagnostics AnalyzeResonanceStructure(
    const std::vector<std::string> &syms,
    const std::vector<std::tuple<int, int, int>> &bonds ) {
    ResonanceDiagnostics d;
    const int n = (int)syms.size();
    if (n == 0) return d;

    std::vector<int> bondSum( n, 0 );
    std::vector<char> isHeavy( n, 0 );
    std::vector<std::vector<std::pair<int, int>>> adj( n );
    int heavyAtoms = 0;
    int heavyEdges = 0;

    for (int i = 0; i < n; ++i)
    {
        isHeavy[ i ] = (syms[ i ] != "H");
        if (isHeavy[ i ]) ++heavyAtoms;
    }

    for (auto [a, b, o] : bonds)
    {
        if (a < 0 || b < 0 || a >= n || b >= n) continue;
        bondSum[ a ] += o;
        bondSum[ b ] += o;
        adj[ a ].push_back( { b, o } );
        adj[ b ].push_back( { a, o } );
        d.multibondPenalty += (o - 1) * (o - 1);
        if (isHeavy[ a ] && isHeavy[ b ]) ++heavyEdges;
    }

    auto countComps = [&]( bool heavyOnly ) {
        std::vector<char> seen( n, 0 );
        int comps = 0;
        for (int s = 0; s < n; ++s)
        {
            if (seen[ s ]) continue;
            if (heavyOnly && !isHeavy[ s ]) continue;
            std::queue<int> q;
            q.push( s );
            seen[ s ] = 1;
            ++comps;
            while (!q.empty())
            {
                int u = q.front(); q.pop();
                for (auto [v, _] : adj[ u ])
                {
                    if (heavyOnly && !isHeavy[ v ]) continue;
                    if (!seen[ v ])
                    {
                        seen[ v ] = 1;
                        q.push( v );
                    }
                }
            }
        }
        return comps;
        };

    d.components = countComps( false );
    d.heavyComponents = (heavyAtoms > 0) ? countComps( true ) : 0;
    d.heavyRings = std::max( 0, heavyEdges - heavyAtoms + d.heavyComponents );

    for (int i = 0; i < n; ++i)
    {
        const std::string &s = syms[ i ];
        const auto &elem = PeriodicTable::Instance().Get( s );
        const int desired = desiredElectronsForUI( s );
        const int lone = std::max( 0, desired - 2 * bondSum[ i ] );
        const int owned = bondSum[ i ] + lone;
        const int fc = elem.valenceElectrons - owned;
        d.absFormalCharge += std::abs( fc );

        if (elem.atomicNumber <= 10)
        {
            const int tgt = (s == "H") ? 2 : 8;
            d.octetPenalty += std::abs( owned - tgt );
        }

        const int enScaled = int( elem.electronegativity * 10.0f );
        if (fc < 0) d.chargePlacementPenalty += std::max( 0, 32 - enScaled );
        if (fc > 0) d.chargePlacementPenalty += std::max( 0, enScaled - 20 );
    }

    return d;
}


static void ShowResonanceTab( InputContext &ctx, float CARD_W ) {
    AtomSystem &atoms = *ctx.atoms;

    auto card = BeginCenteredCard( "Resonance structures", CARD_W );

    static bool enabled = true;
    CustomMenu::CustomCheckbox( "Enable resonance", &enabled );
	ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();

    if (!enabled)
    {
        EndCenteredCard( card ); ImGui::EndTabItem(); return;
    }

    static float topK = 6; if (topK < 1) topK = 1; if (topK > 36) topK = 36;
    CustomMenu::CustomSliderFloat( "Top K", ImGuiDataType_Float, &topK, 1, 6, "%.0f", IM_COL32( 76, 152, 220, 255 ), ImGuiSliderFlags_None, 220 );

    static int searchLevel = (int)Resonance::SearchCaps::Balanced;
    static float maxNodes = 75000.0f;
    static float maxStructs = 192.0f;
    ImGui::Text( "Resonance Search" );
    ImGui::Spacing();
    ImGui::RadioButton( "Fast", &searchLevel, (int)Resonance::SearchCaps::Fast ); ImGui::SameLine();
    ImGui::RadioButton( "Balanced", &searchLevel, (int)Resonance::SearchCaps::Balanced ); ImGui::SameLine();
    ImGui::RadioButton( "Exhaustive", &searchLevel, (int)Resonance::SearchCaps::Exhaustive );
    CustomMenu::CustomSliderFloat( "Max nodes", ImGuiDataType_Float, &maxNodes, 10000, 300000, "%.0f", IM_COL32( 76, 152, 220, 255 ), ImGuiSliderFlags_None, 240 );
    CustomMenu::CustomSliderFloat( "Max structures", ImGuiDataType_Float, &maxStructs, 32, 1024, "%.0f", IM_COL32( 76, 152, 220, 255 ), ImGuiSliderFlags_None, 240 );

    static float netCharge = 0; 
    if (ctx.currentPrebuiltAtom.size() && ImGui::IsWindowAppearing())
    {
        netCharge = ParseTrailingCharge( ctx.currentPrebuiltAtom );
    }
    CustomMenu::CustomSliderFloat( "Net charge", ImGuiDataType_Float, &netCharge, -4, +4, "%.0f", IM_COL32( 76, 152, 220, 255 ), ImGuiSliderFlags_None, 220 );

    ImGui::Separator();
    std::vector<std::string> syms = CollectSymbolsFromScene( atoms );
    if (syms.empty())
    {
        ImGui::TextDisabled( "No molecule in the scene." );
        EndCenteredCard( card );
        ImGui::EndTabItem();
        return;
    }

    static std::string cachedKey;
    static std::vector<std::vector<std::tuple<int, int, int>>> cached;
    static int lastNodesVisited = 0;
    static int lastGeneratedCount = 0;
    static int selectedStructure = 0;

    auto sig = [&]( const std::vector<std::string> &S ) {
        std::string k; k.reserve( S.size() * 3 );
        for (auto &s : S)
        {
            k += s; k += ',';
        }
        return k;
        };
    static std::string lastLabel, lastSymsSig;
    std::string symsSig = sig( syms );
    if (ctx.currentPrebuiltAtom != lastLabel || symsSig != lastSymsSig)
    {
        int parsed = ParseTrailingCharge( ctx.currentPrebuiltAtom );
        if (ctx.currentPrebuiltAtom.find( '+' ) != std::string::npos ||
            ctx.currentPrebuiltAtom.find( '-' ) != std::string::npos)
        {
            netCharge = parsed;
        }
        lastLabel = ctx.currentPrebuiltAtom;
        lastSymsSig = symsSig;
        cachedKey.clear();
    }

    const std::string key = symsSig + "|Q=" + std::to_string( netCharge )
        + "|L=" + std::to_string( searchLevel )
        + "|N=" + std::to_string( (int)maxNodes )
        + "|S=" + std::to_string( (int)maxStructs );
    bool doGen = (key != cachedKey) || ImGui::Button( "Generate / Refresh" );
    if (doGen)
    {
        cachedKey = key;
        cached.clear();
        selectedStructure = 0;
        try
        {
            Resonance::Generator gen( syms, netCharge );
            Resonance::SearchCaps caps;
            caps.level = (Resonance::SearchCaps::Level)searchLevel;
            caps.maxNodes = (int)maxNodes;
            caps.maxStructures = (int)maxStructs;
            gen.setSearchCaps( caps );

            auto allOrig = gen.generateStructuresOriginal();
            cached = allOrig;
            lastNodesVisited = gen.nodesVisited();
            lastGeneratedCount = (int)allOrig.size();

            auto Canonize = []( const std::vector<std::tuple<int, int, int>> &bonds ) {
                std::vector<std::tuple<int, int, int>> v; v.reserve( bonds.size() );
                for (auto [a, b, o] : bonds)
                {
                    if (a > b) std::swap( a, b ); v.emplace_back( a, b, o );
                }
                std::sort( v.begin(), v.end() );
                return v;
                };
            auto bestOrig = gen.bestStructure();
            auto bestC = Canonize( bestOrig );
            int bestIdx = -1;
            for (int i = 0; i < (int)cached.size(); ++i)
                if (Canonize( cached[ i ] ) == bestC)
                {
                    bestIdx = i; break;
                }
            if (bestIdx > 0) std::rotate( cached.begin(), cached.begin() + bestIdx, cached.begin() + bestIdx + 1 );
        }
        catch (const std::exception &e)
        {
            ImGui::TextColored( ImVec4( 1, 0.5f, 0.5f, 1 ), "Resonance generation failed: %s", e.what() );
        }
    }

    ImGui::Text( "Generated: %d  |  Search nodes: %d", lastGeneratedCount, lastNodesVisited );

    if (cached.empty())
    {
        ImGui::TextDisabled( "No valid resonance structures for this molecule/charge." );
        EndCenteredCard( card );
        ImGui::EndTabItem();
        return;
    }

    const int cols = 3;
    if (ImGui::BeginTable( "res_k_table", cols, ImGuiTableFlags_SizingStretchProp ))
    {
        const int showN = std::min<int>( topK, (int)cached.size() );
        for (int i = 0; i < showN; ++i)
        {
            if (i % cols == 0) ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::PushID( i );
            ImVec2 cell = ImVec2( (CARD_W - 24.0f) / cols, (CARD_W - 24.0f) / cols );
            ImGui::BeginChild( "mini", cell, true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse );
            DrawLewisMini( syms, cached[ i ], ImVec2( cell.x - 8.0f, cell.y - 28.0f ) );
            if (i == selectedStructure) ImGui::TextColored( ImVec4( 0.45f, 0.95f, 0.75f, 1.0f ), "#%d", i + 1 );
            else ImGui::Text( "%d", i + 1 );
            ImGui::EndChild();
            if (ImGui::IsItemClicked())
            {
                selectedStructure = i;
                atoms.build( syms, cached[ i ] );
            }
            ImGui::PopID();
        }
        ImGui::EndTable();
    }

    if (!cached.empty())
    {
        selectedStructure = std::clamp( selectedStructure, 0, (int)cached.size() - 1 );
        ResonanceDiagnostics rd = AnalyzeResonanceStructure( syms, cached[ selectedStructure ] );
        ImGui::Separator();
        ImGui::Text( "Selected structure: #%d", selectedStructure + 1 );
        if (ImGui::BeginTable( "res_diag_table", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_RowBg ))
        {
            ImGui::TableNextRow(); ImGui::TableSetColumnIndex( 0 ); ImGui::TextUnformatted( "|Formal charge| sum" ); ImGui::TableSetColumnIndex( 1 ); ImGui::Text( "%d", rd.absFormalCharge );
            ImGui::TableNextRow(); ImGui::TableSetColumnIndex( 0 ); ImGui::TextUnformatted( "Components (heavy)" ); ImGui::TableSetColumnIndex( 1 ); ImGui::Text( "%d (%d)", rd.components, rd.heavyComponents );
            ImGui::TableNextRow(); ImGui::TableSetColumnIndex( 0 ); ImGui::TextUnformatted( "Heavy ring count" ); ImGui::TableSetColumnIndex( 1 ); ImGui::Text( "%d", rd.heavyRings );
            ImGui::TableNextRow(); ImGui::TableSetColumnIndex( 0 ); ImGui::TextUnformatted( "Octet penalty" ); ImGui::TableSetColumnIndex( 1 ); ImGui::Text( "%d", rd.octetPenalty );
            ImGui::TableNextRow(); ImGui::TableSetColumnIndex( 0 ); ImGui::TextUnformatted( "Charge placement penalty" ); ImGui::TableSetColumnIndex( 1 ); ImGui::Text( "%d", rd.chargePlacementPenalty );
            ImGui::TableNextRow(); ImGui::TableSetColumnIndex( 0 ); ImGui::TextUnformatted( "Multiple-bond penalty" ); ImGui::TableSetColumnIndex( 1 ); ImGui::Text( "%d", rd.multibondPenalty );
            ImGui::EndTable();
        }
    }

    EndCenteredCard( card );
    ImGui::EndTabItem();
}


void ShowImGuiMenu( InputContext &ctx ) {
    AtomSystem &atoms = *ctx.atoms;

    if (!ImGui::Begin( "Molecule Menu", 0, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize ))
    {
        ImGui::End(); return;
    }

    float CARD_W = ImMin( 520.0f, ImGui::GetContentRegionAvail().x ); // max 520, otherwise fill

    if (ImGui::BeginTabBar( "Tabs" ))
    {
        if (ImGui::BeginTabItem( "Main" ))
        {
            {
                auto card = BeginCenteredCard( "Create / Spawn", CARD_W, false );
                static char sym[ 8 ]{};
                ImGui::Text( "Insert Atom" );
                ImGui::InputText( "##1", sym, sizeof( sym ) );
                if (ImGui::Button( "Spawn Atom" ) && sym[ 0 ])
                {
                    glm::vec3 dir = ScreenRay( ctx, ctx.winWidth / 2, ctx.winHeight / 2 );
                    float t = (0.f - ctx.camera->Position.y) / dir.y;
                    atoms.spawnAtom( sym, ctx.camera->Position + dir * t );
                    sym[ 0 ] = '\0';
                }

                ImGui::Separator();

                static char formula[ 32 ]{};
                ImGui::Text( "Formula" );
                ImGui::InputText( "##2", formula, sizeof( formula ) );
                if (ImGui::Button( "Generate (automatic resonance)" ) && formula[ 0 ])
                {
                    ParsedFormula pf = parseFormulaFull( formula );
                    Resonance::Generator gen( pf.atoms, pf.charge );
                    atoms.build( pf.atoms, gen.bestStructure() );
                    ctx.currentPrebuiltAtom = formula;
                    formula[ 0 ] = '\0';
                }

                ImGui::Separator();
                if (ImGui::Button( "Delete Molecule" ))
                {
                    atoms.clear(); ctx.currentPrebuiltAtom.clear();
                }
                EndCenteredCard( card );
            }


            {
                auto card = BeginCenteredCard( "Physics Engine", CARD_W );
                CustomMenu::CustomCheckbox( "Molecule Stability System", &atoms.betterStabilization );
                CustomMenu::CustomCheckbox( "Follow Camera", &atoms.followCamera );
                EndCenteredCard( card );
            }


            {
                auto card = BeginCenteredCard( "VSEPR/Resonance Options", CARD_W );
                CustomMenu::CustomCheckbox( "Single Central Atom Mode", &Resonance::centralOnlyBonding );
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip( "Enable this for molecules with only one central bonding atom for performance when using the automatic resonance generator." );
                CustomMenu::CustomCheckbox( "Apply VSEPR Forces", &atoms.applyVESPR );
                if (atoms.applyVESPR)
                    CustomMenu::CustomCheckbox( "Large-molecule VSEPR", &atoms.fastCorrection );
                CustomMenu::CustomCheckbox( "Highlight Ring Bonds", &atoms.highlightRings );
                if (atoms.highlightRings)
                    CustomMenu::CustomCheckbox( "Highlight Aromatic Candidates", &atoms.highlightAromaticCandidates );
                EndCenteredCard( card );
            }

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem( "Resonance" ))
        {
            ShowResonanceTab( ctx, CARD_W );
        }


        if (ImGui::BeginTabItem( "Prebuilt" ))
        {
            auto card = BeginCenteredCard( "Choose a Molecule", CARD_W );
            ImGui::Text( "Choose:" );

            if (ImGui::BeginTable( "prebuilt_table", 4, ImGuiTableFlags_SizingStretchProp ))
            {
                auto addButton = [&]( const char *label, auto fn ) {
                    ImGui::TableNextColumn();
                    if (ImGui::Button( label ))
                        fn( ctx );
                    };

                addButton( "H2O", buildH2O );
                addButton( "CO2", buildCO2 );
                addButton( "NH3", buildNH3 );
                addButton( "CH4", buildCH4 );
                addButton( "NO2-", buildNO2m );
                addButton( "HCN", buildHCN );
                addButton( "O3", buildO3 );
                addButton( "CN-", buildCNminus );
                addButton( "O2", buildO2 );
                addButton( "N2", buildN2 );
                addButton( "H2", buildH2 );
                addButton( "CO", buildCO );
                addButton( "HCl", buildHCl );
                addButton( "HF", buildHF );
                addButton( "SO2", buildSO2 );
                addButton( "SO3", buildSO3 );
                addButton( "H2S", buildH2S );
                addButton( "NH4+", buildNH4p );
                addButton( "NO3-", buildNO3m );
                addButton( "SO4^2-", buildSO4m2 );
                addButton( "HCO3-", buildHCO3m );
                addButton( "CH3OH", buildCH3OH );
                addButton( "C2H6", buildC2H6 );
                addButton( "C2H4", buildC2H4 );
                addButton( "C2H2", buildC2H2 );
                addButton( "H2CO", buildH2CO );
                addButton( "HCOOH", buildHCOOH );
                addButton( "C6H6", buildC6H6 );

                ImGui::EndTable();
            }

            ImGui::Separator();
            if (ImGui::Button( "Delete Molecule" ))
            {
                atoms.clear();
                ctx.currentPrebuiltAtom.clear();
            }

            EndCenteredCard( card );
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem( "Instructions" ))
        {
            auto card = BeginCenteredCard( "Shortcuts", CARD_W );
            ImGui::BulletText( "W/A/S/D - move camera (fly mode)" );
            ImGui::BulletText( "TAB - toggle Fly/Edit mode" );
            ImGui::BulletText( "Left-drag - move atoms" );
            ImGui::BulletText( "Click 2 atoms + 1/2/3 - bond order" );
            ImGui::BulletText( "Right-click 2 atoms - break bond" );
            EndCenteredCard( card );

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem( "Advanced" ))
        {
            auto card = BeginCenteredCard( "VSEPR PID", CARD_W );
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();
            CustomMenu::CustomSliderFloat( "Proportion", ImGuiDataType_Float,
                &atoms.correctionProportion, 0.1f, 1.0f, "%.2f",
                IM_COL32( 76, 152, 220, 255 ), ImGuiSliderFlags_None, 220, "k" );

            CustomMenu::CustomSliderFloat( "Step Size (degrees)", ImGuiDataType_Float,
                &atoms.maxCorrectionPerStep, 1.0f, 50.0f, "%.0f",
                IM_COL32( 76, 152, 220, 255 ), ImGuiSliderFlags_None, 220, "\xC2\xB0" );

            ImGui::Separator();
            if (ImGui::Button( "Run Unit Tests" ))
            {
                runMoleculeTests( atoms, *ctx.textRenderer );
            }
            EndCenteredCard( card );

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void ShowStatsOverlay( InputContext &ctx, float dipole, const std::string &forcesCSV ) {
    AtomSystem &atoms = *ctx.atoms;
    AtomSystem::MoleculeAnalysis analysis = atoms.analyzeMolecule();

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_AlwaysAutoResize
        | ImGuiWindowFlags_NoSavedSettings;

    ImGui::SetNextWindowPos( ImVec2( 10, 10 ), ImGuiCond_Always );
    ImGui::Begin( "Simulation Status", nullptr, flags );

    const char *mol = ctx.currentPrebuiltAtom.empty() ? "(none)" : ctx.currentPrebuiltAtom.c_str();
    ImGui::Text( "Molecule: %s", mol );
    ImGui::Separator();

    ImGui::Text( "Polarity: %s", atoms.isPolar ? "Polar" : "Non Polar" );

    std::string forces = forcesCSV;
    while (!forces.empty() && (forces.back() == ' ' || forces.back() == ',')) forces.pop_back();
    if (forces.empty()) forces = "None";
    ImGui::Text( "Forces: %s", forces.c_str() );
    ImGui::Text( "Dipole Magnitude: %.4f", dipole );
    ImGui::Text( "Adjustment Magnitude: %.3f", atoms.latestCorrectionStrength );
    ImGui::Text( "Angle RMS Deviation: %.2f°", analysis.angleRmsDeviation );

    float stability = glm::clamp( 1.f - 0.5f * atoms.latestCorrectionStrength, 0.f, 1.f );
    ImGui::Text( "Simulation Stability: %.0f%%", stability * 100.f );
    ImGui::ProgressBar( stability, ImVec2( 240, 0.0f ) );

    ImGui::Separator();
    ImGui::TextDisabled( "WASD move | TAB for edit mode" );

    ImGui::End();

    ImGui::SetNextWindowPos( ImVec2( 10, 210 ), ImGuiCond_Always );
    ImGui::Begin( "Molecule Analysis", nullptr, flags );
    if (ImGui::BeginTable( "analysis_table", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_RowBg ))
    {
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex( 0 ); ImGui::TextUnformatted( "Atoms (Heavy)" ); ImGui::TableSetColumnIndex( 1 ); ImGui::Text( "%d (%d)", analysis.atomCount, analysis.heavyAtomCount );
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex( 0 ); ImGui::TextUnformatted( "Bonds" ); ImGui::TableSetColumnIndex( 1 ); ImGui::Text( "%d", analysis.bondCount );
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex( 0 ); ImGui::TextUnformatted( "Components" ); ImGui::TableSetColumnIndex( 1 ); ImGui::Text( "%d (heavy %d)", analysis.components, analysis.heavyComponents );
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex( 0 ); ImGui::TextUnformatted( "Rings" ); ImGui::TableSetColumnIndex( 1 ); ImGui::Text( "%d (heavy %d)", analysis.ringCount, analysis.heavyRingCount );
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex( 0 ); ImGui::TextUnformatted( "Aromatic candidates" ); ImGui::TableSetColumnIndex( 1 ); ImGui::Text( "%d", analysis.aromaticRingCandidates );
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex( 0 ); ImGui::TextUnformatted( "Average bond order" ); ImGui::TableSetColumnIndex( 1 ); ImGui::Text( "%.2f", analysis.averageBondOrder );
        ImGui::EndTable();
    }
    ImGui::End();

    ImGui::SetNextWindowPos( ImVec2( 280, 10 ), ImGuiCond_Always ); 
    ImGui::Begin( "Bonds", nullptr, flags );

    if (ImGui::BeginTable( "bonds_table", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_RowBg ))
    {
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex( 0 ); ImGui::TextUnformatted( "Single" ); ImGui::TableSetColumnIndex( 1 ); ImGui::Text( "%d", atoms.singleBonds );
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex( 0 ); ImGui::TextUnformatted( "Double" ); ImGui::TableSetColumnIndex( 1 ); ImGui::Text( "%d", atoms.doubleBonds );
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex( 0 ); ImGui::TextUnformatted( "Triple" ); ImGui::TableSetColumnIndex( 1 ); ImGui::Text( "%d", atoms.tripleBonds );
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex( 0 ); ImGui::TextUnformatted( "Sigma" );  ImGui::TableSetColumnIndex( 1 ); ImGui::Text( "%d", atoms.sigmaBonds );
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex( 0 ); ImGui::TextUnformatted( "Pi" );     ImGui::TableSetColumnIndex( 1 ); ImGui::Text( "%d", atoms.piBonds );
        ImGui::EndTable();
    }

    ImGui::End();
}