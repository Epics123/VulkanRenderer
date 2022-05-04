#include "Light.h"
#include <glm/gtc/matrix_transform.hpp>

void Light::updateModel()
{
	glm::vec3 p = glm::vec3(pos.x, pos.y, pos.z);
	model = glm::translate(glm::mat4(1.0f), p);
	pos = glm::vec4(p, 0.0f);
}
