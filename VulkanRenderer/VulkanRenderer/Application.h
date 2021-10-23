#pragma once

#define GLFW_INCLUDE_VULKAN
#include "glfw3.h"

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>

class Application
{
public:
	Application();
	Application(const char* appName, uint32_t appWidth, uint32_t appHeight);

	void run();

private:
	// Initialize window using GLFW
	void initWindow();
	// Initialize Vulkan library
	void vulkanInit();
	// Create Vulkan instance that will interact with the application
	void createVulkanInstance();
	// Check if requested validation layers are available
	bool checkValidationLayerSupport();
	// Update application
	void update();
	// Clean up application
	void cleanup();

private:
	const char* name;
	uint32_t width;
	uint32_t height;

	GLFWwindow* window;
	VkInstance instance;

	const std::vector<const char*> validationLayers =
	{
		"VK_LAYER_KHRONOS_validation"
	};

	#ifdef NDEBUG // not debug
		const bool enableValidationLayers = false;
	#else
		const bool enableValidationLayers = true;
	#endif
};