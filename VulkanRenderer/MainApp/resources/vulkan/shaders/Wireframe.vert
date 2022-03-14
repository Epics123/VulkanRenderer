#version 450
layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 mvp;
}ubo;

layout(location = 0) in vec3 aPosition;

layout(location = 3) out vec4 vPosition;

void main() {
    mat4 mv = ubo.model * ubo.view;
    vPosition = mv * vec4(aPosition, 1.0f);
    gl_Position = ubo.proj * vPosition;
}