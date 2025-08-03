#include <imgui_impl_glfw.h>

#include "InputHandler.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <iostream>


glm::vec3 ScreenRay( const InputContext &ctx, int mx, int my ) {
    float ndcX = 2.f * mx / ctx.winWidth - 1.f;
    float ndcY = -2.f * my / ctx.winHeight + 1.f;
    glm::vec4 clip( ndcX, ndcY, -1.f, 1.f );

    glm::mat4 proj = glm::perspective( glm::radians( ctx.camera->Zoom ),
        float( ctx.winWidth ) / ctx.winHeight,
        0.1f, 100.f );
    glm::vec4 eye = glm::inverse( proj ) * clip;
    eye = glm::vec4( eye.x, eye.y, -1.f, 0.f );
    glm::vec3 dir = glm::normalize( glm::vec3( glm::inverse( ctx.camera->GetViewMatrix() ) * eye ) );
    return dir;
}


Atom *PickAtom( const InputContext &ctx,
    int mx, int my,
    const std::vector<Atom> &atoms,
    const std::vector<Atom> &lonePairs ) {
    glm::vec3 O = ctx.camera->Position;
    glm::vec3 D = ScreenRay( ctx, mx, my );

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


namespace
{
    InputContext *g_ctx = nullptr;
}

static void framebuffer_size_callback( GLFWwindow *, int w, int h ) {
    glViewport( 0, 0, w, h );
    if (g_ctx)
    {
        g_ctx->winWidth = w; g_ctx->winHeight = h;
    }
}

static void mouse_callback( GLFWwindow* w, double xpos, double ypos ) {
    ImGui_ImplGlfw_CursorPosCallback( w, xpos, ypos );

    InputContext &ctx = *g_ctx;
    if (ctx.firstMouse)
    {
        ctx.lastX = (float)xpos; ctx.lastY = (float)ypos; ctx.firstMouse = false;
    }

    float xoffset = (float)xpos - ctx.lastX;
    float yoffset = ctx.lastY - (float)ypos;
    ctx.lastX = (float)xpos; ctx.lastY = (float)ypos;

    if (ctx.currentMode == ControlMode::FLY_CAMERA)
    {
        ctx.camera->ProcessMouseMovement( xoffset, yoffset );
    }
    else if (ctx.dragging && ctx.selectedAtom)
    {
        // project atom along ray keeping its depth
        float depth = glm::length( ctx.selectedAtom->position - ctx.camera->Position );

        glm::mat4 proj = glm::perspective( glm::radians( ctx.camera->Zoom ),
            float( ctx.winWidth ) / ctx.winHeight,
            0.1f, 100.f );
        glm::mat4 view = ctx.camera->GetViewMatrix();
        glm::mat4 invVP = glm::inverse( proj * view );

        float ndcX = 2.f * xpos / ctx.winWidth - 1.f;
        float ndcY = 1.f - 2.f * ypos / ctx.winHeight;
        glm::vec4 ndc( ndcX, ndcY, 1.f, 1.f );
        glm::vec4 world = invVP * ndc; world /= world.w;
        glm::vec3 newDir = glm::normalize( glm::vec3( world ) - ctx.camera->Position );

        ctx.selectedAtom->position = ctx.camera->Position + newDir * depth;
        ctx.selectedAtom->velocity = glm::vec3( 0.f );

        constexpr float FLOOR_Y = -0.2f;
        if (ctx.selectedAtom->position.y - ctx.selectedAtom->radius < FLOOR_Y)
            ctx.selectedAtom->position.y = FLOOR_Y + ctx.selectedAtom->radius;
    }

    // update hover tooltip
    if (ctx.currentMode == ControlMode::PICK_DRAG && !ctx.dragging)
    {
        double mx, my; glfwGetCursorPos( ctx.window, &mx, &my );
        ctx.hoveredAtom = PickAtom( ctx, (int)mx, (int)my, ctx.atoms->getAtoms(), {} );
    }
}

static void mouse_button_callback( GLFWwindow *w, int button, int action, int mods) {
    ImGui_ImplGlfw_MouseButtonCallback( w, button, action, mods );

    InputContext &ctx = *g_ctx;
    double mx, my; glfwGetCursorPos( ctx.window, &mx, &my );

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS &&
        ctx.currentMode == ControlMode::PICK_DRAG)
    {
        Atom *hit = PickAtom( ctx, (int)mx, (int)my, ctx.atoms->getAtoms(), ctx.atoms->getLonePairs() );

        if (!hit)
        {
            if (!ctx.buildSymbol.empty())
            {
                glm::vec3 dir = ScreenRay( ctx, (int)mx, (int)my );
                float t = (0.f - ctx.camera->Position.y) / dir.y;
                glm::vec3 pos = ctx.camera->Position + dir * t;
                ctx.atoms->spawnAtom( ctx.buildSymbol, pos );
                ctx.buildSymbol.clear();
            }
            ctx.bondFirst = nullptr; ctx.dragging = false; ctx.selectedAtom = nullptr;
            return;
        }

        bool isRealAtom = std::find_if( ctx.atoms->getAtoms().begin(), ctx.atoms->getAtoms().end(),
            [&]( const Atom &a ) {return &a == hit; } ) != ctx.atoms->getAtoms().end();
        if (!isRealAtom)
        {
            ctx.selectedAtom = hit; ctx.dragging = true;
            ctx.grabPlaneY = hit->position.y;
            ctx.grabRayDir = ScreenRay( ctx, (int)mx, (int)my );
            return;
        }

        if (ctx.bondFirst && ctx.bondFirst != hit)
        {
            // second click – actually create bond
            int idx0 = std::find_if( ctx.atoms->getAtoms().begin(), ctx.atoms->getAtoms().end(),
                [&]( const Atom &a ) {return &a == ctx.bondFirst; } ) - ctx.atoms->getAtoms().begin();
            int idx1 = std::find_if( ctx.atoms->getAtoms().begin(), ctx.atoms->getAtoms().end(),
                [&]( const Atom &a ) {return &a == hit; } ) - ctx.atoms->getAtoms().begin();
            if (idx0 < (int)ctx.atoms->getAtoms().size())
            {
                BondType t = ctx.nextOrder == 1 ? BondType::SINGLE :
                    ctx.nextOrder == 2 ? BondType::DOUBLE : BondType::TRIPLE;
                ctx.atoms->createBond( idx0, idx1, t );
            }
            ctx.bondFirst = nullptr;
        }
        else
        {
            ctx.bondFirst = hit; // first click
        }
        ctx.selectedAtom = hit; ctx.dragging = true;
        ctx.grabPlaneY = hit->position.y;
        ctx.grabRayDir = ScreenRay( ctx, (int)mx, (int)my );
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        ctx.dragging = false; ctx.selectedAtom = nullptr;
    }


    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS &&
        ctx.currentMode == ControlMode::PICK_DRAG)
    {
        Atom *hit = PickAtom( ctx, (int)mx, (int)my, ctx.atoms->getAtoms(), {} );
        if (!hit)
        {
            ctx.breakFirst = nullptr; return;
        }
        if (ctx.breakFirst == nullptr)
        {
            ctx.breakFirst = hit; return;
        }
        if (ctx.breakFirst == hit)
        {
            ctx.breakFirst = nullptr; return;
        }

        auto &B = ctx.atoms->getBonds();
        for (int i = 0; i < (int)B.size(); ++i)
        {
            bool match = (B[ i ].atomA == ctx.breakFirst && B[ i ].atomB == hit) ||
                (B[ i ].atomA == hit && B[ i ].atomB == ctx.breakFirst);
            if (match)
            {
                // update bond counters
                switch (B[ i ].type)
                {
                case BondType::SINGLE: --ctx.atoms->singleBonds; --ctx.atoms->sigmaBonds; break;
                case BondType::DOUBLE: --ctx.atoms->doubleBonds; --ctx.atoms->sigmaBonds; --ctx.atoms->piBonds; break;
                case BondType::TRIPLE: --ctx.atoms->tripleBonds; --ctx.atoms->sigmaBonds; ctx.atoms->piBonds -= 2; break;
                }
                // remove from adjacency lists
                auto erasePtr = []( Atom *a, Atom *b ) {
                    auto &v = a->bondedAtoms;
                    v.erase( std::remove( v.begin(), v.end(), b ), v.end() ); };
                erasePtr( B[ i ].atomA, B[ i ].atomB );
                erasePtr( B[ i ].atomB, B[ i ].atomA );
                ctx.atoms->bonds.erase( ctx.atoms->bonds.begin() + i );
                ctx.atoms->firstCentralGeometry.clear();
                ctx.atoms->setDirtyLonePairs();
                break;
            }
        }
        ctx.breakFirst = nullptr;
    }
}

static void scroll_callback( GLFWwindow *w, double xoff, double yoff ) {
    ImGui_ImplGlfw_ScrollCallback( w, xoff, yoff );

    g_ctx->camera->ProcessMouseScroll( (float)yoff );
}


void InitInputHandler( InputContext &ctx ) {
    g_ctx = &ctx;
    // GLFW callbacks
    glfwSetFramebufferSizeCallback( ctx.window, framebuffer_size_callback );
    glfwSetCursorPosCallback( ctx.window, mouse_callback );
    glfwSetMouseButtonCallback( ctx.window, mouse_button_callback );
    glfwSetScrollCallback( ctx.window, scroll_callback );

    glfwSetInputMode( ctx.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
}


// Call once per frame from the main loop
void ProcessInput( InputContext &ctx, float deltaTime ) {
    ImGuiIO &io = ImGui::GetIO();

    if (!io.WantCaptureKeyboard)
    {
        if (glfwGetKey( ctx.window, GLFW_KEY_ESCAPE ) == GLFW_PRESS)
        {
            if (!ctx.buildSymbol.empty() && ctx.currentMode == ControlMode::PICK_DRAG)
            {
                ctx.buildSymbol.clear();
                return;
            }
            glfwSetWindowShouldClose( ctx.window, true );
        }

        static bool lastX = false;
        bool nowX = glfwGetKey( ctx.window, GLFW_KEY_X ) == GLFW_PRESS;
        if (ctx.currentMode == ControlMode::PICK_DRAG && nowX && !lastX)
        {
            ctx.deleteMode = !ctx.deleteMode;
            if (ctx.deleteMode)
                ctx.buildSymbol.clear();
        }
        lastX = nowX;

        // Fly camera WASD
        if (ctx.currentMode == ControlMode::FLY_CAMERA)
        {
            if (glfwGetKey( ctx.window, GLFW_KEY_W ) == GLFW_PRESS)
                ctx.camera->ProcessKeyboard( Camera_Movement::FORWARD, deltaTime );
            if (glfwGetKey( ctx.window, GLFW_KEY_S ) == GLFW_PRESS)
                ctx.camera->ProcessKeyboard( Camera_Movement::BACKWARD, deltaTime );
            if (glfwGetKey( ctx.window, GLFW_KEY_A ) == GLFW_PRESS)
                ctx.camera->ProcessKeyboard( Camera_Movement::LEFT, deltaTime );
            if (glfwGetKey( ctx.window, GLFW_KEY_D ) == GLFW_PRESS)
                ctx.camera->ProcessKeyboard( Camera_Movement::RIGHT, deltaTime );
        }

        // Bond order hotkeys 1/2/3
        static bool lastKey[ 3 ] = { false, false, false };
        for (int i = 0; i < 3; ++i)
        {
            bool pressed = glfwGetKey( ctx.window, GLFW_KEY_1 + i ) == GLFW_PRESS;
            if (pressed && !lastKey[ i ])
                ctx.nextOrder = i + 1;
            lastKey[ i ] = pressed;
        }

        // TAB – switch modes
        static bool lastTab = false;
        bool tab = glfwGetKey( ctx.window, GLFW_KEY_TAB ) == GLFW_PRESS;
        if (tab && !lastTab)
        {
            if (ctx.currentMode == ControlMode::FLY_CAMERA)
            {
                ctx.currentMode = ControlMode::PICK_DRAG;
                glfwSetInputMode( ctx.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
            }
            else
            {
                ctx.currentMode = ControlMode::FLY_CAMERA;
                glfwSetInputMode( ctx.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
            }
        }
        lastTab = tab;
    }

}
