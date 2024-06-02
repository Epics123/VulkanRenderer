#version 450

layout (location = 0) in vec3 aPosition;

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
	vec4 ambientColor;
	PointLight pointLights[10];
	SpotLight spotLights[10];
	int numLights;
	int numSpotLights;
} ubo;

layout (push_constant) uniform Push
{ 
	mat4 modelMatrix;
	mat4 normalMatrix;
}push;

void main()
{
	vec4 postitionWorld = push.modelMatrix * vec4(aPosition, 1.0f);
	gl_Position = ubo.projection * ubo.view * postitionWorld;
}