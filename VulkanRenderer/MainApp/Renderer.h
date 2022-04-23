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
#include "Camera.h"
#include "Pipeline.h"

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

enum RenderMode
{
	DEFAULT_LIT,
	WIREFRAME
};

class Renderer
{
public:
	static Renderer* rendererInstance;

	static Renderer* initInstance(Window* window);

	static void cleanupInstance();

	Renderer() = delete;

	Renderer(Renderer const&) = delete;

	void operator=(Renderer const&) = delete;

	void init();

	void imguiInit();

	void initScene();

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

	void createImguiRenderPass();

	void createFrameBuffers();

	void createCommandPool();

	void createCommandBuffers();

	void createCommandBuffer(std::vector<VkCommandBuffer>& buffers, VkCommandBufferAllocateInfo allocInfo, 
							 Pipeline pipeline, std::vector<VkDescriptorSet>& desc);

	void createSyncObjects();

	void createDescriptorSetLayout();

	void createUniformBuffers();

	void createDescriptorPool();

	void createDescriptorSets();

	void updateUniformBuffer(uint32_t currentImage, float dt, uint32_t objIndex);

	void cleanupSwapChain();

	void recreateSwapChain();

	void prepareInstanceData();

	// Load the model
	void loadModel();

	void createTextureImage();

	void createTextureImageView();

	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

	void createDepthResources();

	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	VkFormat findDepthFormat();

	bool hasStencilComponent(VkFormat format);

	VkCommandBuffer beginSingleTimeCommands();

	void endSingleTimeCommands(VkCommandBuffer commandBuffer);

	void drawFrame(float dt);

	void drawImGui();

	// Clean up application
	void cleanup();

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	void setupDebugMessenger();

	void deviceWaitIdle();

	Camera& getActiveCamera() { return mainCamera; }

	Pipeline getActivePipeline() { return gPipeline; }

	VkRenderPass getRenderPass() { return renderPass; }

	VkRenderPass getImguiRenderPass() { return imguiRenderPass; }

	VkInstance getVulkanInstance() { return instance; }

	VkPhysicalDevice getPhysicalDevice() { return physicalDevice; }

	VkDevice getDevice() { return device; }

	VkQueue getGraphicsQueue() { return graphicsQueue; }
	
	VkDescriptorPool* getDescriptorPool() { return &descriptorPool; }

	std::vector<VkCommandBuffer> getCommandBuffers() { return commandBuffers; }

	std::vector<VkCommandBuffer> getImguiCommandBuffers() { return imguiCommandBuffers; }

	VkCommandPool getCommandPool() { return commandPool; }

	void setRenderMode(RenderMode mode);

	static void compileShaders();

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

	static std::vector<char> readBinaryFile(const std::string& filename);

	static void DrawVec3Control(const char* label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f);

	static void DrawFloatControl(const char* label, float& value, float resetValue = 1.0f, float columnWidth = 100.0f);

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

	float currentFramerate;
	float currentFrametime;

	const int INSTANCE_COUNT = 2;

private:
	
	struct InstanceData
	{
		glm::vec3 pos;
		glm::vec3 rot;
		glm::vec3 scale;
	};

	struct InstanceBuffer
	{
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		size_t size = 0;
		VkDescriptorBufferInfo descriptor;
	} instanceBuffer;

	// Data for a renderable object, should expand to have mesh and material references 
	struct RenderObject
	{
		glm::mat4 transformMatrix;
	};

	Renderer(Window* appWindow);

	Window* window;

	Camera mainCamera;

	std::string modelPath;
	std::string texturePath;

	std::vector<RenderObject> objects;
	Light light;

	VkInstance instance;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE; // GPU
	VkDevice device; // Logical device
	VkQueue graphicsQueue; // Drawing queue
	VkQueue presentQueue; // Presentation queue
	VkSurfaceKHR surface;
	VkDebugUtilsMessengerEXT debugMessenger;

	// Swap Chain
	VkSwapchainKHR swapChain = VK_NULL_HANDLE;
	VkSwapchainKHR oldSwapChain = VK_NULL_HANDLE;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainImageExtent;
	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	std::vector<VkFramebuffer> imguiFramebuffers;

	VkRenderPass renderPass;
	VkRenderPass imguiRenderPass;
	VkPipelineLayout pipelineLayout;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;
	VkPipeline graphicsPipeline;
	VkCommandPool commandPool;
	VkCommandPool imguiCommandPool;
	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<VkCommandBuffer> wireframeCommandBuffers;
	std::vector<VkCommandBuffer> imguiCommandBuffers;
	std::vector<VkDescriptorSet> descriptorSets;

	Pipeline gPipeline;
	Pipeline wireframePipeline;

	RenderMode renderMode = DEFAULT_LIT;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	std::vector<VkFence> imagesInFlight;
	size_t currentFrame = 0;
	bool framebufferResized = false;
	bool canResizeWindow = false;

	VertexSTDVector vertices;
	IndexSTDVector indices;

	VkClearValue clearValue = { {0.0f, 0.0f, 0.0f, 1.0f}};

	VertexBuffer vertexBuffer;
	IndexBuffer indexBuffer;
	std::vector<UniformBuffer> uniformBuffers;
	std::vector<UniformBuffer> lightUniformBuffers;

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