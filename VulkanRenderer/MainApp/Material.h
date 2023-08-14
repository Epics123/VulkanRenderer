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
	std::string textureDir;

	ShaderParameters();
	ShaderParameters(uint32_t texIndex, uint32_t toggleTex = 0, glm::vec4 albedo = glm::vec4{1.0f}, float roughness = 1.0f, float ao = 1.0f, float metallic = 0.0f);
};

class Material
{
public:
	Material();
	Material(ShaderParameters& params);
	~Material();

	// Gets the material's parameters that will be passed to a shader
	ShaderParameters& getShaderParameters() { return shaderParams; }
	void setShaderParameters(ShaderParameters& params);

	// Sets the file name of this material
	void setMaterialFileName(const std::string& file) { fileName = file; }
	// Gets the current file name of this material (does not return a path)
	const std::string& getMaterialFileName() { return fileName; }

	// Set the internal name to be used by this material
	void setNameInternal(const std::string& name) { nameInternal = name; }
	// Get the internal name of this material
	const std::string& getNameInternal() { return nameInternal; }

	void cleanup(class Device& device);

private:
	ShaderParameters shaderParams;
	std::string fileName;
	std::string nameInternal;
};

class MaterialBuilder
{
public:
	// Returns the file path to the materials directory
	static const std::string& getMaterialFilePath() { return materialPath; }

	int getBindingFromFileName(const std::string& filename);
	
	const std::string albedoExtension = "_albedo";
	const std::string normalExtension = "_normal";
	const std::string roughnessExtension = "_roughness";
	const std::string aoExtension = "_ao";
	const std::string heightExtension = "_height";
	const std::string metallicExtension = "_metallic";

private:
	static const std::string materialPath;
};