#version 450
//#extension GL_KHR_vulkan_glsl:enable

layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
    mat4 mvp;
}ubo;

//layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 aTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    gl_Position = ubo.mvp * vec4(aPosition, 1.0);
    fragColor = aColor;
    fragTexCoord = aTexCoord;
}