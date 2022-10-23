#include "GameObject.h"

glm::mat4 TransformComponent::getTransform()
{
	glm::mat4 transate = glm::translate(glm::mat4{ 1.0f }, translation);

	glm::quat qPitch = glm::angleAxis(rotation.x, glm::vec3(1, 0, 0));
	glm::quat qYaw = glm::angleAxis(rotation.z, glm::vec3(0, 0, 1));
	glm::quat qRoll = glm::angleAxis(rotation.y, glm::vec3(0, 0, 1));

	glm::quat orientation = qPitch * qYaw * qRoll;
	glm::mat4 rotate = glm::mat4_cast(orientation);

	glm::mat4 scaleMat = glm::scale(transate, scale);

	glm::mat4 transform = rotate * scaleMat * transate;
	return transform;
}

glm::mat3 TransformComponent::getNormalMatrix()
{
	return glm::transpose(glm::inverse(getTransform()));
}
