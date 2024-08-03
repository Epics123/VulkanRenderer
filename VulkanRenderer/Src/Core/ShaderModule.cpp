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

		// The Includer owns the content memory and will delete it when it is no longer needed.
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
	createShader(filePath, shaderEntryPoint, name);
}

ShaderModule::ShaderModule(const Context* inContext, const std::vector<char>& data, const std::string& entryPoint, VkShaderStageFlagBits stages, const std::string& name)
	: context{inContext}, shaderEntryPoint{entryPoint}, vkStageFlags{stages}
{
	createShader(data, shaderEntryPoint, name);
}

ShaderModule::ShaderModule(const Context* inContext, const std::string& filePath, VkShaderStageFlagBits stages, const std::string& name)
	:ShaderModule(inContext, filePath, "main", stages, name)
{

}

ShaderModule::~ShaderModule()
{
	vkDestroyShaderModule(context->getDevice(), vkShaderModule, nullptr);
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
		spirv = glslToSpirV(fileData, shaderStageFromFileName(filepath.c_str()), file.parent_path().string(), entryPoint.c_str());
	}

	createShader(spirv, entryPoint, name);
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

	if (!glslangInitialized)
	{
		glslang::InitializeProcess();
		glslangInitialized = true;
	}

	glslang::TShader tmpShader(shaderStage);
	const char* glslCStr = data.data();
	tmpShader.setStrings(&glslCStr, 1);

	glslang::EShTargetClientVersion clientVersion = glslang::EShTargetVulkan_1_3;
	glslang::EShTargetLanguageVersion langVersion = glslang::EShTargetSpv_1_0;

	if (shaderStage == EShLangRayGen || shaderStage == EShLangAnyHit || shaderStage == EShLangClosestHit || shaderStage == EShLangMiss)
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
	
	std::string preprocessedGLSL;
	if(!tmpShader.preprocess(resources, 460, ENoProfile, false, false, messages, &preprocessedGLSL, includer))
	{
		CORE_ERROR("Preprocessing failed for shader: ");
		CORE_ERROR("\t{0}", tmpShader.getInfoLog());
		CORE_ERROR("\t{0}", tmpShader.getInfoDebugLog());

		return std::vector<char>();
	}

	preprocessedGLSL = removeUnnecessaryLines(preprocessedGLSL);

	const char* preprocessedGLSLStr = preprocessedGLSL.c_str();
	shader.setStrings(&preprocessedGLSLStr, 1);
	if(!shader.parse(resources, 460, false, messages))
	{
		CORE_ERROR("Parsing failed for shader: ");
		CORE_ERROR("\t{0}", shader.getInfoLog());
		CORE_ERROR("\t{0}", shader.getInfoDebugLog());

		return std::vector<char>();
	}

	glslang::SpvOptions options;

#if _DEBUG
	shader.setDebugInfo(true);
	options.generateDebugInfo = true;
	options.disableOptimizer = true;
	options.optimizeSize = false;
	options.stripDebugInfo = false;
#else
	// Might not actually need this
	options.disableOptimizer = true; // this ensure that variables that aren't used in shaders are not removed, without this flag, SPIRV
									 // generated will be optimized & unused variables will be removed,
									 // this will cause issues in debug vs release if struct on cpu vs gpu are different
	options.optimizeSize = true;
	options.stripDebugInfo = true;
#endif

	glslang::TProgram program;
	program.addShader(&shader);
	if(!program.link(messages))
	{
		CORE_ERROR("Linking failed for shader: ");
		CORE_ERROR("\t{0}", program.getInfoLog());
		CORE_ERROR("\t{0}", program.getInfoDebugLog());

		return std::vector<char>();
	}

	std::vector<uint32_t> spirvData;
	spv::SpvBuildLogger spvLogger;
	glslang::GlslangToSpv(*program.getIntermediate(shaderStage), spirvData, &spvLogger, &options);

	std::vector<char> bytcode;
	bytcode.resize(spirvData.size() * (sizeof(uint32_t) / sizeof(char)));
	std::memcpy(bytcode.data(), spirvData.data(), bytcode.size());
	return bytcode;
}

std::string ShaderModule::removeUnnecessaryLines(const std::string& str)
{
	std::istringstream iss(str);
	std::ostringstream oss;
	std::string line;

	while (std::getline(iss, line))
	{
		if(line != "#extension GL_GOOGLE_include_directive : require" && line.substr(0, 5) != "#line")
		{
			oss << line << '\n';
		}
	}

	return oss.str();
}

EShLanguage ShaderModule::shaderStageFromFileName(const char* fileName)
{
	if(Utils::fileEndsWith(fileName, ".vert"))
	{
		return EShLangVertex;
	}
	else if(Utils::fileEndsWith(fileName, ".frag"))
	{
		return EShLangFragment;
	}
	else if(Utils::fileEndsWith(fileName, ".comp"))
	{
		return EShLangCompute;
	}
	else if(Utils::fileEndsWith(fileName, ".rgen"))
	{
		return EShLangRayGen;
	}
	else if(Utils::fileEndsWith(fileName, ".rmiss"))
	{
		return EShLangMiss;
	}
	else if(Utils::fileEndsWith(fileName, ".rchit"))
	{
		return EShLangClosestHit;
	}
	else if(Utils::fileEndsWith(fileName, ".rahit"))
	{
		return EShLangAnyHit;
	}

	return EShLangVertex;
}
