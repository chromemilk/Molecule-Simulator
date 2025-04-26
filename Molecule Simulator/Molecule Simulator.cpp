#include "Renderer.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

int main() {
    // Init GLFW
    if (!glfwInit()) return -1;

    GLFWwindow *window = glfwCreateWindow( 800, 600, "Particle Simulator", nullptr, nullptr );
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent( window );

    // Init GLAD
    if (!gladLoadGLLoader( (GLADloadproc)glfwGetProcAddress ))
    {
        return -1;
    }

    // Init Renderer
    Renderer::Init();

    while (!glfwWindowShouldClose( window ))
    {
        glClearColor( 0.1f, 0.1f, 0.1f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT );

        // Draw a particle
        Renderer::DrawParticle( glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 1.0f, 0.5f, 0.2f ), 10.0f );

        glfwSwapBuffers( window );
        glfwPollEvents();
    }

    Renderer::Shutdown();
    glfwTerminate();
    return 0;
}
