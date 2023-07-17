#pragma once
#include "glm/glm.hpp"

// Ubo for a given material
struct ShaderParameters
{
	uint32_t textureIndex;
	glm::vec4 albedo;
	float roughness;
	float ambientOcclusion;
	uint32_t toggleTexture;

	ShaderParameters();
	ShaderParameters(uint32_t texIndex, glm::vec4 albedo = glm::vec4{1.0f}, float roughness = 1.0f, float ao = 1.0f, uint32_t toggleTex = 0);
};

class Material
{
public:
	Material();
	~Material();

	ShaderParameters& getShaderParameters() { return shaderParams; }
	void setShaderParameters(ShaderParameters params);

private:
	ShaderParameters shaderParams;

};