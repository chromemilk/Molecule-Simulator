# 3D Particle Simulator

A real-time 3D particle simulation written in modern C++ using OpenGL.

## ✨ Features
- Particle system with position, velocity, and lifespan
- Real-time physics updates
- Basic rendering using OpenGL
- Modular and extensible C++ structure
- Easy to expand with new particle behaviors (gravity, collisions, etc.)
- Model interations using Valence Electron Shell Repulsion Theory 

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
- Shader-based particle rendering
- Gravity and force fields
- Collisions with planes and spheres
- Particle emitters
- GPU compute particles (for millions of particles!)

## 🎨 Future Ideas
- Add a GUI (ImGui) for real-time tweaking
- Add volumetric effects (smoke, fire, water simulation)
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
./ParticleSimulator


