#include "Material.h"
#include "Log.h"

#include <fstream>
#include <sstream>

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

Material& MaterialBuilder::buildMaterial(std::string filepath)
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
				CORE_INFO(name);
			}
			if (line.compare("#toggleTexture") == 0)
			{
				std::getline(inFile, line);
				params.toggleTexture = std::stoi(line);
				usingTexture = false;
			}
			if (line.compare("#params") == 0)
			{
				if(usingTexture)
				{
					//load textures relevant to this material
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

		Material mat = Material(params);
		return mat;
	}

	return Material();
}
