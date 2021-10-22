#include "Application.h"

Application::Application()
{
	name = "Vulkan App";
	width = 800;
	height = 600;
}

Application::Application(const char* appName, uint32_t appWidth, uint32_t appHeight)
{
	name = appName;
	width = appWidth;
	height = appHeight;
}

void Application::run()
{
	initWindow();
	vulkanInit();
	update();
	cleanup();
}

void Application::initWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // Disable window resizing (for now)

	window = glfwCreateWindow(width, height, name, nullptr, nullptr);
}

void Application::vulkanInit()
{

}

void Application::update()
{
	while(!glfwWindowShouldClose(window))
		glfwPollEvents();
}

void Application::cleanup()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}
