#include "Material.h"

#include "Buffer.h"

Material::Material()
{

}

Material::~Material()
{

}

void Material::setShaderParameters(ShaderParameters params)
{
	shaderParams.albedo = params.albedo;
	shaderParams.ambientOcclusion = params.ambientOcclusion;
	shaderParams.roughness = params.roughness;
	shaderParams.textureIndex = params.textureIndex;
	shaderParams.toggleTexture = params.toggleTexture;
}

ShaderParameters::ShaderParameters(uint32_t texIndex, glm::vec4 albedo, float roughness, float ao, uint32_t toggleTex)
	:textureIndex(texIndex), albedo(albedo), roughness(roughness), ambientOcclusion(ao), toggleTexture(toggleTex)
{

}

ShaderParameters::ShaderParameters()
{
	albedo = glm::vec4{1.0f};
	ambientOcclusion = 1.0f;
	roughness = 1.0f;
	textureIndex = 0;
	toggleTexture = 0;
}
