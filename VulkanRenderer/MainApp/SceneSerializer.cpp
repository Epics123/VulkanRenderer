#include "SceneSerializer.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

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

static void serializeObject(YAML::Emitter& out, GameObject& obj)
{
	out << YAML::BeginMap; // Object
	out << YAML::Key << "Object" << YAML::Value << "12345678"; //TODO: use object ID

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
		out << YAML::EndMap;
	}

	if (obj.spotLight)
	{
		out << YAML::Key << "SpotLightComponent";
		out << YAML::BeginMap;
		out << YAML::Key << "LightType" << YAML::Value << obj.spotLight->lightType;
		out << YAML::Key << "Intensity" << YAML::Value << obj.spotLight->intensity;
		out << YAML::Key << "CutoffAngle" << YAML::Value << obj.spotLight->cutoffAngle;
		out << YAML::Key << "OuterCutoffAngle" << YAML::Value << obj.spotLight->outerCutoffAngle;
		out << YAML::EndMap;
	}

	out << YAML::EndMap; // Object
}

void SceneSerializer::serialize(const std::string& filepath, GameObject::Map& gameObjects)
{
	YAML::Emitter out;
	out << YAML::BeginMap;
	out << YAML::Key << "Scene" << YAML::Value << "Untitled";
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
