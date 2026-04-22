# 3D Molecule Simulator

A real-time 3D molecular structure and dynamics simulator written in modern C++17 using OpenGL 3.3+, GLFW, GLAD, GLM, and ImGui. Interactively build, visualize, and analyze molecular geometries and properties with force-based simulation and VSEPR methods.

## Table of Contents

* [Features](#features)
* [Preview](#preview)
* [Controls & UI](#controls--ui)
* [Architecture & Operations](#architecture--operations)
* [Tech Stack](#tech-stack)
* [Prerequisites](#prerequisites)
* [Building & Running](#building--running)
* [Planned Features](#planned-features)
* [Future Ideas](#future-ideas)

https://github.com/user-attachments/assets/4c46024b-c0af-442c-bb21-6fc3e0394bcf


* [License](#license)
* [Acknowledgements](#acknowledgements)

## Features

* **Interactive Molecule Builder**: Spawn atoms by symbol or input chemical formulas; auto-generate resonance‑optimized structures via a custom `Resonance::Generator`.
* **Force-Based Simulation**: Bonds modeled as springs apply real-time forces; atoms move under Verlet-like integration with damping and collision against a bounding box and floor.
* **VSEPR Geometry Calculations**: Dynamically compute lone‑pair positions and ideal bond angles using energy‑minimization or fast correction methods; display both real and ideal angles on-screen.
* **Formal Charge & Dipole Analysis**: Calculate formal charges per atom; compute net dipole moment from partial charges (fractional based on electronegativity differences) and show polarity vectors in 3D.
* **Lone Pair Visualization**: Generate virtual dots for lone pairs using tetrahedral or planar heuristics; update positions in real time as bonds form or break.
* **Multiple Bond Orders**: Display single, double, and triple bonds with separate cylinders; count and render sigma and pi bonds.
* **Simulation Stability Indicator**: Monitor VSEPR correction magnitude and show a percentage stability gauge.
* **Extended Periodic Table**: Look up atomic properties (atomic number, mass, electronegativity, valence) from a built-in table for hover tooltips and calculations.
* **Custom Particle Engine**: Lightweight engine without external physics libraries; supports fixed atoms and flight mode.
* **Flying Camera**: Free‑roaming 360° camera with optional auto-follow of molecule center.
* **Real-Time Interactions**: Create/delete bonds (1–3 order), drag atoms, and remove entire molecule on-the-fly.
* **Prebuilt Templates**: One‑click generation of common molecules: H₂O, CO₂, NH₃, CH₄, NO₂⁻, HCN, O₃, CN⁻.
* **Dynamic Lighting & Shaders**: Phong‑style shaders for atoms and bonds; raytracing module available (OpenGL 4.6 required).
* **ImGui-Based UI**: Tabbed menu for main controls, prebuilt selections, and usage instructions.

## Preview

## Controls & UI

### Camera & Interaction

* **W/A/S/D**: Move camera in flight mode.
* **Mouse Look**: Hold right mouse button and move to rotate camera.
* **TAB**: Toggle between Fly mode and Edit mode.
* **Left‑drag**: Grab and drag atoms in Edit mode.

### Bond Management

* **Click two atoms + \[1/2/3]**: Create a single, double, or triple bond.
* **Right‑click two atoms**: Break existing bond.
* **Delete Molecule**: Clears all atoms and bonds.

### ImGui Menu Tabs

* **Main**: Spawn atoms by symbol; input chemical formulas for auto‑resonance; toggle VSEPR correction, stability options, and camera follow.
* **Prebuilt**: One‑click generation of common molecules (H₂O, CO₂, NH₃, CH₄, NO₂⁻, HCN, O₃, CN⁻).
* **Instructions**: Keybindings and mouse operations summary.

Tooltips on hover display per-atom details: element symbol, atomic number, mass, electronegativity, dipole magnitude, lone pairs, bond counts (single/double/triple/sigma/pi).

## Architecture & Operations

The core of the simulator revolves around two tightly coupled subsystems: **Resonance & Formula Parsing** and the **AtomSystem & Simulation Loop**.

### Resonance & Formula Parsing

1. **Formula Tokenization** (`Parser::parseFormula`):

   * Reads chemical formulas (e.g. `C6H6`, `K4[ON(SO3)2]2`) and expands bracketed groups and multipliers.
   * Produces a flat list of element symbols and formal charges.
2. **Resonance Structure Generation** (`Resonance::Generator`):

   * Constructs all valid bonding graphs given valence rules and total electron counts.
   * Evaluates each graph by computing formal charges and selects the structure minimizing total charge deviations.
   * Returns a vector of `Bond` objects: atom index pairs with bond orders (single/double/triple).

### AtomSystem & Simulation Loop

1. **System Initialization** (`AtomSystem::AtomSystem(size, textRenderer)`):

   * Allocates storage for atoms and bonds; initializes rendering text overlay.
2. **Building a Molecule** (`AtomSystem::build(symbols, bonds)`):

   * **Spawn Atoms**: Places each atom at the computed center of mass.
   * **Attach Bonds**: Iterates `bonds` from the resonance generator or manual input; creates spring-like `Bond` instances.
   * **Initial Layout**: Offsets atom positions radially to avoid overlap, resets velocities.
3. **Main Loop** (`AtomSystem::updateAll(dt)` + `AtomSystem::renderAll(viewProj)`):

   * **Bond Forces** (`Bond::applyForce`): Hooke’s law spring forces between bonded atoms.
   * **VSEPR Corrections**:

     * **Force-Based** (`applyVSEPRForces`): Applies repulsive forces between electron domains (bonds & lone pairs).
     * **Fast-Angle** (`applyVSEPRAngleFast`): Directly snaps bond vectors toward ideal angles for tetrahedral, trigonal, etc.
   * **Integration** (`Atom::update(dt)`): Verlet-like position update with optional gravity, damping, and bounding collisions.
   * **Property Updates**:

     * Lone-pair positions via heuristics (tetrahedral, trigonal planar).
     * Formal charge recalculation per atom.
     * Partial-charge dipole computation: electronegativity-based partial charges aggregated into a net dipole vector.
   * **Render**:

     * Atoms as lit spheres, bonds as multi-cylinder segments for sigma/π distinctions.
     * Overlays: bond angles, lone-pair dots, stability gauge, and hover-tooltips.

## Tech Stack

* **Language**: C++17
* **Graphics**: OpenGL 3.3+ / 4.6
* **Windowing/Input**: GLFW
* **OpenGL Loader**: GLAD
* **Math**: GLM
* **UI**: ImGui
* **Build**: CMake >= 3.10

## Prerequisites

Install the following on your system:

* **C++17 compiler** (e.g., GCC 9+, Clang 9+, MSVC 2019+)
* **CMake** >= 3.10
* **GLFW**, **GLAD**, **GLM**, **ImGui**

You can install dependencies via package managers:

* **vcpkg**: \$1

## Testing Suite

A comprehensive set of automated tests validates parsing, resonance generation, and the atom system. The test suite is implemented in `Tests.cpp` and uses the built-in test harness to:

* **Parse & Expand**: Verify `Parser::parseFormula` correctly tokenizes formulas with nested groups and charges.
* **Resonance Validation**: Ensure `Resonance::Generator` produces the lowest-formal-charge structure for cases like NO₃⁻, benzene, and complex ions.
* **Atom System Checks**: Build molecules with `AtomSystem::build` and confirm correct atom counts and bond orders via lambdas in each `TestCase`.

Ensure `glfw`, `glad`, and `glm` headers are accessible to the test target for linking and compilation.

* To enable GPU raytracing (requires OpenGL 4.6), edit `main.cpp` and set `useRT = true`.

## Planned Features

* **Chemical Equation Parsing**: Support multi-component reactions and stoichiometry.
* **Automatic Resonance Structures**: Display all equivalent resonance forms.
* **Expanding Octet Rules**: Nuanced handling for period‑3+ elements.
* **GUI Panels**: ImGui windows for per-atom and per-bond property sliders.
* **Dynamic Raytraced Lighting**: Real-time raytracing integration.

## Future Ideas

* **Spatial Partitioning**: Octree/K‑d tree for performance on large molecules.
* **Multithreading**: Offload physics or rendering tasks to worker threads.
* **Scripting Interface**: Python or Lua API for automated workflows.
* **File I/O**: Import/export common formats (XYZ, PDB, MOL).
* **Advanced Hamiltonian Simulation**: Hook up semi-empirical quantum engines.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Acknowledgements

* **LearnOpenGL**: Comprehensive OpenGL tutorials.
* **TheCherno**: In-depth C++ and graphics programming series.
* **GLFW**, **GLAD**, **GLM**, **ImGui**: Open-source graphics libraries and UI framework.
