#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout (location = 3) in vec2 texCoord;

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
};

layout (set = 0, binding = 0) uniform GlobalUbo
{
	mat4 projection;
	mat4 view;
	mat4 invView;
	vec4 ambientColor;
	PointLight pointLights[10];
	SpotLight spotLight[10];
	int numLights;
} ubo;

layout(binding = 1) uniform sampler2D texSampler;

layout (push_constant) uniform Push
{ 
	mat4 modelMatrix;
	mat4 normalMatrix;
}push;

const float SPECULAR_POWER = 512.0;

void main()
{
	vec3 diffuse = ubo.ambientColor.xyz * ubo.ambientColor.w;
	vec3 surfaceNormal = normalize(fragNormalWorld);
	vec3 spec = vec3(0.0);

	vec3 cameraPosWorld = ubo.invView[3].xyz;
	vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld);

	for(int i = 0; i < ubo.numLights; i++)
	{
		PointLight pointLight = ubo.pointLights[i];
		vec3 dirToLight = pointLight.position.xyz - fragPosWorld;
		float attenuation = 1.0 / dot(dirToLight, dirToLight); // dist sq
		dirToLight = normalize(dirToLight);


		float cosAngIncidence = max(dot(surfaceNormal, dirToLight), 0);

		vec3 intensity = pointLight.color.xyz * pointLight.color.w * attenuation;

		diffuse += intensity * cosAngIncidence;
		
		// specular highlight
		vec3 halfAngle = normalize(dirToLight + viewDirection);
		float blinnTerm = dot(surfaceNormal, halfAngle);
		blinnTerm = clamp(blinnTerm, 0, 1);
		blinnTerm = pow(blinnTerm, SPECULAR_POWER);

		spec += intensity * blinnTerm;
	}
	

	outColor = vec4((diffuse * fragColor + spec * fragColor) * texture(texSampler, texCoord).rgb, 1.0f);
}