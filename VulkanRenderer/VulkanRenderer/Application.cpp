#include "Application.h"

Application::Application()
{
	name = "Vulkan App";
	width = 800;
	height = 600;

	window = nullptr;
}

Application::Application(const char* appName, uint32_t appWidth, uint32_t appHeight)
{
	name = appName;
	width = appWidth;
	height = appHeight;

	window = nullptr;
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
	// Init GLFW
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // Disable window resizing (for now)

	// Create Window
	window = glfwCreateWindow(width, height, name, nullptr, nullptr);
}

void Application::vulkanInit()
{
	createVulkanInstance();
}

void Application::createVulkanInstance()
{
	if(enableValidationLayers && !checkValidationLayerSupport())
		throw std::runtime_error("Validation layers requested, but not available!");

	// Set Vulkan application info
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	// Get GLFW extensions
	std::vector<const char*> glfwExtensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(glfwExtensions.size());
	createInfo.ppEnabledExtensionNames = glfwExtensions.data();

	// Create Vulkan instance
	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create instance!");
	}

	// Get Vulkan extension count
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	// Set available extensions
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	// Print available extensions
	printf("Available Extensions:\n");
	for (uint32_t i = 0; i < extensions.size(); i++)
	{
		printf("\t%s\n", extensions.at(i).extensionName);
	}
}

bool Application::checkValidationLayerSupport()
{
	// Get number of layers
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	// Set layer data
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (uint32_t i = 0; i < validationLayers.size(); i++)
	{
		bool layerFound = false;
		for (uint32_t j = 0; j < availableLayers.size(); j++)
		{
			// Check if validationLayers contains one of the available layers
			if (strcmp(validationLayers.at(i), availableLayers.at(j).layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if(!layerFound)
			return false;
	}

	return true;
}

std::vector<const char*> Application::getRequiredExtensions()
{
	// Get the number of required extensions
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	// Allocate extensions array
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if(enableValidationLayers)
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	return extensions;
}

void Application::update()
{
	while(!glfwWindowShouldClose(window))
		glfwPollEvents();
}

void Application::cleanup()
{
	vkDestroyInstance(instance, nullptr);
	glfwDestroyWindow(window);
	glfwTerminate();
}
