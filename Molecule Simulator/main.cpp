#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iomanip>

#include "Parser.h"
#include "UI.h"
#include "InputHandler.h"

#include "Camera.h"
#include "AtomSystem.h"
#include "Renderer.h"
#include "TextRenderer.h"
#include "Raytracing.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>

void setStyle() {
    ImGuiStyle &style = ImGui::GetStyle();

    

    style.FrameBorderSize = 1.0f;
    style.WindowBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.TabBorderSize = 0.0f;
    style.WindowRounding = 8.0f;
    style.ChildRounding = 8.0f;
    style.FrameRounding = 8.0f;
    style.PopupRounding = 8.0f;
    style.ScrollbarRounding = 10.0f;
    style.GrabRounding = 10.0f;
    style.TabRounding = 8.0f;

    ImVec4 bgWindow = ImVec4( 28 / 255.f, 28 / 255.f, 34 / 255.f, 0.90f );
    ImVec4 bgChild = ImVec4( 38 / 255.f, 38 / 255.f, 48 / 255.f, 0.85f );
    ImVec4 bgFrame = ImVec4( 48 / 255.f, 56 / 255.f, 72 / 255.f, 0.85f );
    ImVec4 bgFrameHov = ImVec4( 68 / 255.f, 80 / 255.f, 104 / 255.f, 0.90f );
    ImVec4 bgFrameAct = ImVec4( 58 / 255.f, 68 / 255.f, 92 / 255.f, 0.95f );
    ImVec4 accent = ImVec4( 0 / 255.f, 122 / 255.f, 204 / 255.f, 0.90f );
    ImVec4 accentHov = ImVec4( 0 / 255.f, 142 / 255.f, 224 / 255.f, 0.95f );
    ImVec4 accentAct = ImVec4( 0 / 255.f, 102 / 255.f, 184 / 255.f, 1.00f );
    ImVec4 textColor = ImVec4( 1, 1, 1, 1 );
    ImVec4 borderCol = ImVec4( 64 / 255.f, 72 / 255.f, 88 / 255.f, 0.85f );

    ImVec4 textDim = ImVec4( 0.78f, 0.80f, 0.86f, 1.0f );

    auto &c = style.Colors;
    c[ ImGuiCol_Text ] = textColor;
    c[ ImGuiCol_WindowBg ] = bgWindow;
    c[ ImGuiCol_ChildBg ] = bgChild;
    c[ ImGuiCol_PopupBg ] = bgChild;
    c[ ImGuiCol_Border ] = borderCol;
    c[ ImGuiCol_BorderShadow ] = ImVec4( 0, 0, 0, 0.30f );

    c[ ImGuiCol_FrameBg ] = bgFrame;
    c[ ImGuiCol_FrameBgHovered ] = bgFrameHov;
    c[ ImGuiCol_FrameBgActive ] = bgFrameAct;

    c[ ImGuiCol_TitleBg ] = bgFrame;
    c[ ImGuiCol_TitleBgActive ] = bgChild;
    c[ ImGuiCol_MenuBarBg ] = bgChild;

    c[ ImGuiCol_SliderGrab ] = accent;
    c[ ImGuiCol_SliderGrabActive ] = accentAct;
    c[ ImGuiCol_CheckMark ] = accent;

    c[ ImGuiCol_Button ] = bgFrame;
    c[ ImGuiCol_ButtonHovered ] = accentHov;
    c[ ImGuiCol_ButtonActive ] = accentAct;

    c[ ImGuiCol_Header ] = accent;
    c[ ImGuiCol_HeaderHovered ] = accentHov;
    c[ ImGuiCol_HeaderActive ] = accentAct;

    c[ ImGuiCol_Tab ] = ImVec4( accent.x, accent.y, accent.z, 0.45f );
    c[ ImGuiCol_TabHovered ] = accentHov;
    c[ ImGuiCol_TabActive ] = accentAct;
    c[ ImGuiCol_TabUnfocused ] = ImVec4( accent.x, accent.y, accent.z, 0.25f );
    c[ ImGuiCol_TabUnfocusedActive ] = ImVec4( accent.x, accent.y, accent.z, 0.60f );

    c[ ImGuiCol_TextSelectedBg ] = ImVec4( accent.x, accent.y, accent.z, 0.35f );
    c[ ImGuiCol_NavHighlight ] = ImVec4( accent.x, accent.y, accent.z, 0.35f );

    c[ ImGuiCol_Separator ] = borderCol;
    c[ ImGuiCol_SeparatorHovered ] = accentHov;
    c[ ImGuiCol_SeparatorActive ] = accentAct;
}


static Raytracer ray;          
static bool useRT = false; // Dont use this 

int main() {
    if (!glfwInit())
    {
        std::cerr << "GLFW init failed"; return 1;
    }
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 6 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

    GLFWwindow *window = glfwCreateWindow( WIN_W, WIN_H, "Molecule Viewer", nullptr, nullptr );
    if (!window)
    {
        std::cerr << "Window creation failed";
        return 1;
    }
    glfwMakeContextCurrent( window );
    glfwSwapInterval( 1 ); 

    if (!gladLoadGLLoader( (GLADloadproc)glfwGetProcAddress ))
    {
        std::cerr << "GLAD init failed";

        return 1;
    }
    glEnable( GL_DEPTH_TEST );

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO(); (void)io;
    ImGui_ImplGlfw_InitForOpenGL( window, true );
    ImGui_ImplOpenGL3_Init( "#version 460" );

    Camera camera( glm::vec3( 0, 0, 8 ) );
    TextRenderer textRenderer; textRenderer.Init();
    AtomSystem atoms( 20, textRenderer );
    Renderer::Init( &camera );
    ray.init();

    InputContext ctx;
    ctx.window = window;
    ctx.camera = &camera;
    ctx.atoms = &atoms;
    ctx.textRenderer = &textRenderer;
    InitInputHandler( ctx );

   
    float lastFrame = 0.f;
    while (!glfwWindowShouldClose( window ))
    {
        float now = (float)glfwGetTime();
        float deltaTime = now - lastFrame; lastFrame = now;

        ProcessInput( ctx, deltaTime );
        atoms.update( deltaTime );

        if (atoms.getAtoms().size() > 1 && atoms.followCamera)
        {
            glm::vec3 center = atoms.getCenter();
            camera.FollowTargetEuler( center, deltaTime, 1.0f, 12.0f );
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        setStyle();
        ShowImGuiMenu( ctx );

        glClearColor( 0.05f, 0.05f, 0.05f, 1.f );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        int fbW, fbH; glfwGetFramebufferSize( window, &fbW, &fbH );

        atoms.updateLonePairs();
        atoms.updateFormalCharges();

        glDisable( GL_DEPTH_TEST );
        glEnable( GL_BLEND ); glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        Renderer::DrawGrid( fbW, fbH );
        glDisable( GL_BLEND ); glEnable( GL_DEPTH_TEST );

        if (useRT)
        {
            std::vector<GPUSphere> gpu; gpu.reserve( atoms.getAtoms().size() );
            for (const Atom &a : atoms.getAtoms()) gpu.push_back( { a.position, a.radius } );
            ray.setScene( gpu );
            ray.render( camera, fbW, fbH );
        }
        else
        {
            atoms.render( fbW, fbH,
                ctx.selectedAtom,
                ctx.hoveredAtom,
                ctx.bondFirst,
                ctx.breakFirst,
                camera );
        }

        float dipole = atoms.computeDipole();
        textRenderer.DrawScreenText( ctx.currentPrebuiltAtom +
            " -- Polarity: " + std::string( atoms.isPolar ? "Polar" : "Non Polar" ), 10, 30, fbW, fbH );
        textRenderer.DrawScreenText( "Dipole Magnitude: " + std::to_string( dipole ), 10, 50, fbW, fbH );
        textRenderer.DrawScreenText( "Adjustment Magnitude: " + std::to_string( atoms.latestCorrectionStrength ), 10, 70, fbW, fbH );
        textRenderer.DrawScreenText( "Single Bonds: " + std::to_string( atoms.singleBonds ), 10, 110, fbW, fbH );
        textRenderer.DrawScreenText( "Double Bonds: " + std::to_string( atoms.doubleBonds ), 10, 130, fbW, fbH );
        textRenderer.DrawScreenText( "Triple Bonds: " + std::to_string( atoms.tripleBonds ), 10, 150, fbW, fbH );
        textRenderer.DrawScreenText( "Sigma Bonds:  " + std::to_string( atoms.sigmaBonds ), 10, 170, fbW, fbH );
        textRenderer.DrawScreenText( "PI Bonds:     " + std::to_string( atoms.piBonds ), 10, 190, fbW, fbH );
        float stability = glm::clamp( 1.f - 0.5f * atoms.latestCorrectionStrength, 0.f, 1.f );
        textRenderer.DrawScreenText( "Simulation Stability: " + std::to_string( stability * 100.f ) + "%", 10, 210, fbW, fbH );
        textRenderer.DrawScreenText( "WASD move - TAB edit mode", 10, 230, fbW, fbH );

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

        glfwSwapBuffers( window );
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    Renderer::Shutdown();
    glfwTerminate();
    return 0;
}
