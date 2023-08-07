#pragma once

#include "glm/glm.hpp"

#include <memory>
#include <vector>
#include <string>

// Ubo for a given material
struct ShaderParameters
{
	uint32_t textureIndex;
	glm::vec4 albedo;
	float roughness;
	float ambientOcclusion;
	float metallic;
	uint32_t toggleTexture;

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

private:
	ShaderParameters shaderParams;
};

class MaterialBuilder
{
public:
	Material& buildMaterial(std::string filepath);
};