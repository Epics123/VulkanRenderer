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
    vec3 cameraPos;
    vec3 ambientColor;
    float ambientIntensity;
    Light pointLights[1];
}lbo;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 2) in vec4 vNormal;
layout(location = 3) in vec3 vPosition;

layout(location = 0) out vec4 outColor;

void main() {
    // Temp ambient cause it being dumb
    vec4 tmpAmbient = vec4(0.005f, 0.005f, 0.01f, 0.0f);
    float tmpSpecularStrength = 14.0f;
    vec3 tmpSpecAlbedo = vec3(1.0f, 0.0f, 0.0f);

    Light curLight = lbo.pointLights[0];
    vec4 fragNormal = vec4(normalize(vNormal.xyz), 0.0f);

    vec4 lightDir = vec4(normalize(curLight.pos - vPosition), 0.0f);
    vec4 viewDir = vec4(normalize(lbo.cameraPos - vPosition), 0.0f);
    vec4 reflectDir = vec4(reflect(-lightDir, fragNormal).xyz, 0.0f);

    vec4 diffuse = max(dot(lightDir, vNormal), 0.0f) * curLight.diffuse;
    vec3 spec = pow(max(dot(reflectDir, viewDir), 0.0f), 128) * tmpSpecularStrength * tmpSpecAlbedo;

    vec4 texColor = texture(texSampler, fragTexCoord);
    //outColor = vec4(outColor.rgb * ((diffuse).rgb + spec.rgb + tmpAmbient), outColor.a);
    //outColor = vec4(outColor.rgb * (spec.rgb), outColor.a);
    vec4 ks = vec4(diffuse.rgb + spec, 0.0f);
    outColor = ks * texColor + tmpAmbient;
    //outColor.xyz = lbo.cameraPos;
    outColor.a = 1.0f;
}