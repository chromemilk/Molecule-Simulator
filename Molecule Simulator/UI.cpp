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
            if (ImGui::Button( "H2O" ))  buildH2O( ctx );  ImGui::SameLine();
            if (ImGui::Button( "CO2" ))  buildCO2( ctx );

            if (ImGui::Button( "NH3" ))  buildNH3( ctx );  ImGui::SameLine();
            if (ImGui::Button( "CH4" ))  buildCH4( ctx );

            if (ImGui::Button( "NO2-" )) buildNO2m( ctx ); ImGui::SameLine();
            if (ImGui::Button( "HCN" ))  buildHCN( ctx );

            if (ImGui::Button( "O3" ))   buildO3( ctx );   ImGui::SameLine();
            if (ImGui::Button( "CN-" ))  buildCNminus( ctx );

            ImGui::Separator();
            if (ImGui::Button( "Delete Molecule" ))
            {
                atoms.clear(); ctx.currentPrebuiltAtom.clear();
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