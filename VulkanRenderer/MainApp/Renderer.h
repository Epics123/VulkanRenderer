#ifndef RENDERER_H
#define RENDERER_H

#define GLFW_INCLUDE_VULKAN

#include <glfw3.h>
#include <glfw3native.h>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>

#include "VertexBuffer.h"
#include "Image.h"
#include "TextureSampler.h"
#include "Window.h"

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete()
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class Renderer
{
public:
	Renderer(Window* appWindow);

	void init();

	// Create Vulkan instance that will interact with the application
	void createVulkanInstance();

	// Create the swap chain
	void createSwapChain();

	// Check if requested validation layers are available
	bool checkValidationLayerSupport();

	// Get required GLFW Extensions
	std::vector<const char*> getRequiredExtensions();

	// Select compatible GPU to use
	void selectPhysicalDevice();

	// Check if device is supported
	bool isDeviceSuitable(VkPhysicalDevice device);

	bool checkDeviceExtensionSupport(VkPhysicalDevice device);

	// Find graphics queue family
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

	VkSurfaceFormatKHR chooseSwapChainFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	VkShaderModule createShaderModule(const std::vector<char>& code);

	void createImageViews();

	void createLogicalDevice();

	void createSurface();

	void createGraphicsPipeline();

	void createRenderPass();

	void createFrameBuffers();

	void createCommandPool();

	void createCommandBuffers();

	void createSyncObjects();

	void createDescriptorSetLayout();

	void createUniformBuffers();

	void createDescriptorPool();

	void createDescriptorSets();

	void updateUniformBuffer(uint32_t currentImage);

	void cleanupSwapChain();

	void recreateSwapChain();

	// Load the model
	void loadModel();

	void createTextureImage();

	void createTextureImageView();

	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

	void createDepthResources();

	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	VkFormat findDepthFormat();

	bool hasStencilComponent(VkFormat format);

	void drawFrame();

	// Clean up application
	void cleanup();

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	void setupDebugMessenger();

	void deviceWaitIdle();

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

	static std::vector<char> readBinaryFile(const std::string& filename);

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

private:
	Window* window;

	std::string modelPath;
	std::string texturePath;

	VkInstance instance;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE; // GPU
	VkDevice device; // Logical device
	VkQueue graphicsQueue; // Drawing queue
	VkQueue presentQueue; // Presentation queue
	VkSurfaceKHR surface;
	VkDebugUtilsMessengerEXT debugMessenger;

	// Swap Chain
	VkSwapchainKHR swapChain;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainImageExtent;
	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;
	std::vector<VkFramebuffer> swapChainFramebuffers;

	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;
	VkPipeline graphicsPipeline;
	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<VkDescriptorSet> descriptorSets;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	std::vector<VkFence> imagesInFlight;
	size_t currentFrame = 0;
	bool framebufferResized = false;

	VertexSTDVector vertices;
	IndexSTDVector indices;

	VertexBuffer vertexBuffer;
	IndexBuffer indexBuffer;
	std::vector<UniformBuffer> uniformBuffers;

	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	const std::vector<const char*> validationLayers =
	{
		"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char*> deviceExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	const int MAX_FRAMES_IN_FLIGHT = 2;

#ifdef NDEBUG // not debug
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif
};

#endif // !RENDERER_H