#pragma once

#include "Texture.h"

#include "glm/glm.hpp"

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <map>

// Ubo for a given material
struct ShaderParameters
{
	uint32_t textureIndex;
	glm::vec4 albedo;
	float roughness;
	float ambientOcclusion;
	float metallic;
	uint32_t toggleTexture;

	std::map<uint32_t, Texture> materialTextures;

	ShaderParameters();
	ShaderParameters(uint32_t texIndex, uint32_t toggleTex = 0, glm::vec4 albedo = glm::vec4{1.0f}, float roughness = 1.0f, float ao = 1.0f, float metallic = 0.0f);
};

class Material
{
public:
	Material();
	Material(ShaderParameters& params);
	~Material();

	ShaderParameters& getShaderParameters() { return shaderParams; }
	void setShaderParameters(ShaderParameters params);

	void cleanup(class Device& device);

private:
	ShaderParameters shaderParams;
};

class MaterialBuilder
{
public:
	Material buildMaterial(std::string filepath, class Device& device);

private:
	int getBindingFromFileName(const std::string& filename);

	const std::string albedoExtension = "_albedo";
	const std::string normalExtension = "_normal";
	const std::string roughnessExtension = "_roughness";
	const std::string aoExtension = "_ao";
	const std::string heightExtension = "_height";
	const std::string metallicExtension = "_metallic";
};