#version 330 core
out vec4 FragColor;

in vec3 WorldPos;

uniform vec3 cameraPos;

void main()
{
    float gridScale = 5.0;
    vec2 grid = WorldPos.xz * gridScale;

    float lineWidth = 0.02;

    float d = min(
        abs(fract(grid.x) - 0.5),
        abs(fract(grid.y) - 0.5)
    );

    float line = smoothstep(0.0, lineWidth, d);

    // Distance from camera to grid fragment
    float dist = length(cameraPos.xz - WorldPos.xz);

    // Fadeout with distance
    float fade = smoothstep(30.0, 80.0, dist);

    vec3 background = vec3(0.02, 0.02, 0.03);
    vec3 gridColor  = vec3(0.2, 0.6, 1.0);

    vec3 finalColor = mix(gridColor, background, line);
    finalColor = mix(finalColor, background, fade); // fade farther away

    FragColor = vec4(finalColor, 1.0);
}
