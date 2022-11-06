#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout (location = 3) flat in int lightIndex;

layout (location = 0) out vec4 outColor;

struct PointLight
{
	vec4 position;
	vec4 color; // w is intensity
	float radius;
};

layout (set = 0, binding = 0) uniform GlobalUbo
{
	mat4 projection;
	mat4 view;
	vec4 ambientColor;
	PointLight pointLights[10];
	int numLights;
} ubo;

layout (push_constant) uniform Push
{ 
	mat4 modelMatrix;
	mat4 normalMatrix;
}push;

void main()
{
	vec3 diffuse = ubo.ambientColor.xyz * ubo.ambientColor.w;
	vec3 surfaceNormal = normalize(fragNormalWorld);

	vec4 color;

	for(int i = 0; i < ubo.numLights; i++)
	{
		PointLight pointLight = ubo.pointLights[i];
		vec3 dirToLight = pointLight.position.xyz - fragPosWorld;
		float attenuation = 1.0 / dot(dirToLight, dirToLight); // dist sq
		float cosAngIncidence = max(dot(surfaceNormal, normalize(dirToLight)), 0);

		vec3 intensity = pointLight.color.xyz * pointLight.color.w * attenuation;

		diffuse += intensity * cosAngIncidence;
		//color = vec4(cosAngIncidence, 0.0, 0.0, 1.0);
	}
	

	color = vec4(diffuse * fragColor, 1.0f);
	//color = vec4(surfaceNormal, 1.0);

	//vec4 color = vec4(diffuse * fragColor, 1.0);
	//color = vec4(diffuse * fragColor, 1.0);

	outColor = color;
	//outColor = vec4(diffuse, 1.0);

//	vec3 dirToLight = (ubo.lightPosition).xyz - fragPosWorld;
//	float attenuation = 1.0 / dot(dirToLight, dirToLight); // dist sq
//
//	vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w * attenuation;
//	vec3 ambientColor = ubo.ambientColor.xyz * ubo.ambientColor.w;
//
//	vec3 diffuse = lightColor * max(dot(normalize(fragNormalWorld), normalize(dirToLight)), 0);
//
//	vec4 color = vec4((diffuse + ambientColor) * fragColor, 1.0);
//
//	outColor = color;
}





//struct Light {
//mat4 model;
//vec4 diffuse;
//vec3 pos;
//float intensity;
//};
//
//layout(binding = 1) uniform sampler2D texSampler;
//
//layout(binding = 2) uniform LightBuffer
//{
//    mat4 model;
//    vec4 cameraPos;
//    vec4 lightColor;
//    vec4 lightPos;
//    float ambientIntensity;
//    //Light pointLights[1];
//    //Light pointLights;
//}lbo;
//
//layout(location = 0) in vec3 fragColor;
//layout(location = 1) in vec2 fragTexCoord;
//
//layout(location = 2) in vec4 vNormal;
//layout(location = 3) in vec3 vPosition;
//
//layout(location = 0) out vec4 outColor;
//
//void main() {
//    //Light curLight = lbo.pointLights[0];
//    //Light curLight = lbo.pointLights;
//    vec4 fragNormal = vec4(normalize(vNormal.xyz), 0.0f);
//
//    vec4 ambient = vec4(lbo.ambientIntensity * lbo.lightColor.xyz, 1.0f);
//
//    //vec4 lightDir = vec4(normalize(curLight.pos - vPosition), 0.0f);
//    vec4 lightDir = vec4(normalize(lbo.lightPos.xyz - vPosition), 0.0f);
//    vec4 viewDir = vec4(normalize(lbo.cameraPos.xyz - vPosition), 0.0f);
//    vec4 reflectDir = vec4(reflect(-lightDir, fragNormal).xyz, 0.0f);
//
//    //vec4 diffuse = curLight.diffuse * max(dot(normalize(lightDir), vNormal), 0.0f);
//    vec4 diffuse = lbo.lightColor * max(dot(lightDir, vNormal), 0.0f);
//    //vec3 spec = pow(max(dot(reflectDir, viewDir), 0.0f), 128) * tmpSpecularStrength * tmpSpecAlbedo;
//    vec3 spec = pow(max(dot(reflectDir, viewDir), 0.0f), 32) * 0.5 * lbo.lightColor.xyz;
//
//    vec4 texColor = texture(texSampler, fragTexCoord);
//    //outColor = vec4(outColor.rgb * ((diffuse).rgb + spec.rgb + tmpAmbient), outColor.a);
//    //outColor = vec4(outColor.rgb * (spec.rgb), outColor.a);
//    //vec4 ks = vec4(diffuse.rgb + spec, 0.0f);
//    //outColor = ks * texColor + tmpAmbient;
//    //outColor = ks * texColor + ambient;
//
//    outColor.xyz = (ambient.xyz + diffuse.xyz + spec) * texColor.xyz;
//    outColor.a = 1.0f;
//}