#version 450

layout (location = 0) in vec2 fragOffset;
layout (location = 1) flat in int lightIndex;

layout (location = 0) out vec4 outColor;

struct PointLight
{
	vec4 position;
	vec4 color; // w is intensity
	float radius;
};

layout (set = 0, binding = 0) uniform GlobalUbo
{
	mat4 projection;
	mat4 view;
	vec4 ambientColor;
	PointLight pointLights[10];
	int numLights;
} ubo;

void main()
{
	float dist = sqrt(dot(fragOffset, fragOffset));
	if(dist >= 1.0)
		discard;
	outColor = vec4(ubo.pointLights[lightIndex].color.xyz, 1.0);
}