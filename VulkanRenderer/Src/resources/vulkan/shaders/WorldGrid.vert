#version 450
#extension GL_KHR_vulkan_glsl:enable

// Grid shader code from http://asliceofrendering.com/scene%20helper/2020/01/05/InfiniteGrid/

layout(location = 1) out vec3 nearPoint;
layout(location = 2) out vec3 farPoint;

layout (set = 0, binding = 0) uniform GlobalUbo
{
	mat4 projection;
	mat4 view;
	mat4 invView;
} ubo;

vec3 gridPlane[6] = vec3[](
    vec3(1, 1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3(-1, -1, 0), vec3(1, 1, 0), vec3(1, -1, 0)
);

vec3 unprojectPoint(float x, float y, float z, mat4 invView, mat4 invProj)
{
	vec4 unprojectPoint = invView * invProj * vec4(x, y, z, 1.0);
	return unprojectPoint.xyz / unprojectPoint.w;
}

void main()
{
	mat4 invProj = inverse(ubo.projection);
	mat4 invView = inverse(ubo.view);

	vec3 pos = gridPlane[gl_VertexIndex].xyz;
	nearPoint = unprojectPoint(pos.x, pos.y, 0.0, invView, invProj).xyz; // unprojecting on the near plane
    farPoint = unprojectPoint(pos.x, pos.y, 1.0, invView, invProj).xyz; // unprojecting on the far plane

	gl_Position = vec4(pos, 1.0);
}