#include "Light.h"
#include <glm/gtc/matrix_transform.hpp>

void Light::updateModel()
{
	model = glm::translate(glm::mat4(1.0f), pos);
}
