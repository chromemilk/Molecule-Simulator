#pragma once
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "Camera.h"

struct GPUSphere { glm::vec3 c; float r; };

class Raytracer {
public:
    void init();                                   // call once after GL init
    void setScene(const std::vector<GPUSphere>& s);  // every frame
    void render(const Camera& cam, int fbW, int fbH);// draws a full-screen tri
private:
    GLuint prog = 0;                               // ray-tracing shader
    GLuint vao = 0;                               // full-screen triangle
    int    sphereCount = 0;
    std::vector<GLfloat> sphereFlat;                 // packed vec4 array
};
