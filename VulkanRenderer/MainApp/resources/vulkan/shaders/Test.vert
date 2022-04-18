#version 450
//#extension GL_KHR_vulkan_glsl:enable

layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 normalModel;
    mat4 view;
    mat4 proj;
    mat4 mvp;
}ubo;

//layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aNormal;

// Lighting attributes -> should be uniforms
layout(location = 4) in vec3 aLightPos;
layout(location = 5) in vec4 aDiffuse;
layout(location = 6) in float aIntensity;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

layout(location = 2) out vec4 vNormal;
layout(location = 3) out vec4 vPosition;

mat4 lightModel;

void main() {
    fragColor = aColor;
    fragTexCoord = aTexCoord;

    mat4 mv = ubo.model * ubo.view;
    vPosition = mv * vec4(aPosition, 1.0f);
    gl_Position = ubo.proj * vPosition;
    vNormal = vec4((ubo.normalModel * vec4(aNormal, 0.0f)).xyz, 0.0f);
}