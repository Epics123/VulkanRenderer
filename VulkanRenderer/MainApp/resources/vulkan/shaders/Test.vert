#version 450
#extension GL_KHR_vulkan_glsl:enable

layout (location = 0) in vec2 aPosition;
layout (location = 1) in vec3 aColor;

//layout (location = 0) out vec3 fragColor;

layout (push_constant) uniform Push
{ 
	mat2 transform;
	vec2 offset;
	vec3 color;
}push;

void main()
{
	gl_Position = vec4(push.transform * aPosition + push.offset, 0.0, 1.0);
	//fragColor = aColor;
}


//layout(binding = 0) uniform UniformBufferObject
//{
//    mat4 model;
//    mat4 normalModel;
//    mat4 view;
//    mat4 proj;
//    mat4 mvp;
//    mat4 mv;
//}ubo;
//
////layout(binding = 1) uniform sampler2D texSampler;
//
//layout(location = 0) in vec3 aPosition;
//layout(location = 1) in vec3 aColor;
//layout(location = 2) in vec2 aTexCoord;
//layout(location = 3) in vec3 aNormal;
//
//layout(location = 0) out vec3 fragColor;
//layout(location = 1) out vec2 fragTexCoord;
//
//layout(location = 2) out vec4 vNormal;
//layout(location = 3) out vec3 vPosition;
//
//void main() {
//    fragColor = aColor;
//    fragTexCoord = aTexCoord;
//
//    vPosition = vec3( ubo.model * vec4(aPosition, 1.0f));
//    gl_Position = ubo.mvp * vec4(aPosition, 1.0f);
//    vNormal = vec4((ubo.normalModel * vec4(aNormal, 0.0f)).xyz, 0.0f);
//}