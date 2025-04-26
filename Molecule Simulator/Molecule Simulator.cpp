#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "stb_easy_font.h"
#include "Renderer.h"
#include "Camera.h"
#include "AtomSystem.h"
#include "TextRenderer.h"

#include <iostream>

// Window settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// Camera setup
Camera camera( glm::vec3( 0.0f, 0.0f, 8.0f ) );
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Global window pointer
GLFWwindow *window = nullptr;

// Molecule system (pass the text renderer reference to AtomSystem)
TextRenderer textRenderer;
AtomSystem atomSystem( 100, textRenderer );

// Mouse callback
void mouse_callback( GLFWwindow *window, double xpos, double ypos ) {
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement( xoffset, yoffset );
}

// Input processing
void processInput( GLFWwindow *window ) {
    if (glfwGetKey( window, GLFW_KEY_ESCAPE ) == GLFW_PRESS)
        glfwSetWindowShouldClose( window, true );

    if (glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS)
        camera.ProcessKeyboard( Camera_Movement::FORWARD, deltaTime );
    if (glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS)
        camera.ProcessKeyboard( Camera_Movement::BACKWARD, deltaTime );
    if (glfwGetKey( window, GLFW_KEY_A ) == GLFW_PRESS)
        camera.ProcessKeyboard( Camera_Movement::LEFT, deltaTime );
    if (glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS)
        camera.ProcessKeyboard( Camera_Movement::RIGHT, deltaTime );
}

int main() {
    // Initialize GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

    window = glfwCreateWindow( SCR_WIDTH, SCR_HEIGHT, "Molecule Simulator", nullptr, nullptr );
    if (!window)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent( window );

    glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
    glfwSetCursorPosCallback( window, mouse_callback );

    // Initialize GLAD
    if (!gladLoadGLLoader( (GLADloadproc)glfwGetProcAddress ))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    glEnable( GL_PROGRAM_POINT_SIZE );
    glViewport( 0, 0, SCR_WIDTH, SCR_HEIGHT );

    // Initialize the renderer and camera
    Renderer::Init();
    Renderer::SetCamera( &camera );

    // Initialize the TextRenderer here (only once)
    textRenderer.Init();

    // --- Molecule Setup ---
    Atom oxygen( "O", glm::vec3( 0.0f, 0.0f, 0.0f ), 16.0f );
    Atom hydrogen1( "H", glm::vec3( 0.9f, 0.6f, 0.0f ), 1.0f );
    Atom hydrogen2( "H", glm::vec3( -0.9f, 0.6f, 0.0f ), 1.0f );

    atomSystem.addAtom( oxygen );
    atomSystem.addAtom( hydrogen1 );
    atomSystem.addAtom( hydrogen2 );

    atomSystem.createBond( 0, 1 );
    atomSystem.createBond( 0, 2 );

    atomSystem.updateLonePairs(); 


    // --- Main loop ---
    while (!glfwWindowShouldClose( window ))
    {
        // Timing
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input
        processInput( window );

        // Logic
        atomSystem.update( deltaTime );

        // Render
        glClearColor( 0.1f, 0.1f, 0.1f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        // Render 3D atoms and bonds
        atomSystem.render( SCR_WIDTH, SCR_HEIGHT );

        // Render text (at the very end, after everything else)
        // This could be modified or removed if you want to move labels to render dynamically
        //textRenderer.DrawText( "H2O", glm::vec3( 0.0f, 0.0f, 0.0f ), SCR_WIDTH, SCR_HEIGHT );

        glfwSwapBuffers( window );
        glfwPollEvents();
    }

    Renderer::Shutdown();
    glfwTerminate();
    return 0;
}
