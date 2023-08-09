#include "Material.h"
#include "Log.h"
#include "Utils.h"
#include "Device.h"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <string>

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

void Material::setShaderParameters(ShaderParameters params)
{
	shaderParams.albedo = params.albedo;
	shaderParams.ambientOcclusion = params.ambientOcclusion;
	shaderParams.roughness = params.roughness;
	shaderParams.metallic = params.metallic;
	shaderParams.textureIndex = params.textureIndex;
	shaderParams.toggleTexture = params.toggleTexture;
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

Material MaterialBuilder::buildMaterial(std::string filepath, Device& device)
{
	std::ifstream inFile;
	inFile.open(filepath);

	ShaderParameters params{};

	std::string name;
	bool usingTexture = false;
	if(inFile.is_open())
	{
		std::string line;
		while(inFile)
		{
			std::getline(inFile, line);
			if(line.compare("#name") == 0) // why the fuck does compare return 0 if strings are equal??
			{
				std::getline(inFile, line);
				name = line;
			}
			if (line.compare("#toggleTexture") == 0)
			{
				std::getline(inFile, line);
				params.toggleTexture = std::stoi(line);
				usingTexture = params.toggleTexture == 0 ? false : true;
			}
			if (line.compare("#params") == 0)
			{
				if(usingTexture)
				{
					std::getline(inFile, line);
					//load textures relevant to this material
					if(line.compare("#textureDir") == 0)
					{
						std::string textureDir;
						std::getline(inFile, textureDir);

						for(const auto& entry : std::filesystem::directory_iterator(textureDir))
						{
							std::filesystem::path path = entry.path();
							std::string fileName = path.string();

							//TODO: Handle case where multiple materials reference the same textures

							Texture texture;

							//Check if the texture we are loading is a normal map
							if (path.stem().string().find(normalExtension) != std::string::npos)
							{
								//Set texture format for normal maps
								texture.setTextureFormat(VK_FORMAT_R8G8B8A8_UNORM);
								Utils::loadImageFromFile(device, fileName.c_str(), texture, VK_FORMAT_R8G8B8A8_UNORM);
							}
							// load texture with default format
							else
							{
								Utils::loadImageFromFile(device, fileName.c_str(), texture);
							}
							texture.createTextureImageView(device);
							texture.createTextureSampler(device);
							texture.setNameInternal(path.stem().string());

							int binding = getBindingFromFileName(path.stem().string());
							if(binding < 0)
							{
								CORE_ERROR("Invalid file name to find binding: {0}", fileName)
							}
							else
							{
								params.materialTextures[(uint32_t)binding] = texture;
							}
						}
					}
				}
				else
				{
					std::getline(inFile, line);
					
					std::stringstream ss;
					ss << line;
					std::string tmp;

					std::vector<float> albedoValues;
					while(!ss.eof())
					{
						ss >> tmp;
						float num = std::stof(tmp);
						albedoValues.push_back(num);
					}

					glm::vec4 albedo;
					if(albedoValues.size() < 4)
					{
						CORE_ERROR("Invalid albedo data in material file!")
						albedo = glm::vec4(1.0f);
					}
					else
					{
						albedo = { albedoValues[0], albedoValues[1], albedoValues[2], albedoValues[3] };
					}
					params.albedo = albedo;

					std::getline(inFile, line);
					params.roughness = std::stof(line);
					std::getline(inFile, line);
					params.ambientOcclusion = std::stof(line);
					std::getline(inFile, line);
					params.metallic = std::stof(line);
				}
			}
		}
		inFile.close();

		return Material(params);
	}

	CORE_ERROR("Failed to open file: {0}", filepath)
	return Material();
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
