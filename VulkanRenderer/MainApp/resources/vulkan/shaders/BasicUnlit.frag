#version 450
#extension GL_KHR_vulkan_glsl:enable
#extension GL_EXT_nonuniform_qualifier:enable

layout (location = 0) in vec3 fragColor;
layout (location = 3) in vec2 texCoord;

layout (location = 0) out vec4 outColor;

layout (push_constant) uniform Push
{ 
	mat4 modelMatrix;
	mat4 normalMatrix;
	uint textureIndex;
}push;

layout(set = 1, binding = 0) uniform sampler2D texSampler[];

void main()
{
	outColor = vec4(fragColor * texture(texSampler[push.textureIndex], texCoord).rgb, 1.0);
}