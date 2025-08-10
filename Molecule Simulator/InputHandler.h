
#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "AtomSystem.h"
#include "Camera.h"
#include "TextRenderer.h"

// Screen size – keep identical to the values in main.cpp
constexpr int WIN_W = 1280;
constexpr int WIN_H = 720;

enum class ControlMode
{
    FLY_CAMERA, PICK_DRAG
};


struct InputContext
{
    GLFWwindow *window = nullptr;
    Camera *camera = nullptr;
    AtomSystem *atoms = nullptr;
    TextRenderer *textRenderer = nullptr;   

    std::string currentPrebuiltAtom;
    std::string buildSymbol;
    bool deleteMode = false;

    // Bond order to create with the next click pair (1/2/3)
    int nextOrder = 1;

    ControlMode currentMode = ControlMode::FLY_CAMERA;

    // Picking / dragging bookkeeping
    Atom *selectedAtom = nullptr;
    Atom *hoveredAtom = nullptr;
    Atom *bondFirst = nullptr;   // first click when making bonds
    Atom *breakFirst = nullptr;   // first click when breaking bonds
    bool  dragging = false;

    float   grabPlaneY = 0.f;       // plane y-coord where grab started
    glm::vec3 grabRayDir;

    bool firstMouse = true;
    float lastX = 0.f, lastY = 0.f;

    int winWidth = WIN_W;
    int winHeight = WIN_H;
};

void InitInputHandler( InputContext &ctx );

void ProcessInput( InputContext &ctx, float deltaTime );

glm::vec3 ScreenRay( const InputContext &ctx, int mx, int my );

Atom *PickAtom( const InputContext &ctx,
    int mx, int my,
    const std::vector<Atom> &atoms,
    const std::vector<Atom> &lonePairs );
