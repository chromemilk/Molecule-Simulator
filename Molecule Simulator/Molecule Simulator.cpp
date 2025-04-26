#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Renderer.h"
#include "Camera.h"
#include "ParticleSystem.h"

#include <iostream>

// Window size
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Camera
Camera camera( glm::vec3( 0.0f, 0.0f, 5.0f ) );
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Particle System
ParticleSystem particleSystem( 1000 );

// Mouse callback
void mouse_callback( GLFWwindow *window, double xpos, double ypos ) {
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed

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

    GLFWwindow *window = glfwCreateWindow( SCR_WIDTH, SCR_HEIGHT, "Particle Simulator", nullptr, nullptr );
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

    glEnable( GL_PROGRAM_POINT_SIZE ); // Allow setting point size in shaders
    glViewport( 0, 0, SCR_WIDTH, SCR_HEIGHT );

    Renderer::Init();
	Renderer::SetCamera( &camera ); // Set the camera for the renderer

    // Main render loop
    while (!glfwWindowShouldClose( window ))
    {
        // Time logic
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input
        processInput( window );

        // Logic
        particleSystem.spawnParticle(); // You can spawn multiple per frame
        particleSystem.update( deltaTime );

        // Render
        glClearColor( 0.1f, 0.1f, 0.1f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT );

        particleSystem.render();

        glfwSwapBuffers( window );
        glfwPollEvents();
    }

    Renderer::Shutdown();
    glfwTerminate();
    return 0;
}
