#version 450
#extension GL_KHR_vulkan_glsl:enable

const vec2 OFFSETS[6] = vec2[](
  vec2(-1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, -1.0),
  vec2(1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, 1.0)
);

layout (location = 0) out vec2 fragOffset;
layout (location = 1) flat out int lightIndex;

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
	fragOffset = OFFSETS[gl_VertexIndex];
	lightIndex = int(floor(gl_VertexIndex / 6));

	vec3 cameraRightWorld = {ubo.view[0][0], ubo.view[1][0], ubo.view[2][0]};
	vec3 cameraUpWorld = {ubo.view[0][1], ubo.view[1][1], ubo.view[2][1]};

	PointLight light = ubo.pointLights[lightIndex];

	vec3 positionWorld = light.position.xyz + light.radius * fragOffset.x * cameraRightWorld + light.radius * fragOffset.y * cameraUpWorld;

	gl_Position = ubo.projection * ubo.view * vec4(positionWorld, 1.0f);
}