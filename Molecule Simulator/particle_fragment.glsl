#version 330 core
out vec4 FragColor;

uniform vec3 particleColor;

void main() {
    // Calculate distance from center
    float dist = length(gl_PointCoord - vec2(0.5));
    if (dist > 0.5)
        discard; // outside of circle

    FragColor = vec4(particleColor, 1.0 - dist); // fade edges
}
