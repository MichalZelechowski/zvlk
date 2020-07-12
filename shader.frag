#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

layout(set = 2, binding = 0) uniform sampler2D texSampler;
layout(set = 2, binding = 1) uniform MaterialUbo {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float shiness;
} materialUbo;

void main() {
    vec3 lightPosition = vec3(1000.0, 1000.0, 1000.0);
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    vec3 cameraPosition = vec3(1000.0, 1000.0, 1000.0);

    //ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    //diffuse
    vec3 normal = normalize(inNormal);
    vec3 lightDirection = normalize(lightPosition - inPosition);  
    vec3 diffuse = max(dot(normal, lightDirection), 0.0) * lightColor;

    //specular
    float specularStrength = 0.5; //TODO in material
    vec3 viewDirection = normalize(cameraPosition - inPosition);
    vec3 reflectDirection = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), materialUbo.shiness);
    vec3 specular = specularStrength * spec * lightColor;  

    //total
    outColor = vec4(ambient * materialUbo.ambient.rgb + diffuse * materialUbo.diffuse.rgb + specular * materialUbo.specular.rgb, 1.0);
}