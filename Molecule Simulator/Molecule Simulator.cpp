#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"
#include "Renderer.h"
#include "TextRenderer.h"
#include "AtomSystem.h"

#include <iostream>
#include <chrono>

constexpr int WIN_W = 1280;
constexpr int WIN_H = 720;

GLFWwindow *window = nullptr;
Camera      camera( glm::vec3( 0, 0, 8 ) );
bool        firstMouse = true;
float       lastX = WIN_W * 0.5f, lastY = WIN_H * 0.5f;

float deltaTime = 0.f, lastFrame = 0.f;

TextRenderer textRenderer;
AtomSystem   atoms( 100, textRenderer );

void framebuffer_size_callback( GLFWwindow *, int w, int h ) {
    glViewport( 0, 0, w, h );
}

void mouse_callback( GLFWwindow *, double xpos, double ypos ) {
    if (firstMouse)
    {
        lastX = (float)xpos; lastY = (float)ypos; firstMouse = false;
    }
    float xoff = (float)xpos - lastX;
    float yoff = lastY - (float)ypos;   // reversed
    lastX = (float)xpos;  lastY = (float)ypos;
    camera.ProcessMouseMovement( xoff, yoff );
}

void scroll_callback( GLFWwindow *, double, double yoffset ) {
    camera.ProcessMouseScroll( (float)yoffset );
}

void processInput( GLFWwindow *win ) {
    if (glfwGetKey( win, GLFW_KEY_ESCAPE ) == GLFW_PRESS)
        glfwSetWindowShouldClose( win, true );

    if (glfwGetKey( win, GLFW_KEY_W ) == GLFW_PRESS) camera.ProcessKeyboard( Camera_Movement::FORWARD, deltaTime );
    if (glfwGetKey( win, GLFW_KEY_S ) == GLFW_PRESS) camera.ProcessKeyboard( Camera_Movement::BACKWARD, deltaTime );
    if (glfwGetKey( win, GLFW_KEY_A ) == GLFW_PRESS) camera.ProcessKeyboard( Camera_Movement::LEFT, deltaTime );
    if (glfwGetKey( win, GLFW_KEY_D ) == GLFW_PRESS) camera.ProcessKeyboard( Camera_Movement::RIGHT, deltaTime );
}

void buildWater( AtomSystem &sys ) {
    sys.spawnAtom( "O" );               // index 0
    sys.spawnAtom( "H" );               // index 1
    sys.spawnAtom( "H" );               // index 2

    sys.createBond( 0, 1, BondType::SINGLE );
    sys.createBond( 0, 2, BondType::SINGLE );
}

void buildCO2( AtomSystem &sys ) {
    sys.spawnAtom( "C" );               // 0
    sys.spawnAtom( "O" );               // 1
    sys.spawnAtom( "O" );               // 2

    sys.createBond( 0, 1, BondType::DOUBLE );
    sys.createBond( 0, 2, BondType::DOUBLE );
}

void buildHCN( AtomSystem &sys ) {
	sys.spawnAtom( "H" );               // 0
	sys.spawnAtom( "C" );               // 1
	sys.spawnAtom( "N" );               // 2
	sys.createBond( 0, 1, BondType::SINGLE );
	sys.createBond( 1, 2, BondType::TRIPLE );
}


int main() {
    glfwInit();
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    window = glfwCreateWindow( WIN_W, WIN_H, "Molecule Sim", nullptr, nullptr );
    if (!window)
    {
        std::cerr << "GLFW window failed\n"; return 1;
    }
    glfwMakeContextCurrent( window );
    glfwSetFramebufferSizeCallback( window, framebuffer_size_callback );
    glfwSetCursorPosCallback( window, mouse_callback );
    glfwSetScrollCallback( window, scroll_callback );
    glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );

    if (!gladLoadGLLoader( (GLADloadproc)glfwGetProcAddress ))
    {
        std::cerr << "GLAD init failed\n"; return 1;
    }
    glEnable( GL_DEPTH_TEST );

    Renderer::Init( &camera );
    textRenderer.Init();

    //buildWater( atoms );
    //buildCO2( atoms );
	buildHCN( atoms );

    atoms.updateLonePairs();


    while (!glfwWindowShouldClose( window ))
    {
        float now = (float)glfwGetTime();
        deltaTime = now - lastFrame;
        lastFrame = now;

        processInput( window );
        atoms.update( deltaTime );

        glClearColor( 0.05f, 0.05f, 0.05f, 1.f );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        int w, h;  glfwGetFramebufferSize( window, &w, &h );
        atoms.render( w, h );

        float mag = atoms.computeDipole();

        textRenderer.DrawScreenText( "Molecule(s): HCN", 10, 30, w, h );
        textRenderer.DrawScreenText( std::string( "Polarity: " ) + (atoms.isPolar ? "Polar" : "Non-Polar") + "; " + "Magnitude: " + std::to_string(mag), 10, 50, w, h);
        textRenderer.DrawScreenText( "Correction Coef: " + std::to_string(atoms.latestCorrectionStrength), 10, 70, w, h );

        glfwSwapBuffers( window );
        glfwPollEvents();
    }

    Renderer::Shutdown();
    glfwTerminate();
}
