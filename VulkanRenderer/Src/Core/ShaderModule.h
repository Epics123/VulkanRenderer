#pragma once

#include <vulkan/vulkan.h>

#include <glslang/Public/ShaderLang.h>

#include <string>
#include <vector>

class Context;

class ShaderModule
{
public:
	ShaderModule(const Context* inContext, const std::string& filePath, const std::string& entryPoint, VkShaderStageFlagBits stages, const std::string& name);
	ShaderModule(const Context* inContext, const std::vector<char>& data, const std::string& entryPoint, VkShaderStageFlagBits stages, const std::string& name);
	ShaderModule(const Context* inContext, const std::string& filePath, VkShaderStageFlagBits stages, const std::string& name);

	~ShaderModule();

	VkShaderModule getShaderModule() const { return vkShaderModule; }
	VkShaderStageFlagBits getShaderFlags() const { return vkStageFlags; }
	const std::string& getEntryPoint() const { return shaderEntryPoint; }

private:

	void createShader(const std::string& filepath, const std::string& entryPoint, const std::string& name);
	void createShader(const std::vector<char> spirv, const std::string& entryPoint, const std::string& name);

	const std::vector<char> glslToSpirV(const std::vector<char>& data, EShLanguage shaderStage, const std::string& shaderDir, const char* entryPoint);

	std::string removeUnnecessaryLines(const std::string& str);

	EShLanguage shaderStageFromFileName(const char* fileName);
	
private:
	const Context* context = nullptr;
	VkShaderModule vkShaderModule = VK_NULL_HANDLE;
	VkShaderStageFlagBits vkStageFlags;
	std::string shaderEntryPoint;
	std::string debugName = "";
};

