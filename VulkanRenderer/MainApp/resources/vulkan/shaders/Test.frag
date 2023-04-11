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
	vec4 direction; // w is cutoff angle
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

layout(binding = 1) uniform sampler2D texSampler;

layout (push_constant) uniform Push
{ 
	mat4 modelMatrix;
	mat4 normalMatrix;
}push;

const float SPECULAR_POWER = 512.0;

vec3 diffuse;
vec3 spec;

void calculateLighting(vec3 dirToLight, vec3 surfaceNormal, vec3 viewDirection, vec4 color)
{
		float attenuation = 1.0 / dot(dirToLight, dirToLight); // dist sq
		dirToLight = normalize(dirToLight);

		float cosAngIncidence = max(dot(surfaceNormal, dirToLight), 0);

		vec3 intensity = color.xyz * color.w * attenuation;

		diffuse += intensity * cosAngIncidence;
		
		// specular highlight
		vec3 halfAngle = normalize(dirToLight + viewDirection);
		float blinnTerm = dot(surfaceNormal, halfAngle);
		blinnTerm = clamp(blinnTerm, 0, 1);
		blinnTerm = pow(blinnTerm, SPECULAR_POWER);

		spec += intensity * blinnTerm;
}

void main()
{
	diffuse = ubo.ambientColor.xyz * ubo.ambientColor.w;
	vec3 surfaceNormal = normalize(fragNormalWorld);
	spec = vec3(0.0);

	vec3 cameraPosWorld = ubo.invView[3].xyz;
	vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld);

	for(int i = 0; i < ubo.numLights; i++)
	{
		PointLight pointLight = ubo.pointLights[i];
		vec3 dirToLight = pointLight.position.xyz - fragPosWorld;

		calculateLighting(dirToLight, surfaceNormal, viewDirection, pointLight.color);
	}

	for(int j = 0; j < ubo.numSpotLights; j++)
	{
		SpotLight spotLight = ubo.spotLights[j];
		vec3 dirToLight = spotLight.position.xyz - fragPosWorld;
		float theta = dot(dirToLight, normalize(-spotLight.direction.xyz));

		if(theta > spotLight.direction.w)
		{
			calculateLighting(dirToLight, surfaceNormal, viewDirection, spotLight.color);
		}
	}	
	

	outColor = vec4((diffuse * fragColor + spec * fragColor) * texture(texSampler, texCoord).rgb, 1.0f);
}