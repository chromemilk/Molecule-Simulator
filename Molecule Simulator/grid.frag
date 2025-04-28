#version 330 core
out vec4 FragColor;

in vec3 WorldPos;

uniform vec3 cameraPos;

void main()
{
    // Distance from camera to fragment
    float dist = length(cameraPos.xz - WorldPos.xz);

    // Dynamically change grid scale with distance
    float gridScale = mix(5.0, 20.0, smoothstep(30.0, 150.0, dist));
    vec2 grid = WorldPos.xz * gridScale;

    // Dynamically adjust line thickness
    float baseLineWidth = 0.02;
    float lineWidth = baseLineWidth * mix(1.0, 0.3, smoothstep(20.0, 150.0, dist));

    // Calculate distance to nearest grid line
    float d = min(
        abs(fract(grid.x) - 0.5),
        abs(fract(grid.y) - 0.5)
    );

    // Sharper line edges
    float line = smoothstep(0.0, lineWidth, d);

    // Background and Grid color
    vec3 background = vec3(0.02, 0.02, 0.03);
    vec3 gridColor  = vec3(0.2, 0.6, 1.0);

    // Blend between grid and background based on line mask
    vec3 finalColor = mix(gridColor, background, line);

    // Fade the whole grid away at very far distances
    float fade = smoothstep(100.0, 200.0, dist);
    finalColor = mix(finalColor, background, fade);

    FragColor = vec4(finalColor, 1.0);
}
