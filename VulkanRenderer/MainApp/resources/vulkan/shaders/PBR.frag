#version 450
#extension GL_KHR_vulkan_glsl:enable

//PBR lighting from https://learnopengl.com/PBR/Lighting and https://www.youtube.com/watch?v=RRE-F57fbXw

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

//TODO: Add metalic map
layout(set = 1, binding = 0) uniform sampler2D diffuseMap;
layout(set = 1, binding = 1) uniform sampler2D normalMap;
layout(set = 1, binding = 2) uniform sampler2D roughnessMap;
layout(set = 1, binding = 3) uniform sampler2D aoMap;
layout(set = 1, binding = 4) uniform sampler2D heightMap;

layout (push_constant) uniform Push
{ 
	mat4 modelMatrix;
	mat4 normalMatrix;
}push;

const float SPECULAR_POWER = 512.0;
const float PARALAX_HEIGHT_SCALE = 0.05;
const float PI = 3.1415926538;

vec3 diffuse = vec3(0.0);
vec3 spec;

vec3 albedo;
float roughness;

// GGX/Trowbridge-Reitz normal distribution
float normalDisribution(float alpha, vec3 nrm, vec3 halfway)
{
	float numerator = pow(alpha, 2.0);

	float NdotH = max(dot(nrm, halfway), 0.0);
	float denominator = PI * pow(pow(NdotH, 2.0) * (pow(alpha, 2.0) - 1.0) + 1.0, 2.0);
	denominator = max(denominator, 0.000001);

	return numerator / denominator;
}

// Schlick-Beckmann Geometry Shadowing function
float G1(float alpha, vec3 nrm, vec3 x)
{
	float NdotX = max(dot(nrm, x), 0.0);
	float numerator = NdotX;

	float k = alpha * 0.5;
	float denominator = NdotX * (1.0 - k) + k;
	denominator = max(denominator, 0.000001);

	return numerator / denominator;
}

float G(float alpha, vec3 nrm, vec3 view, vec3 light)
{
	return G1(alpha, nrm, view) * G1(alpha, nrm, light);
}

vec3 fresnel(vec3 f0, vec3 view, vec3 halfway, float fresnelPow)
{
	return f0 + (vec3(1.0) - f0) * pow(1 - max(dot(view, halfway), 0.0), fresnelPow);
}

// pbr lighting calculation
vec3 calculateLighting(vec3 V, vec3 N, vec3 L, vec3 H, vec3 albedo, vec4 lightColor)
{
	vec3 F0 = vec3(0.04); // can vary per material, but leaving as a constant value for now
	float LdotN = max(dot(L, N), 0.0);

	float attenuation = 1.0 / dot(L, L); // dist sq
	vec3 radiance = lightColor.xyz * attenuation;

	vec3 Ks = fresnel(F0, V, H, 5.0);
	vec3 Kd = vec3(1.0) - Ks;

	vec3 lambert = albedo / PI;

	vec3 cookTorranceNumerator = normalDisribution(roughness, N, H) * G(roughness, N, V, L) * Ks;
	float cookTorrenceDenominator = 4.0 * max(dot(V, N), 0.0) * LdotN;
	vec3 cookTorrence = cookTorranceNumerator / cookTorrenceDenominator;

	vec3 BRDF = Kd * lambert + cookTorrence;
	vec3 outgoingLight = radiance + BRDF * lightColor.xyz * LdotN;

	return outgoingLight;
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
	vec2 uv = texCoord;

	vec3 cameraPosWorld = ubo.invView[3].xyz;

	vec3 N = calculateNormal(uv);
	vec3 V = normalize(cameraPosWorld - fragPosWorld); // view vector
	vec3 L = vec3(0.0); // light vector
	vec3 H = vec3(0.0); // halfway vector

	albedo = texture(diffuseMap, uv).rgb;
	roughness = texture(roughnessMap, uv).r;
	float ao = texture(aoMap, uv).r;

	vec3 Lo = vec3(0.0);
	for(int i = 0; i < ubo.numLights; i++)
	{
		PointLight pointLight = ubo.pointLights[i];
		L = normalize(pointLight.position.xyz - fragPosWorld);
		H = normalize(V + L);
		Lo += calculateLighting(V, N, L, H, albedo, pointLight.color);
	}

	//vec3 ambient = vec3(0.03) * albedo * ao;
	//vec3 color = ambient + diffuse;

	//outColor = vec4(color, 1.0);
	outColor = vec4(Lo, 1.0);
}