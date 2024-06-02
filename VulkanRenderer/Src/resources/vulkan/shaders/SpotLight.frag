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

struct SpotLight
{
	vec4 position;
	vec4 color; // w is intensity
	vec4 direction;
	float outerCutoff;
};

layout (set = 0, binding = 0) uniform GlobalUbo
{
	mat4 projection;
	mat4 view;
	mat4 invView;
} ubo;

layout (set = 0, binding = 1) uniform LightUbo
{
	vec4 ambientColor;
	PointLight pointLights[10];
	SpotLight spotLights[10];
	int numLights;
	int numSpotLights;
} lightUbo;

const float PI = 3.14159265;

void main()
{
	float dist = sqrt(dot(fragOffset, fragOffset));
	if(dist >= 1.0)
		discard;
	
	outColor = vec4(lightUbo.spotLights[lightIndex].color.xyz, 1.0);//0.5 * (cos(dist * PI) + 1.0));
}