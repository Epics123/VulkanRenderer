#include "Renderer.h"

#include <set>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <iostream>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "imgui_internal.h"

Renderer* Renderer::rendererInstance = nullptr;

Renderer* Renderer::initInstance(Window* window)
{
	if (!Renderer::rendererInstance)
	{
		Renderer::rendererInstance = new Renderer(window);

		return Renderer::rendererInstance;
	}

	return Renderer::rendererInstance;
}

void Renderer::cleanupInstance()
{
	if (Renderer::rendererInstance)
	{
		Renderer::rendererInstance->cleanup();

		delete Renderer::rendererInstance;
		Renderer::rendererInstance = nullptr;
	}
}

Renderer::Renderer(Window* appWindow)
{
	modelPath = "MainApp/resources/vulkan/models/teapot/downScaledPot.obj";
	//modelPath = "MainApp/resources/vulkan/models/debug/defCube.obj";
	//modelPath = "MainApp/resources/vulkan/models/debug/tri.obj";
	//modelPath = "MainApp/resources/vulkan/models/room/room.obj";
	texturePath = "MainApp/resources/vulkan/textures/bricks/Bricks_basecolor.png";
	//texturePath = "MainApp/resources/vulkan/textures/room/viking_room.png";

	window = appWindow;
	init();
}

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

VKAPI_ATTR VkBool32 VKAPI_CALL Renderer::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	std::cerr << "Validation Layers: " << pCallbackData->pMessage << std::endl;
	return VK_FALSE;
}

void Renderer::init()
{
	createVulkanInstance();
	setupDebugMessenger();

	createSurface();
	selectPhysicalDevice();
	createLogicalDevice();

	createSwapChain();
	createImageViews();
	createRenderPass();
	createDescriptorSetLayout();
	compileShaders();
	createGraphicsPipeline();

	createCommandPool();

	createDepthResources();
	createFrameBuffers();

	createTextureImage();
	createTextureImageView();
	createTextureSampler(device, physicalDevice, textureSampler);

	loadModel();

	//prepareInstanceData();

	vertexBuffer.createVertexBuffer(device, vertices, physicalDevice, commandPool, graphicsQueue);
	indexBuffer.createIndexBuffer(device, indices, physicalDevice, commandPool, graphicsQueue);
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();

	createCommandBuffers();
	createSyncObjects();

	mainCamera = Camera();
	mainCamera.updateModel(0.0f);

	imguiInit();

	initScene();
}

void Renderer::imguiInit()
{
	createImguiRenderPass();

	VkImageView attachment[1];
	VkFramebufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	info.renderPass = imguiRenderPass;
	info.attachmentCount = 1;
	info.pAttachments = attachment;
	info.width = swapChainImageExtent.width;
	info.height = swapChainImageExtent.height;
	info.layers = 1;

	for (size_t i = 0; i < swapChainImageViews.size(); i++)
	{
		attachment[0] = swapChainImageViews[i];
		VkResult res = vkCreateFramebuffer(device, &info, nullptr, &imguiFramebuffers[i]);
	}

	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGuiIO& io = ImGui::GetIO();
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

	ImGui_ImplGlfw_InitForVulkan(window->getWindow(), true);

	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = instance;
	init_info.PhysicalDevice = physicalDevice;
	init_info.Device = device;
	init_info.Queue = graphicsQueue;
	init_info.DescriptorPool = descriptorPool;
	init_info.MinImageCount = 3;
	init_info.ImageCount = 3;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	ImGui_ImplVulkan_Init(&init_info, imguiRenderPass);

	// Upload Fonts
	VkCommandBuffer fontCmdBuffer = beginSingleTimeCommands();
	ImGui_ImplVulkan_CreateFontsTexture(fontCmdBuffer);
	endSingleTimeCommands(fontCmdBuffer);

	//TEMP: Should make own key codes
	//io.KeyMap[ImGuiKey_]
}

void Renderer::initScene()
{
	RenderObject teapot1;
	teapot1.transformMatrix = glm::mat4(1.0f);

	RenderObject teapot2;
	teapot2.transformMatrix = glm::translate(glm::mat4{1.0f}, glm::vec3(4.0f, 0.0f, 0.0f));

	objects.push_back(teapot1);
	objects.push_back(teapot2);
}

void Renderer::createVulkanInstance()
{
	if (enableValidationLayers && !checkValidationLayerSupport())
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
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	// Get GLFW extensions
	std::vector<const char*> glfwExtensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(glfwExtensions.size());
	createInfo.ppEnabledExtensionNames = glfwExtensions.data();

	VkResult res = vkCreateInstance(&createInfo, nullptr, &instance);

	// Create Vulkan instance
	if (res != VK_SUCCESS)
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
	/*printf("\nAvailable Extensions:\n");
	for (uint32_t i = 0; i < extensions.size(); i++)
	{
		printf("\t%s\n", extensions.at(i).extensionName);
	}*/
}

void Renderer::createSwapChain()
{
	if(swapChain)
		oldSwapChain = swapChain;

	// Select swap chain settings
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
	VkSurfaceFormatKHR surfaceFormat = chooseSwapChainFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &swapChainSupport.capabilities);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		imageCount = swapChainSupport.capabilities.maxImageCount;

	// Set swapchain info
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // can use VK_IMAGE_USAGE_TRANSFER_DST_BIT for post processing

	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // image is owned by one queue family at a time
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // image can be used across multiple queue families
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = oldSwapChain; //== nullptr ? VK_NULL_HANDLE : oldSwapChain = swapChain;

	// create swap chain
	VkResult result = vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain);
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to create swap chain!");

	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

	swapChainImageFormat = surfaceFormat.format;
	swapChainImageExtent = extent;
}

bool Renderer::checkValidationLayerSupport()
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

		if (!layerFound)
			return false;
	}

	return true;
}

void Renderer::setupDebugMessenger()
{
	if (!enableValidationLayers)
		return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	populateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to set up debug messenger!");
	}
}

void Renderer::deviceWaitIdle()
{
	vkDeviceWaitIdle(device);
}

void Renderer::setRenderMode(RenderMode mode)
{
	renderMode = mode;
}

void Renderer::compileShaders()
{
	// TODO: Shift this to the actual proper API call
	system("CompileShaders.bat");
}

void Renderer::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	Renderer* app = reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(window));
	app->framebufferResized = true;
}

std::vector<char> Renderer::readBinaryFile(const std::string& filename)
{
	// start reading at end of file and read as a binary file
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	if (!file.is_open())
		throw std::runtime_error("Failed to open file!");

	size_t fileSize = (size_t)file.tellg(); // Get file size from read position
	std::vector<char> buffer(fileSize);

	file.seekg(0); // Return to start of file
	file.read(buffer.data(), fileSize); // Read file
	file.close();

	return buffer;
}

void Renderer::DrawVec3Control(const char* label, glm::vec3& values, float resetValue, float columnWidth)
{
	ImGui::PushID(label);

	ImGui::Columns(2);

	ImGui::SetColumnWidth(0, columnWidth);
	ImGui::Text(label);
	ImGui::NextColumn();

	ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0});

	float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
	ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.9f, 0.2f, 0.2f, 1.0f});
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
	if(ImGui::Button("X", buttonSize))
		values.x = resetValue;
	ImGui::PopStyleColor(3);
	
	ImGui::SameLine();
	ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.3f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.3f, 1.0f });
	if (ImGui::Button("Y", buttonSize))
		values.y = resetValue;
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
	if (ImGui::Button("Z", buttonSize))
		values.z = resetValue;
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();

	ImGui::PopStyleVar();

	ImGui::Columns(1);
	ImGui::PopID();
}

void Renderer::DrawFloatControl(const char* label, float& value, float resetValue, float columnWidth)
{
	ImGui::PushID(label);

	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, columnWidth);
	ImGui::Text(label);
	ImGui::NextColumn();

	ImGui::PushMultiItemsWidths(1, ImGui::CalcItemWidth());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

	float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
	ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
	if (ImGui::Button("", buttonSize))
		value = resetValue;
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	ImGui::DragFloat("##", &value, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();

	ImGui::PopItemWidth();
	ImGui::PopStyleVar();

	ImGui::Columns(2);
	ImGui::PopID();
}

void Renderer::createImageViews()
{
	swapChainImageViews.resize(swapChainImages.size());
	for (int i = 0; i < swapChainImages.size(); i++)
	{
		swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}
}

void Renderer::createLogicalDevice()
{
	QueueFamilyIndices indicies = findQueueFamilies(physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indicies.graphicsFamily.value(), indicies.presentFamily.value() };

	// Set queue priority
	float queuePriority = 1.0f; // must be between 0.0 and 1.0f

	// Specify queues to be created
	std::set<uint32_t>::iterator it;
	for (it = uniqueQueueFamilies.begin(); it != uniqueQueueFamilies.end(); ++it)
	{

		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = *it;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	// Specify device features
	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	deviceFeatures.fillModeNonSolid = VK_TRUE;

	// Create logical device info
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();
	createInfo.pEnabledFeatures = &deviceFeatures;

	// Set device specific validation layers
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
	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
		throw std::runtime_error("Failed to create logical device!");

	// Get queue handles
	vkGetDeviceQueue(device, indicies.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indicies.presentFamily.value(), 0, &presentQueue);
}

void Renderer::createSurface()
{
	if (glfwCreateWindowSurface(instance, window->getWindow(), nullptr, &surface) != VK_SUCCESS)
		throw std::runtime_error("Failed to create window surface!");
}

void Renderer::createGraphicsPipeline()
{
	std::vector<char> vertShaderCode = readBinaryFile("MainApp/resources/vulkan/shaders/TestVert.spv");
	std::vector<char> fragShaderCode = readBinaryFile("MainApp/resources/vulkan/shaders/TestFrag.spv");

	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

	gPipeline = Pipeline(device);
	gPipeline.createDefaultPipeline(vertShaderModule, fragShaderModule, &renderPass, &pipelineLayout, &descriptorSetLayout, &swapChainImageExtent);

	std::vector<char> wireframeVertShaderCode = readBinaryFile("MainApp/resources/vulkan/shaders/WireframeVert.spv");
	std::vector<char> wireframeFragShaderCode = readBinaryFile("MainApp/resources/vulkan/shaders/WireframeFrag.spv");

	VkShaderModule wireframeVertShaderModule = createShaderModule(wireframeVertShaderCode);
	VkShaderModule wireframeFragShaderModule = createShaderModule(wireframeFragShaderCode);

	wireframePipeline = Pipeline(device);
	wireframePipeline.setPolygonMode(VK_POLYGON_MODE_LINE);
	//wireframePipeline.setVertexAttributeCount(1);
	wireframePipeline.createDefaultPipeline(wireframeVertShaderModule, wireframeFragShaderModule, &renderPass, &pipelineLayout, &descriptorSetLayout, &swapChainImageExtent);
	//wireframePipeline.createDefaultPipeline(vertShaderModule, fragShaderModule, &renderPass, &pipelineLayout, &descriptorSetLayout, &swapChainImageExtent);

	vkDestroyShaderModule(device, fragShaderModule, nullptr);
	vkDestroyShaderModule(device, vertShaderModule, nullptr);
	vkDestroyShaderModule(device, wireframeVertShaderModule, nullptr);
	vkDestroyShaderModule(device, wireframeFragShaderModule, nullptr);
}

void Renderer::createRenderPass()
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // Can be changed for multisampling
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Clear values to a constant before rendering (clear frame buffer)
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // Rendered contents will be stored in memory and can be read later
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// Subpasses
	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = findDepthFormat();
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;	// Data will not be used after drawing is finished
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef; // See https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Render_passes for more attachments
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create render pass!");
	}
}

void Renderer::createImguiRenderPass()
{
	VkAttachmentDescription imguiColorAttachment{};
	imguiColorAttachment.format = swapChainImageFormat;
	imguiColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	imguiColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	imguiColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	imguiColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	imguiColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	imguiColorAttachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	imguiColorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// Subpasses
	VkAttachmentReference imguiColorAttachmentRef{};
	imguiColorAttachmentRef.attachment = 0;
	imguiColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription imguiSubpass{};
	imguiSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	imguiSubpass.colorAttachmentCount = 1;
	imguiSubpass.pColorAttachments = &imguiColorAttachmentRef;

	VkSubpassDependency imguiDependency{};
	imguiDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	imguiDependency.dstSubpass = 0;
	imguiDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	imguiDependency.srcAccessMask = 0;
	imguiDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	imguiDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo imguiRenderPassInfo{};
	imguiRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	imguiRenderPassInfo.attachmentCount = 1;
	imguiRenderPassInfo.pAttachments = &imguiColorAttachment;
	imguiRenderPassInfo.subpassCount = 1;
	imguiRenderPassInfo.pSubpasses = &imguiSubpass;
	imguiRenderPassInfo.dependencyCount = 1;
	imguiRenderPassInfo.pDependencies = &imguiDependency;

	if (vkCreateRenderPass(device, &imguiRenderPassInfo, nullptr, &imguiRenderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create render pass!");
	}
}

void Renderer::createFrameBuffers()
{
	swapChainFramebuffers.resize(swapChainImageViews.size());
	imguiFramebuffers.resize(swapChainImageViews.size());

	VkResult res;

	for (size_t i = 0; i < swapChainImageViews.size(); i++)
	{
		std::array<VkImageView, 2> attachments = {
			swapChainImageViews[i],
			depthImageView
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = swapChainImageExtent.width;
		framebufferInfo.height = swapChainImageExtent.height;
		framebufferInfo.layers = 1;

		res = vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]);

		if (res != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create framebuffer!");
		}
	}
}

void Renderer::createCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	poolInfo.flags = 0; // Optional

	if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create command pool!");
	}

	VkCommandPoolCreateInfo imguiPoolInfo{};
	imguiPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	imguiPoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	imguiPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (vkCreateCommandPool(device, &imguiPoolInfo, nullptr, &imguiCommandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create imgui command pool!");
	}
}

void Renderer::createCommandBuffers()
{
	commandBuffers.resize(swapChainFramebuffers.size());
	wireframeCommandBuffers.resize(swapChainFramebuffers.size());
	imguiCommandBuffers.resize(swapChainFramebuffers.size());

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

	//createCommandBuffer(commandBuffers, allocInfo, gPipeline, descriptorSets);

	if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate command buffers!");
	}

	for (size_t i = 0; i < commandBuffers.size(); i++)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapChainFramebuffers[i];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChainImageExtent;

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		gPipeline.bindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS);

		VkBuffer vertexbuffers[] = { vertexBuffer.buffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexbuffers, offsets);

		vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
		
		/*for (size_t j = 0; j < objects.size(); j++)
		{
			vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, gPipeline.getPipelineLayout(), 0, 1, &descriptorSets[i], 0, nullptr);
			vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
		}*/

		vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, gPipeline.getPipelineLayout(), 0, 1, &descriptorSets[i], 0, nullptr);
		vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

		//vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), INSTANCE_COUNT, 0, 0, 0);
		vkCmdEndRenderPass(commandBuffers[i]);

		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to record command buffer!");
		}
	}

	VkCommandBufferAllocateInfo wireframeAllocInfo{};
	wireframeAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	wireframeAllocInfo.commandPool = commandPool;
	wireframeAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	wireframeAllocInfo.commandBufferCount = (uint32_t)wireframeCommandBuffers.size();

	//createCommandBuffer(wireframeCommandBuffers, wireframeAllocInfo, wireframePipeline, descriptorSets);

	if (vkAllocateCommandBuffers(device, &wireframeAllocInfo, wireframeCommandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate command buffers!");
	}

	for (size_t i = 0; i < wireframeCommandBuffers.size(); i++)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(wireframeCommandBuffers[i], &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapChainFramebuffers[i];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChainImageExtent;

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(wireframeCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		wireframePipeline.bindPipeline(wireframeCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS);
		vkCmdBindDescriptorSets(wireframeCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, wireframePipeline.getPipelineLayout(), 0, 1, &descriptorSets[i], 0, nullptr);

		VkBuffer vertexbuffers[] = { vertexBuffer.buffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(wireframeCommandBuffers[i], 0, 1, vertexbuffers, offsets);

		vkCmdBindIndexBuffer(wireframeCommandBuffers[i], indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(wireframeCommandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
		vkCmdEndRenderPass(wireframeCommandBuffers[i]);

		if (vkEndCommandBuffer(wireframeCommandBuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to record command buffer!");
		}
	}

	VkCommandBufferAllocateInfo imguiAllocInfo{};
	imguiAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	imguiAllocInfo.commandPool = imguiCommandPool;
	imguiAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	imguiAllocInfo.commandBufferCount = (uint32_t)imguiCommandBuffers.size();

	//createCommandBuffer(commandBuffers, allocInfo, gPipeline, descriptorSets);

	if (vkAllocateCommandBuffers(device, &imguiAllocInfo, imguiCommandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate command buffers!");
	}
}

void Renderer::createCommandBuffer(std::vector<VkCommandBuffer>& buffers, VkCommandBufferAllocateInfo allocInfo, 
								   Pipeline pipeline, std::vector<VkDescriptorSet>& descSets)
{
	if (vkAllocateCommandBuffers(device, &allocInfo, buffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate command buffers!");
	}

	for (size_t i = 0; i < buffers.size(); i++)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(buffers[i], &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapChainFramebuffers[i];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChainImageExtent;

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(buffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		gPipeline.bindPipeline(buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS);
		vkCmdBindDescriptorSets(buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getPipelineLayout(), 0, 1, &descSets[i], 0, nullptr);

		VkBuffer vertexbuffers[] = { vertexBuffer.buffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(buffers[i], 0, 1, vertexbuffers, offsets);

		vkCmdBindIndexBuffer(buffers[i], indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(buffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
		vkCmdEndRenderPass(buffers[i]);

		if (vkEndCommandBuffer(buffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to record command buffer!");
		}
	}
}

void Renderer::createSyncObjects()
{
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;


	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
		{

			throw std::runtime_error("Failed to create semaphores for a frame!");
		}
	}
}

std::vector<const char*> Renderer::getRequiredExtensions()
{
	// Get the number of required extensions
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	// Allocate extensions array
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers)
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	return extensions;
}

void Renderer::selectPhysicalDevice()
{
	// Get device count
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	// Check if there are any devices that support Vulkan
	if (deviceCount == 0)
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

	if (physicalDevice == VK_NULL_HANDLE)
		throw std::runtime_error("Failed to find a compatible GPU!");
}

bool Renderer::isDeviceSuitable(VkPhysicalDevice device)
{
	// Can be extended later (see https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Physical_devices_and_queue_families)
	QueueFamilyIndices indicies = findQueueFamilies(device);

	bool extensionSupported = checkDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionSupported)
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);	// Rather than forcing anisotropy could make it conditional

	return indicies.isComplete() && extensionSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

bool Renderer::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (int i = 0; i < availableExtensions.size(); i++)
	{
		char* extensionName = availableExtensions.at(i).extensionName;
		requiredExtensions.erase(extensionName);
	}

	return requiredExtensions.empty();
}

QueueFamilyIndices Renderer::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indicies;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	for (uint32_t i = 0; i < queueFamilies.size(); i++)
	{
		if (queueFamilies.at(i).queueFlags & VK_QUEUE_GRAPHICS_BIT)
			indicies.graphicsFamily = i;

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
		if (presentSupport)
			indicies.presentFamily = i;

		if (indicies.isComplete())
			break;
	}

	return indicies;
}

SwapChainSupportDetails Renderer::querySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

VkSurfaceFormatKHR Renderer::chooseSwapChainFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (int i = 0; i < availableFormats.size(); i++)
	{
		VkSurfaceFormatKHR currentFormat = availableFormats.at(i);
		if (currentFormat.format == VK_FORMAT_B8G8R8A8_SRGB && currentFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return currentFormat;
	}

	return availableFormats.at(0);
}

VkPresentModeKHR Renderer::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (int i = 0; i < availablePresentModes.size(); i++)
	{
		VkPresentModeKHR currentPresentMode = availablePresentModes.at(i);
		if (currentPresentMode == VK_PRESENT_MODE_FIFO_KHR)
			return currentPresentMode;
		else if (currentPresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return currentPresentMode;
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Renderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != UINT32_MAX)
	{
		return capabilities.currentExtent;
	}
	else
	{
		int extentWidth, extentHeight;
		window->getFrameBufferSize(&extentWidth, &extentHeight);

		VkExtent2D actualExtent = { static_cast<uint32_t>(extentWidth), static_cast<uint32_t>(extentHeight) };
		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

VkShaderModule Renderer::createShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();// * sizeof(uint32_t);
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		throw std::runtime_error("Failed tp create shader module!");

	return shaderModule;
}

void Renderer::createDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // Which stage this descriptor is going to be referenced
	uboLayoutBinding.pImmutableSamplers = nullptr; // Used for image sampling

	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding lightLayoutBinding{};
	lightLayoutBinding.binding = 2;
	lightLayoutBinding.descriptorCount = 1;	// TODO: maybe the count of the lights?
	lightLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	lightLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	lightLayoutBinding.pImmutableSamplers = nullptr;

	std::array<VkDescriptorSetLayoutBinding, 3> bindings = { uboLayoutBinding, samplerLayoutBinding, lightLayoutBinding };
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	VkResult result = vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout);

	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to create descriptor set layout!");
}

void Renderer::createUniformBuffers()
{
	uniformBuffers.resize(swapChainImages.size());
	lightUniformBuffers.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		uniformBuffers[i].createUniformBuffer(device, physicalDevice, commandPool, graphicsQueue, sizeof(UniformBufferObject));
		lightUniformBuffers[i].createUniformBuffer(device, physicalDevice, commandPool, graphicsQueue, sizeof(LightUniformBufferObject));
	}
}

void Renderer::createDescriptorPool()
{
	std::array<VkDescriptorPoolSize, 2> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(swapChainImages.size()) * 1000;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(swapChainImages.size()) * 1000;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = 1000 * static_cast<uint32_t>(swapChainImages.size());

	VkResult result = vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool);
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to create descriptor pool!");
}

void Renderer::createDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(swapChainImages.size(), descriptorSetLayout);

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets.resize(swapChainImages.size());
	VkResult result = vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data());
	if (result != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate descriptor sets!");

	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = uniformBuffers[i].buffer;
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textureImageView;
		imageInfo.sampler = textureSampler;

		VkDescriptorBufferInfo lightBufferInfo{};
		lightBufferInfo.buffer = lightUniformBuffers[i].buffer;
		lightBufferInfo.offset = 0;
		lightBufferInfo.range = sizeof(LightUniformBufferObject);

		std::array<VkWriteDescriptorSet, 3> descriptorWrites{};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;
		descriptorWrites[0].pImageInfo = nullptr; // Optional
		descriptorWrites[0].pTexelBufferView = nullptr; // Optional
		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pBufferInfo = nullptr;
		descriptorWrites[1].pImageInfo = &imageInfo;
		descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[2].dstSet = descriptorSets[i];
		descriptorWrites[2].dstBinding = 2;
		descriptorWrites[2].dstArrayElement = 0;
		descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[2].descriptorCount = 1;
		descriptorWrites[2].pBufferInfo = &lightBufferInfo;
		descriptorWrites[2].pImageInfo = nullptr; // Optional
		descriptorWrites[2].pTexelBufferView = nullptr; // Optional
		

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

void Renderer::updateUniformBuffer(uint32_t currentImage, float dt, uint32_t objIndex)
{
	mainCamera.updateModel(dt);

	UniformBufferObject ubo{};
	ubo.model = objects[objIndex].transformMatrix;//glm::mat4(1.0f);
	ubo.view = mainCamera.invModel;
	ubo.proj = glm::perspective(glm::radians(mainCamera.fov), swapChainImageExtent.width / (float)swapChainImageExtent.height, 0.1f, 500.0f);
	ubo.proj[1][1] *= -1;
	ubo.mvp = ubo.proj * ubo.view * ubo.model;
	ubo.normalModel = glm::transpose(glm::inverse(ubo.model));
	ubo.mv = ubo.view * ubo.model;

	void* data;
	vkMapMemory(device, uniformBuffers[currentImage].bufferMemory, 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(device, uniformBuffers[currentImage].bufferMemory);

	light.updateModel();

	LightUniformBufferObject lightUbo{};	// TODO: get light position data from vertex buffer
	//lightUbo.pointLights[0] = light;
	lightUbo.model = light.model;
	lightUbo.cameraPos = mainCamera.position;
	lightUbo.ambientColor = light.diffuse;//glm::vec3(1.0f);
	lightUbo.ambientIntensity = light.intensity;//1.0f;
	lightUbo.pointLights = light;
	printf("%f, %f, %f\n", lightUbo.pointLights.pos.x, lightUbo.pointLights.pos.y, lightUbo.pointLights.pos.z);

	void* lightData;
	vkMapMemory(device, lightUniformBuffers[currentImage].bufferMemory, 0, sizeof(lightUbo), 0, &lightData);
	memcpy(lightData, &lightUbo, sizeof(lightUbo));
	vkUnmapMemory(device, lightUniformBuffers[currentImage].bufferMemory);
}

void Renderer::cleanupSwapChain()
{
	vkDestroyImageView(device, depthImageView, nullptr);
	vkDestroyImage(device, depthImage, nullptr);
	vkFreeMemory(device, depthImageMemory, nullptr);

	for (int i = 0; i < swapChainFramebuffers.size(); i++)
	{
		vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
	}

	for (int j = 0; j < imguiFramebuffers.size(); j++) 
	{
		vkDestroyFramebuffer(device, imguiFramebuffers[j], nullptr);
	}

	vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
	vkFreeCommandBuffers(device, imguiCommandPool, static_cast<uint32_t>(imguiCommandBuffers.size()), imguiCommandBuffers.data());

	gPipeline.destroyPipeline();
	wireframePipeline.destroyPipeline();
	vkDestroyRenderPass(device, renderPass, nullptr);
	vkDestroyRenderPass(device, imguiRenderPass, nullptr);

	for (int i = 0; i < swapChainImageViews.size(); i++)
	{
		vkDestroyImageView(device, swapChainImageViews[i], nullptr);
	}

	vkDestroySwapchainKHR(device, swapChain, nullptr);

	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		uniformBuffers[i].destroyBuffer(device);
		uniformBuffers[i].freeBufferMemory(device);
	}

	vkDestroyDescriptorPool(device, descriptorPool, nullptr);
}

void Renderer::recreateSwapChain()
{	
	// Check if window is minimized
	int width = 0, height = 0;
	window->getFrameBufferSize(&width, &height);
	while (width == 0 || height == 0)
	{
		window->getFrameBufferSize(&width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(device);

	createSwapChain();
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createDepthResources();
	createFrameBuffers();
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();
	createCommandBuffers();

	imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);
}

void Renderer::prepareInstanceData()
{
	std::vector<InstanceData> instanceData;
	instanceData.resize(INSTANCE_COUNT);

	instanceData[0].pos = glm::vec3(0.0f, 0.0f, 0.0f);
	instanceData[0].rot = glm::vec3(0.0f, 0.0f, 0.0f);
	instanceData[0].pos = glm::vec3(1.0f, 1.0f, 1.0f);

	instanceData[1].pos = glm::vec3(0.0f, 4.0f, 0.0f);
	instanceData[1].rot = glm::vec3(0.0f, 0.0f, 0.0f);
	instanceData[1].pos = glm::vec3(1.0f, 1.0f, 1.0f);

	instanceBuffer.size = instanceData.size() * sizeof(InstanceData);

	struct
	{
		VkDeviceMemory memory = VK_NULL_HANDLE;
		VkBuffer buffer = VK_NULL_HANDLE;
	} stagingBuffer;

	VkBufferCreateInfo bufferInfo{};

	VkResult res;

	res = VertexBuffer::createBuffer(device, physicalDevice, instanceBuffer.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
								stagingBuffer.buffer, stagingBuffer.memory, bufferInfo);

	res = VertexBuffer::createBuffer(device, physicalDevice, instanceBuffer.size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		instanceBuffer.buffer, instanceBuffer.memory, bufferInfo);
	
	VkCommandBuffer copyCmd = beginSingleTimeCommands();

	VkBufferCopy copyRegion = {};
	copyRegion.size = instanceBuffer.size;
	vkCmdCopyBuffer(copyCmd, stagingBuffer.buffer, instanceBuffer.buffer, 1, &copyRegion);

	res = vkEndCommandBuffer(copyCmd);
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &copyCmd;

	// Create fence to ensure that the command buffer has finished executing
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = 0;
	VkFence fence;
	vkCreateFence(device, &fenceCreateInfo, nullptr, &fence);

	// Submit to the queue
	vkQueueSubmit(graphicsQueue, 1, &submitInfo, fence);
	// Wait for the fence to signal that command buffer has finished executing
	vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);

	vkDestroyFence(device, fence, nullptr);
	vkFreeCommandBuffers(device, commandPool, 1, &copyCmd);

	instanceBuffer.descriptor.range = instanceBuffer.size;
	instanceBuffer.descriptor.buffer = instanceBuffer.buffer;
	instanceBuffer.descriptor.offset = 0;

	// Destroy staging resources
	vkDestroyBuffer(device, stagingBuffer.buffer, nullptr);
	vkFreeMemory(device, stagingBuffer.memory, nullptr);
}

void Renderer::loadModel()
{
	// Attribute struct, holds vertices, weights, normals, texcoords, etc
	tinyobj::attrib_t attributes;
	std::vector<tinyobj::shape_t> shapes;	// I don't entirely understand the purpose of these yet  but it seems to be a seperate storage container for individual objects in an obj file?
	std::vector<tinyobj::material_t> materials;	// .obj files can define specific materials per face, we currently ignore this
	std::string warn, err;

	// TinyObjectLoader has MTL compatibility
	// 	   TODO:: implement MTL loading via TOL
	// if(!tinyobj::LoadMtl())
	if (!tinyobj::LoadObj(&attributes, &shapes, &materials, &warn, &err, modelPath.c_str()))
	{
		throw std::runtime_error(warn + err);
	}

	// The load function already triangulates faces; at this point it can be assumed that there are 3 vertices on each face
	for (std::vector<tinyobj::shape_t>::iterator shapeIter = shapes.begin(); shapeIter != shapes.end(); shapeIter++)
	{
		for (const tinyobj::index_t index : shapeIter._Ptr->mesh.indices)	// I would like to not be using foreach but good lord the types
		{
			Vertex vertex{};
			
			vertex.pos = {
				attributes.vertices[3 * index.vertex_index + 0],
				attributes.vertices[3 * index.vertex_index + 1],
				attributes.vertices[3 * index.vertex_index + 2]
			};

			vertex.vertexNormal = {
				attributes.normals[3 * index.normal_index + 0],
				attributes.normals[3 * index.normal_index + 1],
				attributes.normals[3 * index.normal_index + 2]
			};

			vertex.texCoord = {
				attributes.texcoords[2 * index.texcoord_index + 0],
				attributes.texcoords[2 * index.texcoord_index + 1]
			};

			vertex.color = { 1.0f, 1.0f, 1.0f };

			// We are assuming that each vertex is unique here
			// TODO: account for duplicate vertices
			vertices.push_back(vertex);
			indices.push_back((uint32_t)indices.size());
		}
	}
}

void Renderer::createTextureImage()
{
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	//stbi_uc* pixels = stbi_load("MainApp/resources/vulkan/textures/bricks/Bricks_basecolor.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);


	// RBGA * area values
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels)
	{
		throw std::runtime_error("Failed to load texture image");
	}

	// Create buffer to transfer pixel data
	Buffer stagingBuffer;
	Buffer::createBuffer(device, physicalDevice, imageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer.buffer, stagingBuffer.bufferMemory, stagingBuffer.bufferInfo);

	// Copy pixel values from image loading library to staging buffer
	void* data;
	vkMapMemory(device, stagingBuffer.bufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(device, stagingBuffer.bufferMemory);

	// Clean up image loading
	stbi_image_free(pixels);

	Image::createImage(device, physicalDevice, texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

	// Copy the staging buffer to the new image | Can transition from layout_undefined because we don't care about the contents before the copy operation
	Image::transitionImageLayout(graphicsQueue, device, commandPool, textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	Image::copyBufferToImage(graphicsQueue, device, commandPool, stagingBuffer.buffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

	// Transition image to format for shader read access
	Image::transitionImageLayout(graphicsQueue, device, commandPool, textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	// Cleanup
	vkDestroyBuffer(device, stagingBuffer.buffer, nullptr);
	vkFreeMemory(device, stagingBuffer.bufferMemory, nullptr);
}

void Renderer::createTextureImageView()
{
	textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

VkImageView Renderer::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create texture image view");
	}

	return imageView;
}

void Renderer::createDepthResources()
{
	VkFormat depthFormat = findDepthFormat();

	Image::createImage(device, physicalDevice, swapChainImageExtent.width, swapChainImageExtent.height, depthFormat,
		VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
	depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

	Image::transitionImageLayout(graphicsQueue, device, commandPool, depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

VkFormat Renderer::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates)
	{
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &properties);

		if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features)
		{
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features)
		{
			return format;
		}
	}

	throw std::runtime_error("Failed to find supported format");
}

// Returns the most desirable depth format supported by the physical device
VkFormat Renderer::findDepthFormat()
{
	return findSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

bool Renderer::hasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkCommandBuffer Renderer::beginSingleTimeCommands()
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	VkResult res = vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	res = vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void Renderer::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);

	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void Renderer::drawFrame(float dt)
{
	VkResult res;

	drawImGui();
	
	vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	res = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
	if (res == VK_ERROR_OUT_OF_DATE_KHR && canResizeWindow)
	{
		framebufferResized = false;
		recreateSwapChain();
		ImGui_ImplVulkan_SetMinImageCount((uint32_t)swapChainImages.size());
		return;
	}
	else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR)
		throw std::runtime_error("Failed to acquire swap chain image!");

	if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
		vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);

	{
		VkCommandBufferBeginInfo info{};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		res = vkBeginCommandBuffer(imguiCommandBuffers[imageIndex], &info);
	}

	{
		VkRenderPassBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		info.renderPass = imguiRenderPass;
		info.framebuffer = imguiFramebuffers[imageIndex];
		info.renderArea.extent.width = swapChainImageExtent.width;
		info.renderArea.extent.height = swapChainImageExtent.height;
		info.clearValueCount = 1;
		info.pClearValues = &clearValue;
		vkCmdBeginRenderPass(imguiCommandBuffers[imageIndex], &info, VK_SUBPASS_CONTENTS_INLINE);
	}

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), imguiCommandBuffers[imageIndex]);
	vkCmdEndRenderPass(imguiCommandBuffers[imageIndex]);
	res = vkEndCommandBuffer(imguiCommandBuffers[imageIndex]);

	/*for (uint32_t i = 0; i < objects.size(); i++)
	{
		updateUniformBuffer(imageIndex, dt, i);
	}*/

	updateUniformBuffer(imageIndex, dt, 0);
		

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	std::array<VkCommandBuffer, 2> submitCommandBuffers = {commandBuffers[imageIndex], imguiCommandBuffers[imageIndex]};

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = static_cast<uint32_t>(submitCommandBuffers.size());

	switch (renderMode)
	{
	case DEFAULT_LIT:
		submitCommandBuffers[0] = commandBuffers[imageIndex];
		break;
	case WIREFRAME:
		submitCommandBuffers[0] = wireframeCommandBuffers[imageIndex];
		break;
	}
	
	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	submitInfo.pCommandBuffers = submitCommandBuffers.data();

	vkResetFences(device, 1, &inFlightFences[currentFrame]);

	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr; // Optional

	res = vkQueuePresentKHR(presentQueue, &presentInfo);
	if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || framebufferResized)
	{
		framebufferResized = false;
		recreateSwapChain();
	}
	else if (res != VK_SUCCESS)
		throw std::runtime_error("Failed to acquire swap chain image!");

	//vkQueueWaitIdle(presentQueue);

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::drawImGui()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	//ImGui::ShowDemoWindow();

	bool debugActive = true;
	ImGui::Begin("Debug Info", &debugActive, ImGuiWindowFlags_MenuBar);

	ImGui::Text("FPS: %f /", currentFramerate);
	ImGui::SameLine();
	ImGui::Text("%f ms", currentFrametime);

	ImGui::NewLine();

	ImGui::Text("Current Render Mode:");
	ImGui::SameLine();
	switch (renderMode)
	{
	case DEFAULT_LIT:
		ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.0f, 1.0f), "Default Lit");
		break;
	case WIREFRAME:
		ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.8f, 1.0f), "Wireframe");
		break;
	}

	ImGui::NewLine();

	if (ImGui::CollapsingHeader("Scene"))
	{
		if (ImGui::TreeNode("Camera"))
		{
			DrawVec3Control("Camera Position", mainCamera.position, 0.0f, 120.0f);
			DrawVec3Control("Camera Rotation", mainCamera.rotation, 0.0f, 120.0f);

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Teapot"))
		{
			ImGui::Text("TeapotInfo");
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Light"))
		{
			DrawVec3Control("Light Position", light.pos, 0.0f, 120.0f);

			ImGui::PushID("Light Color");

			ImGui::Columns(2);

			ImGui::SetColumnWidth(0, 120.0f);
			ImGui::Text("Light Color");
			ImGui::NextColumn();

			ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

			float col[4] = { light.diffuse.r, light.diffuse.g, light.diffuse.b, light.diffuse.a };
			ImGui::ColorEdit4("##Light Color", col, ImGuiColorEditFlags_NoInputs);
			light.diffuse = glm::vec4(col[0], col[1], col[2], col[3]);

			ImGui::PopStyleVar();

			ImGui::Columns(1);
			ImGui::PopID();

			DrawFloatControl("Light Intensity", light.intensity, 1.0f, 120.0f);

			ImGui::TreePop();
		}
	}

	ImGui::End();
	ImGui::Render();
}

void Renderer::cleanup()
{
	cleanupSwapChain();

	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

	vkDestroySampler(device, textureSampler, nullptr);
	vkDestroyImageView(device, textureImageView, nullptr);
	vkDestroyImage(device, textureImage, nullptr);
	vkFreeMemory(device, textureImageMemory, nullptr);

	vertexBuffer.destroyBuffer(device);
	vertexBuffer.freeBufferMemory(device);
	indexBuffer.destroyBuffer(device);
	indexBuffer.freeBufferMemory(device);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(device, inFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(device, commandPool, nullptr);
	vkDestroyCommandPool(device, imguiCommandPool, nullptr);

	vkDestroyDevice(device, nullptr);

	if (enableValidationLayers)
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);

	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);

	//glfwDestroyWindow(window);
	//window->cleanupWindow();
	//glfwTerminate();
}

void Renderer::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr; // Optional
}