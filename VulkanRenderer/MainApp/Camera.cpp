#include "Camera.h"

void Camera::updatePositon(int key, float speed)
{
	switch (key)
	{
	case GLFW_KEY_W:
		position += speed * forward;
		break;
	case GLFW_KEY_S:
		position -= speed * forward;
		break;
	case GLFW_KEY_A:
		position -= right * speed;
		break;
	case GLFW_KEY_D:
		position += right * speed;
		break;
	case GLFW_KEY_E:
		position += speed * up;
		break;
	case GLFW_KEY_Q:
		position -= speed * up;
	}
}
