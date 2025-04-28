#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3  color;
uniform vec3  lightPos;
uniform vec3  viewPos;
uniform float emissiveBoost;     

void main()
{
    vec3 norm  = normalize(Normal);
    vec3 L     = normalize(lightPos - FragPos);
    float diff = max(dot(norm,L), 0.0);

    vec3 V  = normalize(viewPos - FragPos);
    vec3 H  = normalize(L + V);
    float spec = pow(max(dot(norm,H),0.0), 32.0);

    vec3 ambient  = 0.20 * color;
    vec3 diffuse  = 0.60 * diff * color;
    vec3 specular = 0.40 * spec * vec3(1.0);

    vec3 result = ambient + diffuse + specular;
    result += emissiveBoost * color;   // ? glow

    FragColor = vec4(result, 1.0);
}
