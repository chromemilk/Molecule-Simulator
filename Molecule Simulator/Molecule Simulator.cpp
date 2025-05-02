#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"
#include "Renderer.h"
#include "TextRenderer.h"
#include "AtomSystem.h"
#include "tinyfiledialogs.h" 

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
Atom *hoveredAtom = nullptr;

Atom *bondFirst = nullptr;   // remember first click
int   nextOrder = 1;         // 1=single default
Atom *breakFirst = nullptr;   



float  grabPlaneY = 0.0f;        // y-level at which we grabbed the atom
glm::vec3 grabRayDir;              // ray direction when grab started

bool        wantSpawnPopup = false;          
static char symbolBuf[ 8 ] = "";          

std::string buildSymbol;              // already existed
bool        deleteMode = false;     // keep – toggled with a key now


glm::vec3 screenRay( int mx, int my ) {
    float ndcX = 2.f * mx / WIN_W - 1.f;
    float ndcY = -2.f * my / WIN_H + 1.f;
    glm::vec4 clip( ndcX, ndcY, -1, 1 );

    glm::mat4 proj = glm::perspective( glm::radians( camera.Zoom ),
        (float)WIN_W / WIN_H, 0.1f, 100.f );
    glm::vec4 eye = glm::inverse( proj ) * clip;
    eye = { eye.x, eye.y, -1, 0 };

    glm::vec3 dir = glm::normalize( glm::vec3( glm::inverse( camera.GetViewMatrix() ) * eye ) );
    return dir;
}

int atomIndex( const Atom *ptr ) {
    const auto &vec = atoms.getAtoms();
    for (int i = 0; i < (int)vec.size(); ++i)
        if (&vec[ i ] == ptr) return i;
    return -1;
}



enum class ControlMode
{
    FLY_CAMERA, PICK_DRAG
};
ControlMode currentMode = ControlMode::FLY_CAMERA;

Atom *PickAtom( int mx, int my, int w, int h,
    const std::vector<Atom> &atoms,
    const std::vector<Atom> &lonePairs ) {
    glm::vec3 O = camera.Position;
    glm::vec3 D = screenRay( mx, my );

    auto tryList = [&]( const std::vector<Atom> &list )->Atom *
        {
            float minT = 1e9; Atom *best = nullptr;
            for (const Atom &a : list)
            {
                glm::vec3 AO = a.position - O;
                float t = glm::dot( AO, D );
                if (t <= 0) continue;
                glm::vec3 P = O + t * D;
                if (glm::length( P - a.position ) < a.radius && t < minT)
                {
                    minT = t; best = const_cast<Atom *>( &a );
                }
            }
            return best;
        };

    if (Atom *hit = tryList( atoms ))      return hit;
    if (Atom *hit = tryList( lonePairs ))  return hit;
    return nullptr;
}



// callbacks
void framebuffer_size_callback(GLFWwindow*, int w, int h) {
    glViewport(0, 0, w, h);
}


void mouse_callback( GLFWwindow *, double xpos, double ypos ) {
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

    if (currentMode == ControlMode::FLY_CAMERA)
    {
        camera.ProcessMouseMovement( xoffset, yoffset );
    }
    else if (dragging && selectedAtom)
    {
        float depth = glm::length( selectedAtom->position - camera.Position );

        glm::mat4 proj = glm::perspective( glm::radians( camera.Zoom ),
            float( WIN_W ) / WIN_H, 0.1f, 100.f );
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 invVP = glm::inverse( proj * view );

        float ndcX = 2.f * xpos / WIN_W - 1.f;
        float ndcY = 1.f - 2.f * ypos / WIN_H;
        glm::vec4 ndc( ndcX, ndcY, 1.f, 1.f );
        glm::vec4 world = invVP * ndc;
        world /= world.w;

        glm::vec3 newDir = glm::normalize( glm::vec3( world ) - camera.Position );

        selectedAtom->position = camera.Position + newDir * depth;
        selectedAtom->velocity = glm::vec3( 0.f );   // Zero momentum when dragging

        constexpr float FLOOR_Y = -0.2f;
        if (selectedAtom->position.y - selectedAtom->radius < FLOOR_Y)
        {
            selectedAtom->position.y = FLOOR_Y + selectedAtom->radius;
        }
    }

    if (currentMode == ControlMode::PICK_DRAG && !dragging)
    {
        double mx, my;
        glfwGetCursorPos( window, &mx, &my );

        hoveredAtom = PickAtom( (int)mx, (int)my,
            WIN_W, WIN_H,
            atoms.getAtoms(),
            {} );  // Only pick real atoms for tooltip
    }
}


void mouse_button_callback( GLFWwindow *, int button, int action, int ) {
    double mx, my;
    glfwGetCursorPos( window, &mx, &my );


    if (button == GLFW_MOUSE_BUTTON_LEFT &&
        action == GLFW_PRESS &&
        currentMode == ControlMode::PICK_DRAG)
    {
        Atom *hit = PickAtom( static_cast<int>(mx), static_cast<int>(my),
            WIN_W, WIN_H,
            atoms.getAtoms(), atoms.getLonePairs() );

        if (!hit)
        {
            if (!buildSymbol.empty())                         
            {
                glm::vec3 dir = screenRay( (int)mx, (int)my );
                float t = (0.0f - camera.Position.y) / dir.y; // plane-intersection
                glm::vec3 pos = camera.Position + dir * t;

                atoms.spawnAtom( buildSymbol, pos );

               // buildSymbol.clear();

                return;                                      
            }

            bondFirst = nullptr;
            dragging = false;
            selectedAtom = nullptr;
            return;
        }

        // Is the hit an actual atom or a lone-pair dot?
        int idx = std::find_if( atoms.getAtoms().begin(), atoms.getAtoms().end(),
            [&]( const Atom &a ) { return &a == hit; } )
            - atoms.getAtoms().begin();

        if (idx >= static_cast<int>(atoms.getAtoms().size()))
        {   // Lone-pair: just start drag
            selectedAtom = hit;
            dragging = true;

            grabPlaneY = selectedAtom->position.y;
            grabRayDir = screenRay( static_cast<int>(mx), static_cast<int>(my) );
            return;
        }

        if (bondFirst && bondFirst != hit)
        {
            int idx0 = std::find_if( atoms.getAtoms().begin(), atoms.getAtoms().end(),
                [&]( const Atom &a ) { return &a == bondFirst; } )
                - atoms.getAtoms().begin();
            if (idx0 < static_cast<int>( atoms.getAtoms().size() ))
            {
                BondType t = (nextOrder == 1) ? BondType::SINGLE :
                    (nextOrder == 2) ? BondType::DOUBLE :
                    BondType::TRIPLE;
                atoms.createBond( idx0, idx, t );
            }
            bondFirst = nullptr;
        }
        else
        {
            bondFirst = hit;          // first atom of a future bond
        }

        selectedAtom = hit;
        dragging = true;

        grabPlaneY = selectedAtom->position.y;
        grabRayDir = screenRay( static_cast<int>(mx), static_cast<int>(my) );
    }


    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        dragging = false;
        selectedAtom = nullptr;
    }



    if (button == GLFW_MOUSE_BUTTON_RIGHT &&
        action == GLFW_PRESS &&
        currentMode == ControlMode::PICK_DRAG)
    {
        Atom *hit = PickAtom( static_cast<int>(mx), static_cast<int>(my),
            WIN_W, WIN_H, atoms.getAtoms(), {} );

        if (!hit)
        {                     
            breakFirst = nullptr;
            return;
        }

        if (breakFirst == nullptr)
        {    // first atom of the pair
            breakFirst = hit;
            return;
        }

        if (breakFirst == hit)
        {        
            breakFirst = nullptr;
            return;
        }

        auto &B = atoms.getBonds();
        for (int i = 0; i < static_cast<int>( B.size() ); ++i)
        {
            const bool match = ((B[ i ].atomA == breakFirst && B[ i ].atomB == hit) ||
                (B[ i ].atomA == hit && B[ i ].atomB == breakFirst));
            if (match)
            {
                auto erasePtr = []( Atom *tgt, Atom *oth ) {
                    auto &v = tgt->bondedAtoms;
                    v.erase( std::remove( v.begin(), v.end(), oth ), v.end() );
                    };
                erasePtr( B[ i ].atomA, B[ i ].atomB );
                erasePtr( B[ i ].atomB, B[ i ].atomA );
                atoms.bonds.erase( atoms.bonds.begin() + i );
                break;
            }
        }
        breakFirst = nullptr;          
    }
}


void scroll_callback(GLFWwindow*, double, double yoff) {
    camera.ProcessMouseScroll((float)yoff);
}


void processInput( GLFWwindow *w ) {
   // if (glfwGetKey( w, GLFW_KEY_ESCAPE ) == GLFW_PRESS)
    //    glfwSetWindowShouldClose( w, true );

    if (glfwGetKey( w, GLFW_KEY_ESCAPE ) == GLFW_PRESS) {
        if (!buildSymbol.empty() &&
            currentMode == ControlMode::PICK_DRAG) {
            buildSymbol.clear();
            return;
        }
         glfwSetWindowShouldClose( w, true );
    }

    static bool lastI = false;
    bool nowI = glfwGetKey( w, GLFW_KEY_I ) == GLFW_PRESS;
    if (nowI && !lastI)                                            // edge
    {
        const char *inp = tinyfd_inputBox(
            "Insert element / ion",
            "Enter atomic symbol or ion (e.g. H, Cl-, Mg2+):", "" );
        if (inp && *inp)                                           // OK + non-empty
        {
            buildSymbol = inp;         // switch to spawn mode
            deleteMode = false;       // leave eraser if it was on
        }
    }
    lastI = nowI;

    static bool lastX = false;
    bool nowX = glfwGetKey( w, GLFW_KEY_X ) == GLFW_PRESS;
    if (currentMode == ControlMode::PICK_DRAG && nowX && !lastX)   
    {
        deleteMode = !deleteMode;
        if (deleteMode) buildSymbol.clear();   // cannot spawn while erasing
    }
    lastX = nowX;

    if (currentMode == ControlMode::FLY_CAMERA)
    {
        if (glfwGetKey( w, GLFW_KEY_W ) == GLFW_PRESS)
            camera.ProcessKeyboard( Camera_Movement::FORWARD, deltaTime );
        if (glfwGetKey( w, GLFW_KEY_S ) == GLFW_PRESS)
            camera.ProcessKeyboard( Camera_Movement::BACKWARD, deltaTime );
        if (glfwGetKey( w, GLFW_KEY_A ) == GLFW_PRESS)
            camera.ProcessKeyboard( Camera_Movement::LEFT, deltaTime );
        if (glfwGetKey( w, GLFW_KEY_D ) == GLFW_PRESS)
            camera.ProcessKeyboard( Camera_Movement::RIGHT, deltaTime );
    }

    {
        static bool last[ 3 ] = { false,false,false };
        const int  keys[ 3 ] = { GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3 };
        for (int i = 0; i < 3; ++i)
        {
            bool now = glfwGetKey( w, keys[ i ] ) == GLFW_PRESS;
            if (now && !last[ i ])            // edge
                nextOrder = i + 1;          
            last[ i ] = now;
        }
    }

    static bool tabLast = false;
    bool tabNow = glfwGetKey( w, GLFW_KEY_TAB ) == GLFW_PRESS;
    if (tabNow && !tabLast)                                       // edge
    {
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
    tabLast = tabNow;
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
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    window = glfwCreateWindow( WIN_W, WIN_H, "Molecule Viewer", nullptr, nullptr );
    if (!window)
    {
        std::cerr << "GLFW init failed\n"; return 1;
    }
    glfwMakeContextCurrent( window );
    glfwSetFramebufferSizeCallback( window, framebuffer_size_callback );
    glfwSetCursorPosCallback( window, mouse_callback );
    glfwSetScrollCallback( window, scroll_callback );
    glfwSetMouseButtonCallback( window, mouse_button_callback );
    glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );

    if (!gladLoadGLLoader( (GLADloadproc)glfwGetProcAddress ))
    {
        std::cerr << "GLAD init failed\n"; return 1;
    }
    glEnable( GL_DEPTH_TEST );

    Renderer::Init( &camera );
    textRenderer.Init();


   // buildH2O( atoms );

    while (!glfwWindowShouldClose( window ))
    {
        float now = (float)glfwGetTime();
        deltaTime = now - lastFrame;
        lastFrame = now;

        processInput( window );
        atoms.update( deltaTime );

        glClearColor( 0.05f, 0.05f, 0.05f, 1.f );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        int w, h; glfwGetFramebufferSize( window, &w, &h );
        Renderer::DrawGrid( w, h );

        atoms.updateLonePairs();
        atoms.updateFormalCharges();
        atoms.render( w, h );


        float totalPolarityMagnitude = atoms.computeDipole();
        textRenderer.DrawScreenText( currentPrebuiltAtom + " -- Polarity (partial charge and bond): " + std::string( atoms.isPolar ? "Polar" : "Non-Polar" ),
            10, 30, w, h );
        textRenderer.DrawScreenText( "Dipole Magnitude: " + std::to_string( totalPolarityMagnitude ),
            10, 50, w, h );
        textRenderer.DrawScreenText( "Adjustment Magnitude: " + std::to_string( atoms.latestCorrectionStrength ),
            10, 70, w, h );
        textRenderer.DrawScreenText( "Single Bonds: " + std::to_string( atoms.singleBonds ),
            10, 110, w, h );
        textRenderer.DrawScreenText( "Double Bonds: " + std::to_string( atoms.doubleBonds ),
            10, 130, w, h );
        textRenderer.DrawScreenText( "Triple Bonds: " + std::to_string( atoms.tripleBonds ),
            10, 150, w, h );
        textRenderer.DrawScreenText( "Sigma Bonds: " + std::to_string( atoms.sigmaBonds ),
            10, 170, w, h );
        textRenderer.DrawScreenText( "PI Bonds: " + std::to_string( atoms.piBonds ),
            10, 190, w, h );
        textRenderer.DrawScreenText( "Simulation Stability: " + std::to_string( 1 - (0.5 * atoms.latestCorrectionStrength) ),
            10, 210, w, h );
        textRenderer.DrawScreenText( "W-A-S-D to move, TAB to toggle",
            10, 230, w, h );
        textRenderer.DrawScreenText( "Press TAB to edit----------",
            10, 250, w, h );
        textRenderer.DrawScreenText( "  RClick to delete bonds, LClick + 1/2/3 to make bonds",
            10, 270, w, h );
        textRenderer.DrawScreenText( "  Press I to enter atom, then click screen to place",
            10, 290, w, h );


        glfwSwapBuffers( window );
        glfwPollEvents();
    }

    Renderer::Shutdown();
    glfwTerminate();
    return 0;
}