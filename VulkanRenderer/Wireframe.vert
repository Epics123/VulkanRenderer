#version 450
layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 mvp;
}ubo;

layout(location = 0) in vec3 aPosition;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    mat4 mv = ubo.model * ubo.view;
    vec4 pos = mv * vec4(aPosition, 1.0f);
    gl_Position = ubo.proj * pos;
}