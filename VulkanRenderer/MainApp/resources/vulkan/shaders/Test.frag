#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

layout(location = 2) in vec4 vNormal;
layout(location = 3) in vec4 vPosition;

void main() {
    // Visualize texcoords
    //outColor = vec4(fragTexCoord, 0.0, 1.0);
    //outColor = vec4(fragColor, 1.0);

    vec4 tmpLightPosition = vec4(2.0f, 3.0f, 4.0f, 1.0f);

    vec4 lookDir = normalize(tmpLightPosition - vPosition);
    vec4 normal = normalize(vNormal);
    vec4 diffAlbedo = vec4(0.9f, 0.8f, 0.7f, 1.0f);
    vec4 diffCoef = max(dot(normal, lookDir), 0.0f) * diffAlbedo;
    vec4 ambient = vec4(0.0f);

    outColor = texture(texSampler, fragTexCoord);
    outColor *= diffCoef + ambient;
}