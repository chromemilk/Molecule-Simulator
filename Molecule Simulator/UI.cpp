#include "UI.h"
#include <glm/glm.hpp>
#include "Parser.h"
#include "resonance.h"
#include "tinyfiledialogs.h"
#include "Tests.h"
#include "ImGuiHelpers.h"
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
                CustomMenu::CustomCheckbox( "Central Only Bonding", &Resonance::centralOnlyBonding );
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip( "Enable this for molecules with one central atom for performance when using the automatic resonance generator." );
                CustomMenu::CustomCheckbox( "Apply VSEPR Forces", &atoms.applyVESPR );
                if (atoms.applyVESPR)
                    CustomMenu::CustomCheckbox( "Large-molecule VSEPR", &atoms.fastCorrection );
                EndCenteredCard( card );
            }

            ImGui::EndTabItem();
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

    float stability = glm::clamp( 1.f - 0.5f * atoms.latestCorrectionStrength, 0.f, 1.f );
    ImGui::Text( "Simulation Stability: %.0f%%", stability * 100.f );
    ImGui::ProgressBar( stability, ImVec2( 240, 0.0f ) );

    ImGui::Separator();
    ImGui::TextDisabled( "WASD move | TAB for edit mode" );

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