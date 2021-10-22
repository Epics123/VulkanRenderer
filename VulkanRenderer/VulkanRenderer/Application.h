#pragma once

#define GLFW_INCLUDE_VULKAN
#include "glfw3.h"
#include <iostream>
#include <stdexcept>
#include <cstdlib>

class Application
{
public:
	Application();
	Application(const char* appName, uint32_t appWidth, uint32_t appHeight);

	void run();

private:
	void initWindow();
	void vulkanInit();
	void createVulkanInstance();
	void update();
	void cleanup();

private:
	const char* name;
	uint32_t width;
	uint32_t height;

	GLFWwindow* window;
	VkInstance instance;
};