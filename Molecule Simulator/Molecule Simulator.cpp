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
GLFWwindow* window = nullptr;

// camera
Camera camera(glm::vec3(0, 0, 8));
bool firstMouse = true;
float lastX = WIN_W / 2, lastY = WIN_H / 2;
float deltaTime = 0, lastFrame = 0;

std::string currentPrebuiltAtom = "NA";

// systems
TextRenderer textRenderer;
AtomSystem atoms(100, textRenderer);

// More user interactivity 
Atom *selectedAtom = nullptr;
bool dragging = false;

enum class ControlMode
{
    FLY_CAMERA, PICK_DRAG
};
ControlMode currentMode = ControlMode::FLY_CAMERA;


Atom *PickAtom( int mouseX, int mouseY, int windowWidth, int windowHeight, const std::vector<Atom> &atoms ) {
    // Convert mouse screen position to normalized device coordinates
    float ndcX = (2.0f * mouseX) / windowWidth - 1.0f;
    float ndcY = 1.0f - (2.0f * mouseY) / windowHeight;
    glm::vec4 rayClip = glm::vec4( ndcX, ndcY, -1.0f, 1.0f );

    glm::mat4 proj = glm::perspective( glm::radians( camera.Zoom ), (float)windowWidth / (float)windowHeight, 0.1f, 100.0f );
    glm::mat4 invProj = glm::inverse( proj );
    glm::vec4 rayEye = invProj * rayClip;
    rayEye = glm::vec4( rayEye.x, rayEye.y, -1.0f, 0.0f );

    glm::mat4 invView = glm::inverse( camera.GetViewMatrix() );
    glm::vec3 rayWorld = glm::normalize( glm::vec3( invView * rayEye ) );

    glm::vec3 rayOrigin = camera.Position;

    Atom *closest = nullptr;
    float minDist = 99999.0f;

    for (const auto &atom : atoms)
    {
        glm::vec3 toAtom = atom.position - rayOrigin;
        float t = glm::dot( toAtom, rayWorld );
        glm::vec3 closestPoint = rayOrigin + rayWorld * t;
        float distToCenter = glm::length( closestPoint - atom.position );

        if (distToCenter < atom.radius && t > 0.0f)
        {
            if (t < minDist)
            {
                minDist = t;
                closest = const_cast<Atom *>( &atom );
            }
        }
    }

    return closest;
}


// callbacks
void framebuffer_size_callback(GLFWwindow*, int w, int h) {
    glViewport(0, 0, w, h);
}


void mouse_callback( GLFWwindow *window, double xpos, double ypos ) {
    if (firstMouse)
    {
        lastX = xpos; lastY = ypos; firstMouse = false;
    }

    float xoff = xpos - lastX;
    float yoff = lastY - ypos;
    lastX = xpos; lastY = ypos;

    if (currentMode == ControlMode::FLY_CAMERA)
    {
        camera.ProcessMouseMovement( xoff, yoff );
    }
    else if (dragging && selectedAtom)
    {
        // dragging behavior (already correct)
        float depth = glm::length( selectedAtom->position - camera.Position );
        glm::mat4 proj = glm::perspective( glm::radians( camera.Zoom ), (float)WIN_W / WIN_H, 0.1f, 100.0f );
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 VP = proj * view;
        glm::mat4 invVP = glm::inverse( VP );

        float ndcX = (2.0f * xpos) / WIN_W - 1.0f;
        float ndcY = 1.0f - (2.0f * ypos) / WIN_H;
        glm::vec4 ndcPos = glm::vec4( ndcX, ndcY, 1.0f, 1.0f );
        glm::vec4 worldPos = invVP * ndcPos;
        worldPos /= worldPos.w;
        glm::vec3 newDir = glm::normalize( glm::vec3( worldPos ) - camera.Position );

        selectedAtom->position = camera.Position + newDir * depth;
        selectedAtom->velocity = glm::vec3( 0.0f );
    }
}


void mouse_button_callback( GLFWwindow *window, int button, int action, int mods ) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double xpos, ypos;
        glfwGetCursorPos( window, &xpos, &ypos );
        selectedAtom = PickAtom( (int)xpos, (int)ypos, WIN_W, WIN_H, atoms.getAtoms() ); // Need a `getAtoms()` accessor
        dragging = (selectedAtom != nullptr);
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        dragging = false;
        selectedAtom = nullptr;
    }
}




void scroll_callback(GLFWwindow*, double, double yoff) {
    camera.ProcessMouseScroll((float)yoff);
}

void processInput( GLFWwindow *w ) {
    static bool tabPressedLastFrame = false;

    if (glfwGetKey( w, GLFW_KEY_ESCAPE ) == GLFW_PRESS) glfwSetWindowShouldClose( w, true );

    if (currentMode == ControlMode::FLY_CAMERA)
    {
        if (glfwGetKey( w, GLFW_KEY_W ) == GLFW_PRESS) camera.ProcessKeyboard( Camera_Movement::FORWARD, deltaTime );
        if (glfwGetKey( w, GLFW_KEY_S ) == GLFW_PRESS) camera.ProcessKeyboard( Camera_Movement::BACKWARD, deltaTime );
        if (glfwGetKey( w, GLFW_KEY_A ) == GLFW_PRESS) camera.ProcessKeyboard( Camera_Movement::LEFT, deltaTime );
        if (glfwGetKey( w, GLFW_KEY_D ) == GLFW_PRESS) camera.ProcessKeyboard( Camera_Movement::RIGHT, deltaTime );
    }

    if (glfwGetKey( w, GLFW_KEY_TAB ) == GLFW_PRESS)
    {
        if (!tabPressedLastFrame)
        { // Only toggle once per actual press
            if (currentMode == ControlMode::FLY_CAMERA)
            {
                currentMode = ControlMode::PICK_DRAG;
                glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
            }
            else
            {
                currentMode = ControlMode::FLY_CAMERA;
                glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
            }
        }
        tabPressedLastFrame = true;
    }
    else
    {
        tabPressedLastFrame = false;
    }
}



void buildHCN(AtomSystem& sys) {
    sys.spawnAtom("H"); //0
    sys.spawnAtom("C"); //1
    sys.spawnAtom("N"); //2
    sys.createBond(0, 1, BondType::SINGLE);
    sys.createBond(1, 2, BondType::TRIPLE);
    currentPrebuiltAtom = "HCN";
}

void buildNO2minus(AtomSystem& sys) {
    /* sys.spawnAtom("O");
     sys.spawnAtom( "N" );
     sys.spawnAtom( "O" );

     sys.createBond( 1, 0, BondType::DOUBLE );
     sys.createBond( 1, 2, BondType::SINGLE );   // resonance form
     */

    sys.build({ "O", "N", "O" }, { {1, 0, 2}, {1, 2 ,1} });
    // Template
    // Atoms -> then bonds where the bonds are a tuple of (atomA, atomB, order)
    currentPrebuiltAtom = "NO2-";
}

void buildCO2(AtomSystem& sys) {
    // Carbon Dioxide
    sys.build({ "O", "C", "O" }, { {1, 0, 2}, {1, 2, 2} });
    currentPrebuiltAtom = "CO2";
}

void buildH2O(AtomSystem& sys) {
    // Water
    sys.build({ "H", "O", "H" }, { {1, 0, 1}, {1, 2, 1} });
    currentPrebuiltAtom = "H2O";
}

void buildNH3(AtomSystem& sys) {
    // Ammonia
    sys.build({ "N", "H", "H", "H" }, { {0, 1, 1}, {0, 2, 1}, {0, 3, 1} });
    currentPrebuiltAtom = "NH3";
}

void buildCH4(AtomSystem& sys) {
    // Methane
    sys.build({ "C", "H", "H", "H", "H" }, { {0, 1, 1}, {0, 2, 1}, {0, 3, 1}, {0, 4, 1} });
    currentPrebuiltAtom = "CH4";
}

void buildO3(AtomSystem& sys) {
    // Ozone
    sys.build({ "O", "O", "O" }, { {0, 1, 2}, {1, 2, 1} });
    currentPrebuiltAtom = "O3";
}

void buildCNminus(AtomSystem& sys) {
    // Cyanide 
    sys.build({ "C", "N" }, { {0, 1, 3} });
    currentPrebuiltAtom = "CN-";
}



int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(WIN_W, WIN_H, "Molecule Viewer", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "GLFW failed\n"; return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback( window, mouse_button_callback );
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "GLAD failed\n"; return 1;
    }
    glEnable(GL_DEPTH_TEST);

    Renderer::Init(&camera);
    textRenderer.Init();

    // Change this to see different atoms, or make your own using the molecule builder
    buildO3(atoms);


    while (!glfwWindowShouldClose(window))
    {
        float now = (float)glfwGetTime();
        deltaTime = now - lastFrame;
        lastFrame = now;

        processInput(window);
        atoms.update(deltaTime);

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int w, h; glfwGetFramebufferSize(window, &w, &h);

        Renderer::DrawGrid(w, h);

        atoms.updateLonePairs();
        atoms.updateFormalCharges();

        atoms.render(w, h);

        float totalPolarityMagnitude = atoms.computeDipole();
        textRenderer.DrawScreenText(currentPrebuiltAtom + " -- Polarity (partial charge and bond): " + std::string(atoms.isPolar ? "Polar" : "Non-Polar"),
            10, 30, w, h);
        textRenderer.DrawScreenText("Dipole Magnitude: " + std::to_string(totalPolarityMagnitude),
            10, 50, w, h);
        textRenderer.DrawScreenText("Adjustment Magnitude: " + std::to_string(atoms.latestCorrectionStrength),
            10, 70, w, h);
        textRenderer.DrawScreenText("Single Bonds: " + std::to_string(atoms.singleBonds),
            10, 110, w, h);
        textRenderer.DrawScreenText("Double Bonds: " + std::to_string(atoms.doubleBonds),
            10, 130, w, h);
        textRenderer.DrawScreenText("Triple Bonds: " + std::to_string(atoms.tripleBonds),
            10, 150, w, h);
        textRenderer.DrawScreenText("Sigma Bonds: " + std::to_string(atoms.sigmaBonds),
            10, 170, w, h);
        textRenderer.DrawScreenText("PI Bonds: " + std::to_string(atoms.piBonds),
            10, 190, w, h);
        textRenderer.DrawScreenText("Simulation Stability: " + std::to_string(1 - (0.5 * atoms.latestCorrectionStrength)),
            10, 210, w, h);
        textRenderer.DrawScreenText("W-A-S-D to move, TAB to toggle",
            10, 230, w, h);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    Renderer::Shutdown();
    glfwTerminate();
    return 0;
}
