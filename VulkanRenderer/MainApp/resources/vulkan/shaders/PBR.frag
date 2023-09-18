#version 450
#extension GL_KHR_vulkan_glsl:enable
#extension GL_EXT_nonuniform_qualifier:enable

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

struct DirectionalLight
{
	vec4 position;
	vec4 color;
	vec4 direction;
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
} ubo;

layout (set = 0, binding = 1) uniform LightUbo
{
	vec4 ambientColor;
	PointLight pointLights[10];
	SpotLight spotLights[10];
	DirectionalLight directionalLight;
	int numLights;
	int numSpotLights;
} lightUbo;

//TODO: Add metalic map
layout(set = 1, binding = 0) uniform sampler2D diffuseMap[];
layout(set = 1, binding = 1) uniform sampler2D normalMap[];
layout(set = 1, binding = 2) uniform sampler2D roughnessMap[];
layout(set = 1, binding = 3) uniform sampler2D aoMap[];
layout(set = 1, binding = 4) uniform sampler2D heightMap[];
layout(set = 1, binding = 5) uniform sampler2D metallicMap[];

layout (set = 1, binding = 6) uniform MaterialUbo
{
	vec4 albedo;
	float roughness;
	float ambientOcclusion;
	float metallic;
} matUbo;

layout (push_constant) uniform Push
{ 
	mat4 modelMatrix;
	mat4 normalMatrix;
	uint textureIndex;
	uint toggleTexture;
}push;

const float SPECULAR_POWER = 512.0;
const float PARALAX_HEIGHT_SCALE = 0.05;
const float PI = 3.1415926538;

vec3 diffuse = vec3(0.0);
vec3 spec;

vec3 albedo;
float roughness;
float ao;
float attenuation;
float metallic = 0.0;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
	
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	denom = max(denom, 0.000001);
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	denom = max(denom, 0.000001);
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 fresnel(float cosTheta, vec3 f0, float fresnelPow)
{
	return f0 + (1.0 - f0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), fresnelPow);
}

// pbr lighting calculation
vec3 calculateLighting(vec3 V, vec3 N, vec3 L, vec3 H, vec3 albedo, vec4 lightColor)
{
	vec3 F0 = vec3(0.04); // can vary per material, but leaving as a constant value for now
	F0 = mix(F0, albedo, metallic);
	float NdotL = max(dot(N, L), 0.0);
	float intensity = lightColor.w;

	vec3 radiance = lightColor.xyz * attenuation * intensity;

	float NDF = DistributionGGX(N, H, roughness);
	float G = GeometrySmith(N, V, L, roughness);

	vec3 Ks = fresnel(max(dot(H, V), 0.0), F0, 5.0);
	vec3 Kd = (vec3(1.0) - Ks);

	vec3 numerator = NDF * G * Ks;
	float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.0001;
	vec3 specular = numerator / denominator;

	vec3 outgoingLight = (Kd * albedo / PI + specular) * radiance * NdotL;

	return outgoingLight;
}

// calculate normals from normal map
vec3 calculateNormal(vec2 uv)
{
	vec3 result;

	// get TBN basis vectors
	vec3 normal = fragNormalWorld;
	vec3 tangent = fragTangent;
	vec3 bitangent = fragBitangent;

	// sample normal map and bring range to [-1.0, 1.0]
	vec3 normalMapNormal = 2.0 * texture(normalMap[push.textureIndex], uv).xyz - 1.0;

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

	float currentDepthMapValue = 1.0 - texture(heightMap[push.textureIndex], uv).r;

	vec2 UVs = uv;

	// loop until the point on the height map is "hit"
	while(currentLayerDepth < currentDepthMapValue)
	{
		UVs -= deltaUV;
		currentDepthMapValue = 1.0 - texture(heightMap[push.textureIndex], UVs).r;
		currentLayerDepth += layerDepth;
	}

	// Apply occlusion (interpolate w/ prev uvs)
	vec2 prevUVs = UVs + deltaUV;
	float afterDepth = currentDepthMapValue - currentLayerDepth;
	float beforeDepth = 1.0 - texture(heightMap[push.textureIndex], prevUVs).r - currentLayerDepth + layerDepth;
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

	vec3 cameraPosWorld = inverse(ubo.view)[3].xyz;

	vec3 N;
	vec3 V = normalize(cameraPosWorld - fragPosWorld); // view vector
	vec3 L = vec3(0.0); // light vector
	vec3 H = vec3(0.0); // halfway vector

	if(push.toggleTexture == 1)
	{
		albedo = pow(texture(diffuseMap[push.textureIndex], uv).rgb, vec3(2.2));
		roughness = texture(roughnessMap[push.textureIndex], uv).r;
		ao = texture(aoMap[push.textureIndex], uv).r;
		metallic = texture(metallicMap[push.textureIndex], uv).r;

		N = calculateNormal(uv);
	}
	else
	{
		albedo = matUbo.albedo.xyz;
		roughness = matUbo.roughness;
		ao = matUbo.ambientOcclusion;
		metallic = matUbo.metallic;

		N = fragNormalWorld;
	}

	vec3 Lo = vec3(0.0);
	vec3 ambient = vec3(0.01) * albedo * ao;

	// directional light
	L = normalize(-lightUbo.directionalLight.direction.xyz);
	H = normalize(V + L);
	attenuation = 1.0;
	Lo += calculateLighting(V, N, L, H, albedo, lightUbo.directionalLight.color);

	// point lights
	for(int i = 0; i < lightUbo.numLights; i++)
	{
		PointLight pointLight = lightUbo.pointLights[i];
		L = pointLight.position.xyz - fragPosWorld;
		attenuation = 1.0 / dot(L, L); // dist sq
		L = normalize(L);
		H = normalize(V + L);
		Lo += calculateLighting(V, N, L, H, albedo, pointLight.color);
	}

	// spot lights
	for(int j = 0; j < lightUbo.numSpotLights; j++)
	{
		SpotLight spotLight = lightUbo.spotLights[j];
		L = spotLight.position.xyz - fragPosWorld;
		float theta = dot(normalize(L), normalize(-spotLight.direction.xyz));

		if(theta > spotLight.direction.w)
		{
			float epsilon = spotLight.direction.w - spotLight.outerCutoff;
			float spotFadeIntensity = smoothstep(0.0, 1.0, (theta - spotLight.outerCutoff) / -epsilon); // having a negative epislon value works for some reason, need to swap outer and inner cutoff values on the CPU side
			
			attenuation = spotFadeIntensity;
			L = normalize(L);
			H = normalize(V + L);

			Lo += calculateLighting(V, N, L, H, albedo, spotLight.color);
		}
		else
			Lo += ambient;
	}
	
	vec3 color = ambient + Lo;

	outColor = vec4(color, 1.0);
}