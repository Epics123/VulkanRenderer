#version 450
#extension GL_KHR_vulkan_glsl:enable

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aTexCoord;

layout (location = 0) out vec3 fragColor;

layout (set = 0, binding = 0) uniform GlobalUbo
{
	mat4 projectionView;
	vec3 directionToLight;
} ubo;

layout (push_constant) uniform Push
{ 
	mat4 modelMatrix;
	mat4 normalMatrix;
}push;

const vec3 DIRECTION_TO_LIGHT = normalize(vec3(1.0, -3.0, 1.0));
const float AMBIENT = 0.02;

void main()
{
	gl_Position = ubo.projectionView * push.modelMatrix * vec4(aPosition, 1.0f);

	vec3 normalWorld = normalize((push.normalMatrix * vec4(aNormal, 0.0)).xyz);

	float lightIntensity = AMBIENT + max(dot(normalWorld, ubo.directionToLight), 0);

	fragColor = lightIntensity * aColor;
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