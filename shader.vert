#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform CameraUbo {
    mat4 view;
    mat4 proj;
    vec3 eye;
    vec3 center;
} cameraUbo;

layout(set = 1, binding = 0) uniform TransformationUbo {
    mat4 model;
} transformationUbo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec2 outTexCoord;
layout(location = 2) out vec3 outNormal;

void main() {
    outPosition = (transformationUbo.model * vec4(inPosition, 1.0)).xyz;
    outNormal = mat3(transpose(inverse(transformationUbo.model))) * inNormal;
    outTexCoord = inTexCoord;
    
    gl_Position = cameraUbo.proj * cameraUbo.view * vec4(outPosition, 1.0);
}