#include "Application.h"
//#include <set>
//#include <cstdint>
//#include <algorithm>
//#include <fstream>
//#include <chrono>

//#include "Renderer.h"

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
	window->initWindow(keyCallback, cursorPosCallback, mouseButtonCallback, framebufferResizeCallback, this);
	vulkanRenderer = new Renderer(window);
	//vulkanInit();
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
		glfwPollEvents();

		vulkanRenderer->drawFrame();
	}

	vulkanRenderer->deviceWaitIdle();
}

void Application::cleanup()
{
	vulkanRenderer->cleanup();

	//glfwDestroyWindow(window);
	window->cleanupWindow();
	glfwTerminate();

	// Call renderer cleanup
	delete window;
	delete vulkanRenderer;
}

void Application::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_E && action == GLFW_PRESS)
		printf("E Key Pressed! \n");
}

void Application::cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
	printf("%f, %f\n", (float)xpos, (float)ypos);
}

void Application::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		printf("Left mouse clicked!\n");
}
