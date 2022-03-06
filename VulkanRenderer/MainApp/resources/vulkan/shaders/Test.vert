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
layout(location = 3) in vec3 aNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

layout(location = 2) out vec4 vNormal;
layout(location = 3) out vec4 vPosition;
layout(location = 4) out vec4 vLightPos;

void main() {
    gl_Position = ubo.mvp * vec4(aPosition, 1.0);
    fragColor = aColor;
    fragTexCoord = aTexCoord;

    vec4 tmpLightPosition = vec4(10.0f, 0.0f, 0.0f, 1.0f);

    mat4 mv = ubo.view * ubo.model;
    vPosition = mv * vec4(aPosition, 1.0f);
    vNormal = mv * vec4(aNormal, 0.0f);
    vLightPos = mv * tmpLightPosition;
}