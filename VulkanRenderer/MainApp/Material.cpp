#include "Material.h"
#include "Log.h"
#include "Utils.h"
#include "Device.h"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <string>

const std::string MaterialBuilder::materialPath = "MainApp/resources/vulkan/materials/";

Material::Material()
{

}

Material::Material(ShaderParameters& params)
	:shaderParams(params)
{
	
}

Material::~Material()
{
	
}

void Material::setShaderParameters(ShaderParameters& params)
{
	shaderParams.albedo = params.albedo;
	shaderParams.ambientOcclusion = params.ambientOcclusion;
	shaderParams.roughness = params.roughness;
	shaderParams.metallic = params.metallic;
	shaderParams.textureIndex = params.textureIndex;
	shaderParams.toggleTexture = params.toggleTexture;
	shaderParams.textureDir = params.textureDir;
	shaderParams.materialTextures = params.materialTextures;
}

void Material::cleanup(class Device& device)
{
	if (shaderParams.materialTextures.size() > 0)
	{
		for (std::pair<uint32_t, Texture> tex : shaderParams.materialTextures)
		{
			tex.second.cleanup(device);
		}
	}
}

ShaderParameters::ShaderParameters(uint32_t texIndex, uint32_t toggleTex, glm::vec4 albedo, float roughness, float ao, float metallic)
	:textureIndex(texIndex), albedo(albedo), roughness(roughness), ambientOcclusion(ao), metallic(metallic), toggleTexture(toggleTex)
{

}

ShaderParameters::ShaderParameters()
{
	albedo = glm::vec4{1.0f};
	ambientOcclusion = 1.0f;
	roughness = 1.0f;
	metallic = 0.0f;
	textureIndex = 0;
	toggleTexture = 0;
}

int MaterialBuilder::getBindingFromFileName(const std::string& filename)
{
	int binding = -1;
	size_t notFound = std::string::npos;
	if(filename.find(albedoExtension) != notFound)
	{
		binding = 0;
	}
	else if(filename.find(normalExtension) != notFound)
	{
		binding = 1;
	}
	else if(filename.find(roughnessExtension) != notFound)
	{
		binding = 2;
	}
	else if (filename.find(aoExtension) != notFound)
	{
		binding = 3;
	}
	else if (filename.find(heightExtension) != notFound)
	{
		binding = 4;
	}
	else if (filename.find(metallicExtension) != notFound)
	{
		binding = 5;
	}

	return binding;
}
