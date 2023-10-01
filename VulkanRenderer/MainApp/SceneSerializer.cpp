#include "SceneSerializer.h"

#include "Log.h"
#include "Device.h"
#include "Utils.h"
#include "Utils/YamlHelpers.h"

#include <fstream>
#include <sstream>
#include <filesystem>

YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
	return out;
}

YAML::Emitter& operator<<(YAML::Emitter& out, const LightType t)
{
	switch (t)
	{
	case Point:
		out << YAML::Value << "Point";
		break;
	case Spot:
		out << YAML::Value << "Spot";
		break;
	case Directional:
		out << YAML::Value << "Directional";
	}
	return out;
}

LightType getLightType(const std::string& typeStr)
{
	if(typeStr == "Point")
		return Point;
	else if(typeStr == "Spot")
		return Spot;
	else
		return Directional;
}

static void serializeObject(YAML::Emitter& out, GameObject& obj)
{
	out << YAML::BeginMap; // Object
	out << YAML::Key << "Object" << YAML::Value << obj.getID(); //TODO: use object ID

	out << YAML::Key << "Tag";
	out << YAML::BeginMap; // Name
	out << YAML::Key << "Name" << YAML::Value << obj.getObjectName();
	out << YAML::EndMap; // Name

	out << YAML::Key << "Transform";
	glm::vec3& translation = obj.transform.translation;
	glm::vec3& rotation = obj.transform.rotation;
	glm::vec3& scale = obj.transform.scale;
	out << YAML::BeginMap; // Transform
	out << YAML::Key << "Translation" << YAML::Value << translation;
	out << YAML::Key << "Rotation" << YAML::Value << rotation;
	out << YAML::Key << "Scale" << YAML::Value << scale;
	out << YAML::EndMap; // Transform

	if(obj.model)
	{
		out << YAML::Key << "Model";
		out << YAML::BeginMap;
		out << YAML::Key << "ModelPath" << YAML::Value << obj.model->getModelPath();
		out << YAML::EndMap;
	}

	if(obj.materialComp)
	{
		out << YAML::Key << "Material";
		out << YAML::BeginMap;
		out << YAML::Key << "MaterialFile" << YAML::Value << obj.materialComp->materialFileName;
		out << YAML::EndMap;
	}

	if(obj.pointLight)
	{
		out << YAML::Key << "PointLightComponent";
		out << YAML::BeginMap;
		out << YAML::Key << "LightType" << YAML::Value << obj.pointLight->lightType;
		out << YAML::Key << "Intensity" << YAML::Value << obj.pointLight->intensity;
		glm::vec3 color = obj.pointLight->color;
		out << YAML::Key << "Color" << YAML::Value << color;
		out << YAML::EndMap;
	}

	if (obj.spotLight)
	{
		out << YAML::Key << "SpotLightComponent";
		out << YAML::BeginMap;
		out << YAML::Key << "LightType" << YAML::Value << obj.spotLight->lightType;
		out << YAML::Key << "Intensity" << YAML::Value << obj.spotLight->intensity;
		glm::vec3 color = obj.spotLight->color;
		out << YAML::Key << "Color" << YAML::Value << color;
		out << YAML::Key << "CutoffAngle" << YAML::Value << obj.spotLight->outerCutoffAngle;
		out << YAML::Key << "OuterCutoffAngle" << YAML::Value << obj.spotLight->cutoffAngle;
		out << YAML::EndMap;
	}

	if(obj.directionalLight)
	{
		out << YAML::Key << "DirectionalLightComponent";
		out << YAML::BeginMap;
		out << YAML::Key << "LightType" << YAML::Value << obj.directionalLight->lightType;
		out << YAML::Key << "Intensity" << YAML::Value << obj.directionalLight->intensity;
		glm::vec3 color = obj.directionalLight->color;
		out << YAML::Key << "Color" << YAML::Value << color;
		glm::vec3& direction = obj.directionalLight->direction;
		out << YAML::Key << "Direction" << YAML::Value << direction;
		out << YAML::EndMap;
	}

	out << YAML::EndMap; // Object
}

void SceneSerializer::serialize(const std::string& filepath, GameObject::Map& gameObjects)
{
	YAML::Emitter out;
	out << YAML::BeginMap;
	out << YAML::Key << "Scene" << YAML::Value << "Untitled";
	out << YAML::Key << "TotalObjectCount" << YAML::Value << gameObjects.size();
	int count = 0;
	for (auto& pair : gameObjects)
	{
		if (pair.second.materialComp)
		{
			count++;
		}
	}
	out << YAML::Key << "TotalMaterialCount" << YAML::Value << count;
	out << YAML::Key << "Objects" << YAML::Value << YAML::BeginSeq;

	for(auto& pair : gameObjects)
	{
		serializeObject(out, pair.second);
	}

	out << YAML::EndSeq;
	out << YAML::EndMap;

	std::ofstream fout(filepath);
	fout << out.c_str();
}

bool SceneSerializer::deserialize(const std::string& filepath, Device& device, SceneData& outSceneData)
{
	std::ifstream inFile(filepath);
	std::stringstream ss;
	ss << inFile.rdbuf();

	YAML::Node data = YAML::Load(ss.str());
	if(!data["Scene"])
	{
		CORE_ERROR("Scene Deserialization Failed: Scene file was not in the correct format!")
		return false;
	}

	std::string sceneName = data["Scene"].as<std::string>();
	outSceneData.sceneName = sceneName;

	uint32_t objectCount = data["TotalObjectCount"].as<uint32_t>();
	outSceneData.objectCount = objectCount;

	uint32_t materialCount = data["TotalMaterialCount"].as<uint32_t>();
	outSceneData.materialCount = materialCount;

	auto objects = data["Objects"];
	if(objects)
	{
		for (auto object : objects)
		{
			GameObject deserializedObj = GameObject::createGameObject();

			int uuid = object["Object"].as<int>(); // TODO: Use uuid
			
			std::string name;
			auto tagComponent = object["Tag"];
			if(tagComponent)
			{
				name = tagComponent["Name"].as<std::string>();
				deserializedObj.setObjectName(name);
			}

			auto transformComponent = object["Transform"];
			if(transformComponent)
			{
				TransformComponent& tc = deserializedObj.transform;
				tc.translation = transformComponent["Translation"].as<glm::vec3>();
				tc.rotation = transformComponent["Rotation"].as<glm::vec3>();
				tc.scale = transformComponent["Scale"].as<glm::vec3>();
			}

			auto modelComponent = object["Model"];
			if(modelComponent)
			{
				std::string modelFile = modelComponent["ModelPath"].as<std::string>();
				std::shared_ptr<Model> model = Model::createModelFromFile(device, modelFile);
				deserializedObj.model = model;
			}

			auto materialComponent = object["Material"];
			if(materialComponent)
			{
				std::string materialFile = materialComponent["MaterialFile"].as<std::string>();

				// check if we have already loaded this material
				if(outSceneData.materials.find(materialFile) != outSceneData.materials.end())
				{
					deserializedObj.setMaterial(outSceneData.materials[materialFile]); // Material already loaded, so just assign to object
				}
				// load new material if not already loaded
				else
				{
					Material mat;
					if(materialFile.empty())
					{
						// default material if no file exists
						mat.setShaderParameters(ShaderParameters{});
						deserializedObj.setMaterial(mat);
					}
					else
					{
						mat = MaterialSerializer::deserialize(MaterialBuilder::getMaterialFilePath() + materialFile, device);
						mat.setMaterialFileName(materialFile);
						std::shared_ptr<Material> matPtr = std::make_shared<Material>(mat);
						outSceneData.materials.emplace(materialFile, matPtr);

						deserializedObj.setMaterial(matPtr);
					}
				}
			}

			auto pointLightComponent = object["PointLightComponent"];
			if(pointLightComponent)
			{
				std::string lightTypeStr = pointLightComponent["LightType"].as<std::string>();
				LightType type = getLightType(lightTypeStr);

				float intensity = pointLightComponent["Intensity"].as<float>();
				glm::vec3 color = pointLightComponent["Color"].as<glm::vec3>();

				deserializedObj.pointLight = std::make_unique<PointLightComponent>();
				deserializedObj.pointLight->intensity = intensity;
				deserializedObj.pointLight->lightType = type;
				deserializedObj.pointLight->color = color;
			}

			auto spotLightComponent = object["SpotLightComponent"];
			if(spotLightComponent)
			{
				std::string lightTypeStr = spotLightComponent["LightType"].as<std::string>();
				LightType type = getLightType(lightTypeStr);

				float intensity = spotLightComponent["Intensity"].as<float>();
				glm::vec3 color = spotLightComponent["Color"].as<glm::vec3>();

				float cutoffAngle = spotLightComponent["CutoffAngle"].as<float>();
				float outerCuttoffAngle = spotLightComponent["OuterCutoffAngle"].as<float>();

				deserializedObj.spotLight = std::make_unique<SpotLightComponent>();
				deserializedObj.spotLight->intensity = intensity;
				deserializedObj.spotLight->lightType = type;
				deserializedObj.spotLight->color = color;
				deserializedObj.spotLight->cutoffAngle = outerCuttoffAngle;
				deserializedObj.spotLight->outerCutoffAngle = cutoffAngle;
			}

			auto directionalLightComponent = object["DirectionalLightComponent"];
			if(directionalLightComponent)
			{
				std::string lightTypeStr = directionalLightComponent["LightType"].as<std::string>();
				LightType type = getLightType(lightTypeStr);

				float intensity = directionalLightComponent["Intensity"].as<float>();
				glm::vec3 color = directionalLightComponent["Color"].as<glm::vec3>();
				glm::vec3 direction = directionalLightComponent["Direction"].as<glm::vec3>();

				deserializedObj.directionalLight = std::make_unique<DirectionalLightComponent>();
				deserializedObj.directionalLight->direction = direction;
				deserializedObj.directionalLight->color = color;
				deserializedObj.directionalLight->intensity = intensity;
				deserializedObj.directionalLight->lightType = type;
			}

			outSceneData.objects.emplace(deserializedObj.getID(), std::move(deserializedObj));
		}
	}

	return true;
}

void MaterialSerializer::serialize(const std::string& filepath, std::shared_ptr<Material> material)
{
	YAML::Emitter out;
	out << YAML::BeginMap;
	out << YAML::Key << "Material" << YAML::Value << material->getNameInternal();
	out << YAML::Key << "Params" << YAML::Value << YAML::BeginSeq;

	out << YAML::BeginMap;
	out << YAML::Key << "ToggleTextures" << YAML::Value << material->getShaderParameters().toggleTexture;
	out << YAML::EndMap;

	bool usingTexture = material->getShaderParameters().toggleTexture == 1 ? true : false;

	if (usingTexture)
	{
		out << YAML::BeginMap;
		out << YAML::Key << "TextureDir" << YAML::Value << material->getShaderParameters().textureDir;
		out << YAML::EndMap;
	}
	else
	{
		glm::vec4& albedo = material->getShaderParameters().albedo;
		out << YAML::BeginMap;
		out << YAML::Key << "Albedo" << YAML::Value << albedo;
		out << YAML::EndMap;

		out << YAML::BeginMap;
		out << YAML::Key << "Roughness" << YAML::Value << material->getShaderParameters().roughness;
		out << YAML::EndMap;

		out << YAML::BeginMap;
		out << YAML::Key << "AmbientOcclusion" << YAML::Value << material->getShaderParameters().ambientOcclusion;
		out << YAML::EndMap;

		out << YAML::BeginMap;
		out << YAML::Key << "Metallic" << YAML::Value << material->getShaderParameters().metallic;
		out << YAML::EndMap;
	}

	out << YAML::EndSeq;
	out << YAML::EndMap;

	std::ofstream fout(filepath);
	fout << out.c_str();
}

Material MaterialSerializer::deserialize(const std::string& filepath, class Device& device)
{
	std::ifstream inFile(filepath);
	std::stringstream ss;
	ss << inFile.rdbuf();

	YAML::Node data = YAML::Load(ss.str());
	if (!data["Material"])
	{
		CORE_ERROR("Material deserialization failed for [{0}]: Material file was not in the correct format!", filepath)
		return Material();
	}

	Material mat;
	ShaderParameters params;

	std::string materialName = data["Material"].as<std::string>();
	mat.setNameInternal(materialName);

	auto materialParams = data["Params"];
	for (auto param : materialParams)
	{
		if (param)
		{
			auto toggleTex = param["ToggleTextures"];
			if(toggleTex)
			{
				uint32_t toggleTexture = param["ToggleTextures"].as<uint32_t>();
				params.toggleTexture = toggleTexture;
			}

			if(params.toggleTexture == 0)
			{
				auto albedoParam = param["Albedo"];
				if (albedoParam)
				{
					glm::vec4 albedo = param["Albedo"].as<glm::vec4>();
					params.albedo = albedo;
				}

				auto roughnessParams = param["Roughness"];
				if (roughnessParams)
				{
					float roughness = param["Roughness"].as<float>();
					params.roughness = roughness;
				}

				auto aoParams = param["AmbientOcclusion"];
				if (aoParams)
				{
					float ao = param["AmbientOcclusion"].as<float>();
					params.ambientOcclusion = ao;
				}

				auto metallicParams = param["Metallic"];
				if (metallicParams)
				{
					float metallic = param["Metallic"].as<float>();
					params.metallic = metallic;
				}
			}
			else
			{
				auto textureParams = param["TextureDir"];
				if(textureParams)
				{
					std::string textureDir = param["TextureDir"].as<std::string>();
					params.textureDir = textureDir;

					MaterialBuilder builder;

					for (const auto& entry : std::filesystem::directory_iterator(textureDir))
					{
						std::filesystem::path path = entry.path();
						std::string fileName = path.string();

						//TODO: Handle case where multiple materials reference the same textures

						Texture texture;

						//Check if the texture we are loading is a normal map
						if (path.stem().string().find(builder.normalExtension) != std::string::npos)
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

						int binding = builder.getBindingFromFileName(path.stem().string());
						if (binding < 0)
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
		}
	}

	mat.setShaderParameters(params);

	return mat;
}
