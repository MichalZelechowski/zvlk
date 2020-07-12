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
    vec4 lightColor = vec4(1.0, 1.0, 1.0, 1.0);
    vec3 cameraPosition = vec3(1000.0, 1000.0, 1000.0);

    //diffuse
    vec3 normal = normalize(inNormal);
    vec3 lightDirection = normalize(lightPosition - inPosition);  
    float diffuse = max(dot(normal, lightDirection), 0.0);

    //specular
    vec3 viewDirection = normalize(cameraPosition - inPosition);
    vec3 reflectDirection = reflect(-lightDirection, normal);
    float specular = pow(max(dot(viewDirection, reflectDirection), 0.0), materialUbo.shiness);  

    //total
    outColor = vec4(materialUbo.ambient.rgb + diffuse * materialUbo.diffuse.rgb + specular * materialUbo.specular.rgb, 1.0) * lightColor;
}