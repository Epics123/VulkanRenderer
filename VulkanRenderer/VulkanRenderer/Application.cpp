#include "Application.h"

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		func(instance, debugMessenger, pAllocator);
	}
}

Application::Application()
{
	name = "Vulkan App";
	width = 800;
	height = 600;

	//window = nullptr;
}

Application::Application(const char* appName, uint32_t appWidth, uint32_t appHeight)
{
	name = appName;
	width = appWidth;
	height = appHeight;

	//window = nullptr;
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

	//// Create Window
	window = glfwCreateWindow(width, height, name, nullptr, nullptr);
}

void Application::vulkanInit()
{
	createVulkanInstance();
	setupDebugMessenger();
	selectPhysicalDevice();
	createLogicalDevice();
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
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
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
	printf("\nAvailable Extensions:\n");
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

void Application::selectPhysicalDevice()
{
	// Get device count
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	// Check if there are any devices that support Vulkan
	if(deviceCount == 0)
		throw std::runtime_error("Failed to find GPUs with Vulkan support!");

	// Set available devices
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	for (uint32_t i = 0; i < devices.size(); i++)
	{
		// Check if the device is suitable
		if (isDeviceSuitable(devices.at(i)))
		{
			physicalDevice = devices.at(i);
			break;
		}
	}

	if(physicalDevice == VK_NULL_HANDLE)
		throw std::runtime_error("Failed to find a compatible GPU!");
}

bool Application::isDeviceSuitable(VkPhysicalDevice device)
{
	// Can be extended later (see https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Physical_devices_and_queue_families)
	QueueFamilyIndicies indicies = findQueueFamilies(device);

	return indicies.isComplete(); 
}

QueueFamilyIndicies Application::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndicies indicies;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	for (uint32_t i = 0; i < queueFamilies.size(); i++)
	{
		if(queueFamilies.at(i).queueFlags & VK_QUEUE_GRAPHICS_BIT)
			indicies.graphicsFamily = i;

		if(indicies.isComplete())
			break;
	}

	return indicies;
}

void Application::createLogicalDevice()
{
	QueueFamilyIndicies indicies = findQueueFamilies(physicalDevice);

	// Specify queues to be created
	VkDeviceQueueCreateInfo queueCreateInfo{};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = indicies.graphicsFamily.value();
	queueCreateInfo.queueCount = 1;

	// Set queue priority
	float queuePriority = 1.0f; // must be between 0.0 and 1.0f
	queueCreateInfo.pQueuePriorities = &queuePriority;

	// Specify device features
	VkPhysicalDeviceFeatures deviceFeatures{};

	// Create logical device info
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = &queueCreateInfo;
	createInfo.queueCreateInfoCount = 1;
	createInfo.pEnabledFeatures = &deviceFeatures;

	// Set device specific validation layers
	createInfo.enabledExtensionCount = 0;
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	// Create the logical device
	if(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
		throw std::runtime_error("Failed to create logical device!");

	// Get queue handles
	vkGetDeviceQueue(device, indicies.graphicsFamily.value(), 0, &graphicsQueue);
}

void Application::update()
{
	while(!glfwWindowShouldClose(window))
		glfwPollEvents();
}

void Application::cleanup()
{
	if(enableValidationLayers)
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	vkDestroyDevice(device, nullptr);
	vkDestroyInstance(instance, nullptr);
	glfwDestroyWindow(window);
	glfwTerminate();
}

void Application::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr; // Optional
}

void Application::setupDebugMessenger()
{
	if(!enableValidationLayers)
		return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	populateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to set up debug messenger!");
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL Application::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	std::cerr << "Validation Layers: " << pCallbackData->pMessage << std::endl;
	return VK_FALSE;
}
