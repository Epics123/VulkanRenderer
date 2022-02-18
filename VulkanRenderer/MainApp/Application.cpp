#include "Application.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


Application::Application()
{
	name = "Vulkan App";
	width = 800;
	height = 600;

	window = new Window(name, width, height);
}

Application::Application(const char* appName, uint32_t appWidth, uint32_t appHeight)
{
	name = appName;
	width = appWidth;
	height = appHeight;

	window = new Window(name, width, height);
}

void Application::run()
{
	window->initWindow(keyCallback, cursorPosCallback, mouseButtonCallback, scrollCallback,framebufferResizeCallback, this);
	//vulkanRenderer = new Renderer(window);
	vulkanRenderer = Renderer::initInstance(window);
	update();
	cleanup();
}

void Application::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
	app->framebufferResized = true;
}

void Application::update()
{
	while (!glfwWindowShouldClose(window->getWindow()))
	{
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();
		processInput(window->getWindow());

		vulkanRenderer->drawFrame();
	}

	vulkanRenderer->deviceWaitIdle();
}

void Application::cleanup()
{
	//vulkanRenderer->cleanup();
	Renderer::cleanupInstance();

	//glfwDestroyWindow(window);
	window->cleanupWindow();
	glfwTerminate();

	// Call renderer cleanup
	delete window;
}

void Application::processInput(GLFWwindow* window)
{
	float cameraSpeed = 2.5f * deltaTime;
	if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		vulkanRenderer->getActiveCamera().updatePositon(GLFW_KEY_W, cameraSpeed);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		vulkanRenderer->getActiveCamera().updatePositon(GLFW_KEY_S, cameraSpeed);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		vulkanRenderer->getActiveCamera().updatePositon(GLFW_KEY_A, cameraSpeed);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		vulkanRenderer->getActiveCamera().updatePositon(GLFW_KEY_D, cameraSpeed);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		vulkanRenderer->getActiveCamera().updatePositon(GLFW_KEY_E, cameraSpeed);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		vulkanRenderer->getActiveCamera().updatePositon(GLFW_KEY_Q, cameraSpeed);
}

void Application::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	/*if(key == GLFW_KEY_E && action == GLFW_PRESS)
		printf("E Key Pressed! \n");*/
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void Application::cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
	//printf("%f, %f\n", (float)xpos, (float)ypos);

	if (firstMouse)
	{
		lastMouseX = (float)xpos;
		lastMouseY = (float)ypos;
		firstMouse = false;
	}

	float xOffset = (float)xpos - lastMouseX;
	float yOffset = (float)ypos - lastMouseY;
	lastMouseX = (float)xpos;
	lastMouseY = (float)ypos;

	float sensitivity = 0.1f;
	xOffset *= sensitivity;
	yOffset *= sensitivity;

	printf("%f, %f\n", xOffset, yOffset);

	//Renderer::rendererInstance->getActiveCamera().updateCameraRotation(xOffset, yOffset, 0.0f);
}

void Application::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		printf("Left mouse clicked!\n");
}

void Application::scrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
	Renderer::rendererInstance->getActiveCamera().zoomCamera(yOffset);
}
