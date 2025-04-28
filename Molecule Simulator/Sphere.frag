#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 color;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform float emissiveBoost;


void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);

    vec3 ambient = 0.2 * color;
    vec3 diffuse = 0.6 * diff * color;
    vec3 specular = 0.4 * spec * vec3(1.0);
    vec3 result = ambient + diffuse + specular;
    result += emissiveBoost * color; 
    FragColor = vec4(result, 1.0);

}
