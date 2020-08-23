#version 430 core

uniform vec3 lightPosition;
uniform sampler2D diffuseTexture;

in vec4 fragPosition;
in vec3 fragNormal;
in vec2 fragUV;

out vec4 FragColor;

void main()
{
    float u = fragUV.x;
    float v = fragUV.y;

    vec3 lightDirection = normalize(lightPosition - fragPosition.xyz);

    vec3 normal = normalize(fragNormal);
    float diffuse = max(dot(normal, lightDirection), 0.0);

    vec4 grass_base_color = texture(diffuseTexture, fragUV);
    vec3 ambient = (grass_base_color.xyz * 0.2) * v;
    vec3 result_color = ambient + grass_base_color.xyz; 

    FragColor = vec4(result_color, 1.0);
};