#include "UI.h"
#include <imgui.h>
#include <glm/glm.hpp>
#include "Parser.h"
#include "resonance.h"
#include "tinyfiledialogs.h"
#include "Tests.h"

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
        auto &s = *c.atoms; s.build( { "C","N" }, { {0,1,3} } );             c.currentPrebuiltAtom = "CN-";
    }
}

void ShowImGuiMenu( InputContext &ctx ) {
    AtomSystem &atoms = *ctx.atoms;

    ImGui::Begin( "Molecule Menu" );
    if (ImGui::BeginTabBar( "Tabs" ))
    {
        if (ImGui::BeginTabItem( "Main" ))
        {
            static char sym[ 8 ]{};                     // atom symbol
            ImGui::InputText( "Atom Symbol", sym, sizeof( sym ) );
            if (ImGui::Button( "Spawn Atom" ) && sym[ 0 ])
            {
                glm::vec3 dir = ScreenRay( ctx, ctx.winWidth / 2, ctx.winHeight / 2 );
                float t = (0.f - ctx.camera->Position.y) / dir.y;
                atoms.spawnAtom( sym, ctx.camera->Position + dir * t );
                sym[ 0 ] = '\0';
            }

            ImGui::Separator();

            static char formula[ 32 ]{};                // formula box
            ImGui::InputText( "Formula", formula, sizeof( formula ) );
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

            ImGui::Separator();
            ImGui::Checkbox( "Molecule Stability System", &atoms.betterStabilization );
            ImGui::Checkbox( "Follow Camera", &atoms.followCamera );
            ImGui::Checkbox( "Energy-minimization VSEPR (better for more complex molecules)", &atoms.fastCorrection );
            ImGui::Checkbox( "Central Only Bonding", &Resonance::centralOnlyBonding );
			ImGui::Checkbox( "Apply VSEPR", &atoms.applyVESPR );
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Text( "Enable this for molecules that only have one central atom; it helps with performance" );
                ImGui::EndTooltip();
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem( "Prebuilt" ))
        {
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

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem( "Instructions" ))
        {
            ImGui::BulletText( "W/A/S/D – move camera (fly mode)" );
            ImGui::BulletText( "TAB – toggle Fly/Edit mode" );
            ImGui::BulletText( "Left-drag – move atoms" );
            ImGui::BulletText( "Click 2 atoms + 1/2/3 – bond order" );
            ImGui::BulletText( "Right-click 2 atoms – break bond" );
            ImGui::Separator();
            if (ImGui::Button( "Run Unit Tests" ))
            {
                runMoleculeTests( atoms, *ctx.textRenderer );
            }
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
    ImGui::End();
}
