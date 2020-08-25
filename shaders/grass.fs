#version 430 core

uniform vec3 lightPosition;
uniform vec3 cameraPosition;
uniform sampler2D diffuseTexture;

uniform int iSwitch;

in vec4 fragPosition;
in vec3 fragNormal;
in vec2 fragUV;
in float fragAge;

out vec4 FragColor;

void main()
{
    float u = fragUV.x;
    float v = fragUV.y;

    float ambientCoeff = 0.8;
    float diffuseCoeff = 1.0;

    vec3 lightDir = normalize(lightPosition - fragPosition.xyz);
    vec3 vert2camDir = normalize(cameraPosition - fragPosition.xyz);

    vec3 normal = normalize(vec3(0.0, pow(v, 1.75), 0.0));
    
    float diffuse = max(dot(normal, lightDir), 0.0);
    float yShading = min(v+0.2, 1.0);
    vec3 refl = normalize(reflect(lightDir, normal));

    vec4 grass_base_color = texture(diffuseTexture, fragUV);
    vec4 grass_age_color = vec4(fragAge, 1.0, 0.4, 1.0);

    grass_base_color.r = mix(grass_base_color.r, grass_age_color.r, fragPosition.w);
    
    vec3 result_color_rgb = (ambientCoeff + diffuse*diffuseCoeff) * grass_base_color.rgb * yShading;

    FragColor = vec4(result_color_rgb, 1.0);

}