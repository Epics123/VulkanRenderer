#version 450

struct Light {
vec3 pos;
vec4 diffuse;
float intensity;
};

layout(binding = 1) uniform sampler2D texSampler;

layout(binding = 2) uniform LightBuffer
{
    mat4 model;
    Light pointLights[1];
    vec3 cameraPos;
    vec3 ambientColor;
    float ambientIntensity;
}lbo;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 2) in vec4 vNormal;
layout(location = 3) in vec3 vPosition;

layout(location = 0) out vec4 outColor;

void main() {
    // Temp ambient cause it being dumb
    vec3 tmpAmbient = vec3(0.1f, 0.1f, 0.2f);

    Light curLight = lbo.pointLights[0];

    vec4 lightDir = vec4(normalize(curLight.pos - vPosition), 0.0f);
    vec4 fragNormal = vec4(normalize(vNormal.xyz), 0.0f);
    vec4 diffuse = max(dot(lightDir, vNormal), 0.0f) * curLight.diffuse;

    outColor = texture(texSampler, fragTexCoord);
    outColor = vec4(outColor.rgb * ((diffuse).rgb + tmpAmbient), outColor.a);
    //outColor += vec4(lbo.ambientColor*lbo.ambientIntensity, 0.0f);
//    outColor += vec4(vec3(0.05f, 0.05f, 0.06f), 0.0f);
//outColor = vec4(lbo.ambientColor, 1.0f);
}