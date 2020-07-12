#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;

layout(set = 2, binding = 0) uniform sampler2D texSampler;
layout(set = 2, binding = 1) uniform MaterialUbo {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float shiness;
} materialUbo;

void main() {
    outColor = materialUbo.diffuse;
}