#pragma once

#define GLFW_INCLUDE_VULKAN
#include "glfw3.h"

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>

struct QueueFamilyIndicies
{
	std::optional<uint32_t> graphicsFamily;

	bool isComplete()
	{
		return graphicsFamily.has_value();
	}
};

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

	// Get required GLFW Extensions
	std::vector<const char*> getRequiredExtensions();

	// Select compatible GPU to use
	void selectPhysicalDevice();

	// Check if device is supported
	bool isDeviceSuitable(VkPhysicalDevice device);

	// Find graphics queue family
	QueueFamilyIndicies findQueueFamilies(VkPhysicalDevice device);

	void createLogicalDevice();

	// Update application
	void update();

	// Clean up application
	void cleanup();

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	void setupDebugMessenger();

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
		VkDebugUtilsMessageTypeFlagsEXT messageType, 
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

private:
	const char* name;
	uint32_t width;
	uint32_t height;

	//Window window;
	GLFWwindow* window;
	VkInstance instance;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE; // GPU
	VkDevice device; // Logical device
	VkQueue graphicsQueue;
	VkDebugUtilsMessengerEXT debugMessenger;

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