#pragma once

#include "Texture.h"
#include "Pipeline.h"

struct ShaderEffect
{
	VkPipelineLayout builtLayout;
	std::vector<VkDescriptorSetLayout> setLayouts;

	struct ShaderStage
	{
		VkShaderModule* shaderModule;
		VkShaderStageFlagBits stage;
	};

	std::vector<ShaderStage> stages;
};

struct ShaderPass
{
	ShaderEffect* effect {nullptr};
	Pipeline* pipeline = nullptr;
};

// Ubo for a given material
struct ShaderParameters
{
	
};

struct EffectTemplate
{
	std::vector<ShaderPass*> passShaders;

};

struct MaterialData
{
	Texture texture;
};

class Material
{
public:
	Material();
	~Material();

	MaterialData& getMaterialData() { return data; }

private:
	MaterialData data;

};