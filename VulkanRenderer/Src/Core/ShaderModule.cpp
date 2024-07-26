#include "ShaderModule.h"
#include "Context.h"

#include "Logging/Log.h"
#include "Utils/Utils.h"

#include <glslang/Public/ResourceLimits.h>
#include <glslang/SPIRV/GlslangToSpv.h>

#include <filesystem>
#include <fstream>
#include <sstream>

static constexpr uint32_t MAX_DESC_BINDLESS = 1000;
class CustomIncluder final : public glslang::TShader::Includer
{
public:
	explicit CustomIncluder(const std::string& shaderDir) : shaderDirectory(shaderDir) {}
	~CustomIncluder() = default;

	IncludeResult* includeSystem(const char* headerName, const char* includerName, size_t inclusionDepth) override
	{
		// You can implement system include paths here if needed.
		return nullptr;
	}

	IncludeResult* includeLocal(const char* headerName, const char* includerName, size_t inclusionDepth) override
	{
		std::string fullPath = shaderDirectory + "/" + headerName;
		std::ifstream fileStream(fullPath, std::ios::in);
		if (!fileStream.is_open())
		{
			std::string errMsg = "Failed to open included file: ";
			errMsg.append(headerName);
			CORE_ERROR(errMsg);
			return nullptr;
		}

		std::stringstream fileContent;
		fileContent << fileStream.rdbuf();
		fileStream.close();

		// The Includer owns the content memory and will delete it when it is no
		// longer needed.
		char* content = new char[fileContent.str().length() + 1];
		strncpy(content, fileContent.str().c_str(), fileContent.str().length());
		content[fileContent.str().length()] = '\0';

		return new IncludeResult(headerName, content, fileContent.str().length(), nullptr);
	}

	void releaseInclude(IncludeResult* result) override
	{
		if (result)
		{
			delete result;
		}
	}

private:
	std::string shaderDirectory;
};

ShaderModule::ShaderModule(const Context* inContext, const std::string& filePath, const std::string& entryPoint, VkShaderStageFlagBits stages, const std::string& name)
	: context{inContext}, shaderEntryPoint{entryPoint}, vkStageFlags{stages}
{
	
}

void ShaderModule::createShader(const std::string& filepath, const std::string& entryPoint, const std::string& name)
{
	std::vector<char> spirv;
	const bool isBinary = Utils::fileEndsWith(filepath.c_str(), ".spv");

	std::vector<char> fileData = Utils::readFile(filepath, isBinary);
	std::filesystem::path file(filepath);
	if(isBinary)
	{
		spirv = std::move(fileData);
	}
	else
	{

	}
}

void ShaderModule::createShader(const std::vector<char> spirv, const std::string& entryPoint, const std::string& name)
{
	VkShaderModuleCreateInfo shaderModuleInfo{};
	shaderModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleInfo.codeSize = spirv.size();
	shaderModuleInfo.pCode = (const uint32_t*)spirv.data();
	shaderModuleInfo.pNext = VK_NULL_HANDLE;

	VkResult result = vkCreateShaderModule(context->getDevice(), &shaderModuleInfo, nullptr, &vkShaderModule);
	if(result != VK_SUCCESS)
	{
		CORE_CRITICAL("Failed to create shader module!: Error code {0}", result);
	}

	debugName = "Shader Module: " + name;
}

const std::vector<char> ShaderModule::glslToSpirV(const std::vector<char>& data, EShLanguage shaderStage, const std::string& shaderDir, const char* entryPoint)
{
	static bool glslangInitialized = false;

	if(!glslangInitialized)
	{
		glslang::InitializeProcess();
		glslangInitialized = true;
	}

	glslang::TShader tmpShader(shaderStage);
	const char* glslCStr = data.data();
	tmpShader.setStrings(&glslCStr, 1);

	glslang::EShTargetClientVersion clientVersion = glslang::EShTargetVulkan_1_3;
	glslang::EShTargetLanguageVersion langVersion = glslang::EShTargetSpv_1_0;

	if(shaderStage == EShLangRayGen || shaderStage == EShLangAnyHit || shaderStage == EShLangClosestHit || shaderStage == EShLangMiss)
	{
		langVersion = glslang::EShTargetSpv_1_4;
	}

	tmpShader.setEnvInput(glslang::EShSourceGlsl, shaderStage, glslang::EShClientVulkan, 460);
	tmpShader.setEnvClient(glslang::EShClientVulkan, clientVersion);

	tmpShader.setEntryPoint(entryPoint);
	tmpShader.setSourceEntryPoint(entryPoint);

	glslang::TShader shader(shaderStage);
	shader.setEnvClient(glslang::EShClientVulkan, clientVersion);
	shader.setEnvTarget(glslang::EShTargetSpv, langVersion);

	shader.setEntryPoint(entryPoint);
	shader.setSourceEntryPoint(entryPoint);

	const TBuiltInResource* resources = GetDefaultResources();
	const EShMessages messages = static_cast<EShMessages>(EShMsgDefault | EShMsgSpvRules | EShMsgVulkanRules | EShMsgDebugInfo);

	CustomIncluder includer(shaderDir);

}
