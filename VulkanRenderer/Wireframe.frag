#version 450
layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

void main() {
    outColor = vec4(0.0, 0.8, 0.5, 1.0);
}