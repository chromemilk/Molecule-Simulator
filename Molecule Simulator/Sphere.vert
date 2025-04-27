#version 330 core
layout(location = 0) in vec3 aPos;      // position only

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 vNormal;                      
out vec3 vWorldPos;

void main()
{
    // object-space position & normal
    vec3 objectPos    = aPos;
    vec3 objectNormal = normalize(aPos);          // unit sphere

    // world space
    vec3 worldPos = vec3(model * vec4(objectPos, 1.0));
    vec3 worldNrm = normalize(mat3(model) * objectNormal);

    vWorldPos = worldPos;
    vNormal   = worldNrm;

    gl_Position = projection * view * vec4(worldPos, 1.0);
}
