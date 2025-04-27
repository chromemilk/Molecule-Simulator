#version 330 core
in vec3 vNormal;
in vec3 vWorldPos;

uniform vec3 color;

out vec4 FragColor;

/* single white directional light */
const vec3  lightDir = normalize(vec3(0.3, 0.7, 1.0));
const vec3  ambient  = vec3(0.15);

void main()
{
    float diff = max(dot(normalize(vNormal), lightDir), 0.0);
    vec3  finalColour = color * (ambient + diff*0.85);

    FragColor = vec4(finalColour, 1.0);
}
