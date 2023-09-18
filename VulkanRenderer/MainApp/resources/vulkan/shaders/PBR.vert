#version 450
//#extension GL_KHR_vulkan_glsl:enable

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aTexCoord;
layout (location = 4) in vec3 aTangent;
layout (location = 5) in vec3 aBitangent;

layout (location = 0) out vec3 fragColor;
layout (location = 1) out vec3 fragPosWorld;
layout (location = 2) out vec3 fragNormalWorld;
layout (location = 3) out vec2 texCoord;
layout (location = 4) out vec3 fragTangent;
layout (location = 5) out vec3 fragBitangent;

layout (set = 0, binding = 0) uniform GlobalUbo
{
	mat4 projection;
	mat4 view;
	mat4 invView;
} ubo;

layout (set = 1, binding = 5) uniform MaterialUbo
{
	vec4 albedo;
	float roughness;
	float ambientOcclusion;
} matUbo;

layout (push_constant) uniform Push
{ 
	mat4 modelMatrix;
	mat4 normalMatrix;
	uint textureIndex;
	uint toggleTexture;
}push;

void main()
{
	vec4 postitionWorld = push.modelMatrix * vec4(aPosition, 1.0f);
	gl_Position = ubo.projection * ubo.view * postitionWorld;

	fragNormalWorld = normalize((push.normalMatrix * vec4(aNormal, 0.0)).xyz);
	fragPosWorld = postitionWorld.xyz;
	fragColor = aColor;
	texCoord = aTexCoord;

	fragTangent = normalize((push.normalMatrix * vec4(aTangent, 0.0)).xyz);
	fragBitangent = normalize((push.normalMatrix * vec4(aBitangent, 0.0)).xyz);
}