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

// Lighting attributes -> should be uniforms
layout(location = 4) in vec3 aLightPos;
layout(location = 5) in vec4 aDiffuse;
layout(location = 6) in float aIntensity;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

layout(location = 2) out vec4 vNormal;
layout(location = 3) out vec4 vPosition;
layout(location = 4) out vec4 vLightPos;
layout(location = 5) out vec4 vDiffuse;
layout(location = 6) out float vIntensity;
layout(location = 7) out vec4 vCameraPos;

mat4 lightModel;

void main() {
    //gl_Position = ubo.mvp * vec4(aPosition, 1.0);
    fragColor = aColor;
    fragTexCoord = aTexCoord;
    
    lightModel[0] = vec4(1.0f, 0.0f, 0.0f, 0.0f);
    lightModel[1] = vec4(0.0f, 1.0f, 0.0f, 0.0f);
    lightModel[2] = vec4(0.0f, 0.0f, 1.0f, 0.0f);
    lightModel[3] = vec4(aLightPos.xyz, 1.0f);
    //vec4 tmpLightPosition = vec4(10.0f, 0.0f, 0.0f, 1.0f);
   // lightModel = transpose(lightModel);

    mat4 mv = ubo.model * ubo.view;
    vPosition = mv * vec4(aPosition, 1.0f);
    //vPosition = ubo.mvp * vec4(aPosition, 1.0f);
    gl_Position = ubo.proj * vPosition;
    vNormal = vec4((transpose(ubo.model) * vec4(aNormal, 0.0f)).xyz, 0.0f);
    //vNormal = vec4((mv * vec4(aNormal, 0.0f)).xyz, 0.0f);
    //vNormal = mv * vec4(aNormal, 0.0f);
    //vLightPos = ubo.mvp * vec4(aLightPos, 1.0f);
    vLightPos = lightModel * ubo.view * vec4(aLightPos, 1.0f);
    //vLightPos = ubo.view * vec4(aLightPos, 1.0f);

    vCameraPos = vec4(inverse(ubo.view)[3].xyz, 1.0f);
}