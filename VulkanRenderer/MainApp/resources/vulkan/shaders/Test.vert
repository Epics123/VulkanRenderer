#version 440
//#extension GL_KHR_vulkan_glsl:enable

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec3 aColor;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = vec4(aPosition, 0.0, 1.0);
    fragColor = aColor;
}