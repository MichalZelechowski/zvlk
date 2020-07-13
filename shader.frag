#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform CameraUbo {
    mat4 view;
    mat4 proj;
} cameraUbo;

struct Light {
  vec3 position;
  vec4 color;
  float attenuation;
};

#define MAX_LIGHTS 8
layout(set = 0, binding = 1) uniform LightsUbo {
    float numberOfLights;
    Light lights[MAX_LIGHTS];
} lightsUbo;

layout(set = 2, binding = 0) uniform sampler2D texSampler;
layout(set = 2, binding = 1) uniform MaterialUbo {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float shiness;
} materialUbo;

void main() {
    outColor = vec4(0.0);

    for (int i=0; i< lightsUbo.numberOfLights; ++i) {
        vec3 lightPosition = lightsUbo.lights[i].position;
        vec4 lightColor = lightsUbo.lights[i].color;
        vec3 cameraPosition = cameraUbo.view[3].xyz;

        //diffuse
        vec3 normal = normalize(inNormal);
        vec3 lightDirection = normalize(lightPosition - inPosition);  
        float diffuse = max(dot(normal, lightDirection), 0.0);

        //specular
        vec3 viewDirection = normalize(cameraPosition - inPosition);
        vec3 reflectDirection = reflect(-lightDirection, normal);
        float specular = pow(max(dot(viewDirection, reflectDirection), 0.0), materialUbo.shiness);  

        //total
        outColor += vec4(0.1 * materialUbo.ambient.rgb + diffuse * materialUbo.diffuse.rgb + specular * materialUbo.specular.rgb, 1.0) * lightColor;
    }
    outColor = vec4(outColor.xyz, 1.0);
}