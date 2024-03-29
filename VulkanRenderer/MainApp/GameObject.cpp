#include "GameObject.h"

glm::mat4 TransformComponent::getTransform()
{
	glm::mat4 transate = glm::translate(glm::mat4{ 1.0f }, translation);

	glm::quat qPitch = glm::angleAxis(glm::radians(rotation.x), glm::vec3(1, 0, 0));
	glm::quat qYaw = glm::angleAxis(glm::radians(rotation.z), glm::vec3(0, 0, 1));
	glm::quat qRoll = glm::angleAxis(glm::radians(rotation.y), glm::vec3(0, 1, 0));

	orientation = qPitch * qYaw * qRoll;
	glm::mat4 rotate = glm::mat4_cast(orientation);

	glm::quat invOrient = glm::conjugate(orientation);
	forward = invOrient * glm::vec3(0.0f, 0.0f, 1.0f);
	up = invOrient * glm::vec3(0.0f, 1.0f, 0.0f);
	right = glm::normalize(glm::cross(forward, up));

	glm::mat4 scaleMat = glm::scale(transate, scale);

	glm::mat4 transform = scaleMat * transate * rotate;
	return transform;
}

glm::mat3 TransformComponent::getNormalMatrix()
{
	return glm::transpose(glm::inverse(getTransform()));
}

void TransformComponent::updateTransform()
{
	transformMat = getTransform();
}

GameObject GameObject::makePointLight(float intensity, float radius, glm::vec3 color)
{
	GameObject obj = createGameObject();
	obj.transform.scale.x = radius;
	obj.pointLight = std::make_unique<PointLightComponent>();
	obj.pointLight->intensity = intensity;
	obj.pointLight->lightType = LightType::Point;
	obj.pointLight->color = color;

	return obj;
}

GameObject GameObject::makeSpotLight(float intensity, float cutoffAngle, glm::vec3 color)
{
	GameObject obj = createGameObject();
	obj.spotLight = std::make_unique<SpotLightComponent>();
	obj.spotLight->intensity = intensity;
	obj.spotLight->cutoffAngle = cutoffAngle;
	obj.spotLight->lightType = LightType::Spot;
	obj.spotLight->color = color;

	return obj;
}

void GameObject::setPointLightColor(glm::vec3& color)
{
	if(pointLight)
		pointLight->color = color;
}

void GameObject::setSpotLightColor(glm::vec3& color)
{
	if(spotLight)
		spotLight->color = color;
}

void GameObject::setMaterial(ShaderParameters params)
{
	if(materialComp)
		materialComp->material->setShaderParameters(params);
	else
	{
		materialComp = std::make_unique<MaterialComponent>();
		materialComp->material->setShaderParameters(params);
	}
}

void GameObject::setMaterial(Material& mat)
{
	if(materialComp)
	{
		materialComp->material = std::make_shared<Material>(mat);
		materialComp->materialFileName = mat.getMaterialFileName();
	}
	else
	{
		materialComp = std::make_unique<MaterialComponent>();
		materialComp->material = std::make_shared<Material>(mat);
		materialComp->materialFileName = mat.getMaterialFileName();
	}
}

void GameObject::setMaterial(std::shared_ptr<Material> mat)
{
	if (materialComp)
	{
		materialComp->material = mat;
		materialComp->materialFileName = mat->getMaterialFileName();
	}
	else
	{
		materialComp = std::make_unique<MaterialComponent>();
		materialComp->material = mat;
		materialComp->materialFileName = mat->getMaterialFileName();
	}
}
