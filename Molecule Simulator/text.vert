#version 330 core

layout(location = 0) in vec2 aPos;

uniform mat4 uProjection;
uniform float uScale;

void main() {
    // scale the 8×8-stb_easy_font coords up (or down)
    vec2 scaled = aPos * uScale;
    gl_Position = uProjection * vec4(scaled, 0.0, 1.0);
}
