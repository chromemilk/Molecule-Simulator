# 3D Molecule Simulator

A real-time 3D simulation written in modern C++ using OpenGL.

## Example




https://github.com/user-attachments/assets/ce91ff1f-9787-4f5e-83ff-46e27470fa9b



https://github.com/user-attachments/assets/a79641f6-50ad-49c7-9c50-a4042feb9597




## ✨ Features
- Electron geometry calculations
- Predicts and dynamically finds bond angles
- Finds and models lone pairs
- Uses VSEPR methods to compute geometry
- Computes dipoles and displays molecule polarity/polarity magnitude
- Molecule simulations are build on a custom particle engine
- Extended periodic table for enhanced lookups
- Loads shaders and creates meshes for full 3D enhancements
- Grid
- Uses a flying camera for 360 view of any structure
- Model interations using Valence Electron Shell Repulsion Theory (dynamically computes lone pair positions using VSEPR theory)
- Easy to use molecule builder (new)
- Formal charge calculations (new)

## 📚 Tech Stack
- **C++17**
- **OpenGL 3.3+**
- **GLFW** — Window and input handling
- **GLAD** — OpenGL function loader
- **GLM** — Mathematics library for 3D transformations
- **CMake** — Build system

## 🚀 Getting Started

### Prerequisites
Make sure you have installed:
- C++ compiler supporting C++17
- CMake >= 3.10
- GLFW
- GLAD
- GLM

## 🛠️ Planned Features
- Chemical equation parsing
- Automatic resonance structure computation
- Correction stability system (currently structures fly around until ideal angles are reached)
- Dynamic light shaders with raytracing
- GUI

## 🎨 Future Ideas
- Add a GUI (ImGui) for real-time tweaking
- Optimizations: spatial partitioning, multithreading

## 📄 License
This project is licensed under the MIT License. See the LICENSE file for details.

## 👏 Acknowledgements
LearnOpenGL — Great resource for modern OpenGL

TheCherno — Fantastic C++ and OpenGL tutorials

Open-source contributors to GLFW, GLM, and GLAD


You can install dependencies via a package manager (like `vcpkg`, `brew`, `apt`) or build them manually.

### Building the Project

```bash
git clone https://github.com/yourusername/3D-Particle-Simulator.git
cd 3D-Particle-Simulator
mkdir build
cd build
cmake ..
make
./Molecule Simulator




