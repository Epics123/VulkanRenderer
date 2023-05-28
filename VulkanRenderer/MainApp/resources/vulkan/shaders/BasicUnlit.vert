#version 450
#extension GL_KHR_vulkan_glsl:enable

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aTexCoord;
layout (location = 4) in vec3 aTangent;
layout (location = 5) in vec3 aBitangent;

layout (location = 0) out vec3 fragColor;
layout (location = 3) out vec2 texCoord;

layout (set = 0, binding = 0) uniform GlobalUbo
{
	mat4 projection;
	mat4 view;
	mat4 invView;
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
	fragColor = aColor;
	texCoord = aTexCoord;
}