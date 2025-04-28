#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"
#include "Renderer.h"
#include "TextRenderer.h"
#include "AtomSystem.h"

#include <iostream>

// window size
constexpr int WIN_W = 1280, WIN_H = 720;
GLFWwindow *window = nullptr;

// camera
Camera camera( glm::vec3( 0, 0, 8 ) );
bool firstMouse = true;
float lastX = WIN_W / 2, lastY = WIN_H / 2;
float deltaTime = 0, lastFrame = 0;

// systems
TextRenderer textRenderer;
AtomSystem atoms( 100, textRenderer );

// callbacks
void framebuffer_size_callback( GLFWwindow *, int w, int h ) {
    glViewport( 0, 0, w, h );
}
void mouse_callback( GLFWwindow *, double xpos, double ypos ) {
    if (firstMouse)
    {
        lastX = xpos; lastY = ypos; firstMouse = false;
    }
    float xoff = xpos - lastX;
    float yoff = lastY - ypos;
    lastX = xpos; lastY = ypos;
    camera.ProcessMouseMovement( xoff, yoff );
}
void scroll_callback( GLFWwindow *, double, double yoff ) {
    camera.ProcessMouseScroll( (float)yoff );
}
void processInput( GLFWwindow *w ) {
    if (glfwGetKey( w, GLFW_KEY_ESCAPE ) == GLFW_PRESS) glfwSetWindowShouldClose( w, true );
    if (glfwGetKey( w, GLFW_KEY_W ) == GLFW_PRESS) camera.ProcessKeyboard( Camera_Movement::FORWARD, deltaTime );
    if (glfwGetKey( w, GLFW_KEY_S ) == GLFW_PRESS) camera.ProcessKeyboard( Camera_Movement::BACKWARD, deltaTime );
    if (glfwGetKey( w, GLFW_KEY_A ) == GLFW_PRESS) camera.ProcessKeyboard( Camera_Movement::LEFT, deltaTime );
    if (glfwGetKey( w, GLFW_KEY_D ) == GLFW_PRESS) camera.ProcessKeyboard( Camera_Movement::RIGHT, deltaTime );
}

void buildHCN( AtomSystem &sys ) {
    sys.spawnAtom( "H" ); //0
    sys.spawnAtom( "C" ); //1
    sys.spawnAtom( "N" ); //2
    sys.createBond( 0, 1, BondType::SINGLE );
    sys.createBond( 1, 2, BondType::TRIPLE );
}

void buildNO2minus( AtomSystem &sys ) {
     sys.spawnAtom( "O" );
     sys.spawnAtom( "N" );
     sys.spawnAtom( "O" );

    sys.createBond( 1, 0, BondType::DOUBLE );
    sys.createBond( 1, 2, BondType::SINGLE );   // resonance form
}


int main() {
    glfwInit();
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    window = glfwCreateWindow( WIN_W, WIN_H, "3D Compound Lewis Structure Viewer", nullptr, nullptr );
    if (!window)
    {
        std::cerr << "GLFW failed\n"; return 1;
    }
    glfwMakeContextCurrent( window );
    glfwSetFramebufferSizeCallback( window, framebuffer_size_callback );
    glfwSetCursorPosCallback( window, mouse_callback );
    glfwSetScrollCallback( window, scroll_callback );
    glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
    if (!gladLoadGLLoader( (GLADloadproc)glfwGetProcAddress ))
    {
        std::cerr << "GLAD failed\n"; return 1;
    }
    glEnable( GL_DEPTH_TEST );

    Renderer::Init( &camera );
    textRenderer.Init();

    // CAN CHANGE
    //buildHCN( atoms );
	buildNO2minus( atoms );


    while (!glfwWindowShouldClose( window ))
    {
        float now = (float)glfwGetTime();
        deltaTime = now - lastFrame;
        lastFrame = now;

        processInput( window );
        atoms.update( deltaTime );

        glClearColor( 0.05f, 0.05f, 0.05f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        int w, h; glfwGetFramebufferSize( window, &w, &h );

        Renderer::DrawGrid( w, h );

        atoms.updateLonePairs();
        atoms.updateFormalCharges();

        atoms.render( w, h );

        float mag = atoms.computeDipole();
        textRenderer.DrawScreenText( "NO2- -- Polarity: " + std::string( atoms.isPolar ? "Polar" : "Non-Polar" ),
            10, 30, w, h );
        textRenderer.DrawScreenText( "Dipole Magnitude: " + std::to_string( mag ),
            10, 50, w, h );
        textRenderer.DrawScreenText( "Adjustment Magnitude: " + std::to_string( atoms.latestCorrectionStrength ),
            10, 70, w, h );
        textRenderer.DrawScreenText( "W-A-S-D to move",
            10, 110, w, h );

        glfwSwapBuffers( window );
        glfwPollEvents();
    }

    Renderer::Shutdown();
    glfwTerminate();
    return 0;
}
