#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

layout(location = 2) in vec4 vNormal;
layout(location = 3) in vec4 vPosition;
layout(location = 4) in vec4 vLightPos;
layout(location = 5) in vec4 vDiffuse;
layout(location = 6) in float vIntensity;
layout(location = 7) in vec4 vCameraPos;

void main() {
    // Visualize texcoords
    //outColor = vec4(fragTexCoord, 0.0, 1.0);
    //outColor = vec4(fragColor, 1.0);

   // vec4 tmpLightPosition = vec4(10.0f, 0.0f, 0.0f, 1.0f);

    //vec4 lookDir = vec4(normalize(vLightPos.xyz - vPosition.xyz), 0.0f);
    vec4 lookDir = vec4(normalize(vCameraPos.xyz - vPosition.xyz), 0.0f);
    //vec4 lookDir = normalize(vLightPos - vPosition);
    vec4 normal = vec4(normalize(vNormal.xyz), 0.0f);
    vec4 diffAlbedo = vec4(0.9f, 0.8f, 0.7f, 0.0f);
    vec4 specAlbedo = vec4(0.7f, 0.8f, 0.9f, 0.0f);
    //vec4 diffCoef = max(dot(normal, lookDir), 0.0f) * diffAlbedo;
    vec4 ambient = vec4(0.05f, 0.05f, 0.06f, 0.0f);
    vec4 view = vec4(normalize(-vPosition.xyz), 0.0f);
    vec4 reflection = vec4(normalize(reflect(-lookDir.xyz, normal.xyz)), 0.0f);

    vec4 diffCoef = max(dot(normal, lookDir), 0.0f) * diffAlbedo;// * vIntensity;
    vec4 specCoef = pow(max(dot(reflection, view), 0.0f), 40) * specAlbedo;

    outColor = texture(texSampler, fragTexCoord);
    //outColor = vec4(normal.xyz, 1.0f);
    outColor.rgb *= (diffCoef + specCoef);// + ambient;
    //outColor *= (specCoef);// + ambient;


}