#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform CameraUbo {
    mat4 view;
    mat4 proj;
} cameraUbo;

layout(binding = 1) uniform TransformationUbo {
    mat4 model;
} transformationUbo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 normal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    vec4 transformedVertex = transformationUbo.model * vec4(inPosition, 1.0);
    vec4 transformedNormal = transformationUbo.model * vec4(inPosition+normal, 1.0) - transformedVertex;
    gl_Position = cameraUbo.proj * cameraUbo.view * transformedVertex;
    
    vec3 lightLocation = vec3(1000.0, 1000.0, 1000.0);
    vec3 lightVector = normalize(lightLocation - transformedVertex.xyz);
    
    fragColor = inColor * dot(normalize(transformedNormal.xyz), lightVector);
    fragTexCoord = inTexCoord;
}