#version 450
layout(location = 0) out vec4 outColor;

layout(location = 3) in vec4 vPosition;

void main() {
    outColor = vec4(0.0, 0.5, 0.5, 1.0);
}