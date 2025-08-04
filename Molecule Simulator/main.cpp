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


static Raytracer ray;          
static bool      useRT = false;

//--------------------------------------------------
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
    glfwSwapInterval( 1 ); // v?sync

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

    Camera        camera( glm::vec3( 0, 0, 8 ) );
    TextRenderer  textRenderer; textRenderer.Init();
    AtomSystem    atoms( 100, textRenderer );
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
        textRenderer.DrawScreenText( "Pi Bonds:     " + std::to_string( atoms.piBonds ), 10, 190, fbW, fbH );
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
