#version 450
#extension GL_KHR_vulkan_glsl:enable

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout (location = 3) in vec2 texCoord;
layout (location = 4) in vec3 fragTangent;
layout (location = 5) in vec3 fragBitangent;

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

layout(set = 1, binding = 0) uniform sampler2D diffuseMap;
layout(set = 1, binding = 1) uniform sampler2D normalMap;
layout(set = 1, binding = 4) uniform sampler2D heightMap;

layout (push_constant) uniform Push
{ 
	mat4 modelMatrix;
	mat4 normalMatrix;
}push;

const float SPECULAR_POWER = 512.0;
const float PARALAX_HEIGHT_SCALE = 0.05;

vec3 diffuse;
vec3 spec;

// blinn-phong lighting calculation
void calculateBlinnPongLighting(vec3 dirToLight, vec3 surfaceNormal, vec3 viewDirection, vec4 color)
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

// calculate normals from normal map
vec3 calculateNormal(vec2 uv)
{
	vec3 result;

	// get TBN basis vectors
	vec3 normal = normalize(fragNormalWorld);
	vec3 tangent = normalize(fragTangent);
	vec3 bitangent = normalize(fragBitangent);

	// sample normal map and bring range to [-1.0, 1.0]
	vec3 normalMapNormal = normalize(texture(normalMap, uv).xyz * 2.0 - 1.0);

	// construct TBN matrix
	mat3 TBN = mat3(tangent, bitangent, normal);

	result = TBN * normalMapNormal;
	result = normalize(result);

	return result;
}

vec2 calculateParalaxTexCoords(vec2 uv, vec3 viewDir)
{
	float heightScale = PARALAX_HEIGHT_SCALE;
	const float minLayers = 8.0;
	const float maxLayers = 64.0;

	// set number of layers dynamically based on view direction
	float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));

	float layerDepth = 1.0 / numLayers;
	float currentLayerDepth = 0.0;

	vec2 S = viewDir.xy * heightScale;
	vec2 deltaUV = S / numLayers;

	float currentDepthMapValue = 1.0 - texture(heightMap, uv).r;

	vec2 UVs = uv;

	// loop until the point on the height map is "hit"
	while(currentLayerDepth < currentDepthMapValue)
	{
		UVs -= deltaUV;
		currentDepthMapValue = 1.0 - texture(heightMap, UVs).r;
		currentLayerDepth += layerDepth;
	}

	// Apply occlusion (interpolate w/ prev uvs)
	vec2 prevUVs = UVs + deltaUV;
	float afterDepth = currentDepthMapValue - currentLayerDepth;
	float beforeDepth = 1.0 - texture(heightMap, prevUVs).r - currentLayerDepth + layerDepth;
	float weight = afterDepth / (afterDepth - beforeDepth);
	UVs = prevUVs * weight + UVs * (1.0 - weight);

	// Discard anything outside of the normal range
//	if(UVs.x > 1.0 || UVs.y > 1.0 || UVs.x < 0.0 || UVs.y < 0.0)
//		discard;
	
	return UVs;
}

void main()
{
	diffuse = ubo.ambientColor.xyz * ubo.ambientColor.w;
	spec = vec3(0.0);

	vec3 cameraPosWorld = ubo.invView[3].xyz;
	vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld);

	vec2 uv = calculateParalaxTexCoords(texCoord, viewDirection);
	vec3 surfaceNormal = calculateNormal(uv);

	for(int i = 0; i < ubo.numLights; i++)
	{
		PointLight pointLight = ubo.pointLights[i];
		vec3 dirToLight = pointLight.position.xyz - fragPosWorld;

		calculateBlinnPongLighting(dirToLight, surfaceNormal, viewDirection, pointLight.color);
	}

	float f;
	for(int j = 0; j < ubo.numSpotLights; j++)
	{
		SpotLight spotLight = ubo.spotLights[j];
		vec3 dirToLight = spotLight.position.xyz - fragPosWorld;
		float theta = dot(normalize(dirToLight), normalize(-spotLight.direction.xyz));

		if(theta > spotLight.direction.w)
		{
			float epsilon = spotLight.direction.w - spotLight.outerCutoff;
			float spotFadeIntensity = clamp((theta - spotLight.outerCutoff) / epsilon, 0.0, 1.0);
			calculateBlinnPongLighting(dirToLight, surfaceNormal, viewDirection, spotLight.color);
			diffuse *= spotFadeIntensity;
			spec *= spotFadeIntensity;
		}
	}

	outColor = vec4((diffuse * fragColor + spec * fragColor) * texture(diffuseMap, uv).rgb, 1.0f);
}