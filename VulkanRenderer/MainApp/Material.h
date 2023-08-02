#pragma once

#include "glm/glm.hpp"
#include <memory>

// Ubo for a given material
struct ShaderParameters
{
	uint32_t textureIndex;
	glm::vec4 albedo;
	float roughness;
	float ambientOcclusion;
	uint32_t toggleTexture;

	ShaderParameters();
	ShaderParameters(uint32_t texIndex, uint32_t toggleTex = 0, glm::vec4 albedo = glm::vec4{1.0f}, float roughness = 1.0f, float ao = 1.0f);
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
	//TODO: Add a Buffer that can be bound per material: see https://developer.nvidia.com/vulkan-shader-resource-binding
	std::unique_ptr<class Buffer> uniformBuffer;
};