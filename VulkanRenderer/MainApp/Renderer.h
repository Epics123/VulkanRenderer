#ifndef RENDERER_H
#define RENDERER_H

#define GLFW_INCLUDE_VULKAN

#include <glfw3.h>
#include <glfw3native.h>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>
#include <cassert>
#include <memory>

#include "VertexBuffer.h"
#include "Image.h"
#include "TextureSampler.h"
#include "Window.h"
#include "Camera.h"
#include "Mesh.h"
#include "SwapChain.h"
#include "Model.h"
#include "GameObject.h"
#include "RenderSystem.h"

//struct QueueFamilyIndices
//{
//	std::optional<uint32_t> graphicsFamily;
//	std::optional<uint32_t> presentFamily;
//
//	bool isComplete()
//	{
//		return graphicsFamily.has_value() && presentFamily.has_value();
//	}
//};
//
//struct SwapChainSupportDetails
//{
//	VkSurfaceCapabilitiesKHR capabilities;
//	std::vector<VkSurfaceFormatKHR> formats;
//	std::vector<VkPresentModeKHR> presentModes;
//};

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

	VkShaderModule createShaderModule(const std::vector<char>& code);

	void createRenderPass();
	void createImguiRenderPass();
	void createFrameBuffers();

	void createCommandPool();
	void createCommandBuffers();
	void freeCommandBuffers();
	void createCommandBuffer(std::vector<VkCommandBuffer>& buffers, VkCommandBufferAllocateInfo allocInfo, Pipeline pipeline, std::vector<VkDescriptorSet>& desc);
	void recordCommandBuffer(int imageIndex);

	void createSyncObjects();
	void createDescriptorSetLayout();
	void createUniformBuffers();
	void createDescriptorPool();
	void createDescriptorSets();

	void updateUniformBuffer(uint32_t currentImage, float dt, uint32_t objIndex);

	void cleanupSwapChain();
	void recreateSwapChain();
	VkRenderPass getSwapChainRenderPass() const { return mSwapChain->getRenderPass(); }

	void prepareInstanceData();

	// Load the model
	void loadGameObjects();

	void uploadMeshData(Mesh& mesh);

	void createTextureImage();
	void createTextureImageView();
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	void createDepthResources();
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	VkFormat findDepthFormat();

	bool hasStencilComponent(VkFormat format);

	VkCommandBuffer beginSingleTimeCommands();

	void endSingleTimeCommands(VkCommandBuffer commandBuffer);

	// Frame Drawing
	VkCommandBuffer beginFrame();
	void drawFrame(float dt);
	void endFrame();
	void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
	void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

	bool isFrameInProgress() const { return frameStarted; }

	int getFrameIndex() const
	{
		assert(frameStarted && "Cannot get frame index while frame is not in progress");
		return currentFrameIndex;
	}

	void drawImGui();

	// Clean up application
	void cleanup();

	void deviceWaitIdle();

	Camera& getActiveCamera() { return mainCamera; }

	VkRenderPass getImguiRenderPass() { return imguiRenderPass; }

	VkInstance getVulkanInstance() { return instance; }

	VkPhysicalDevice getPhysicalDevice() { return physicalDevice; }

	VkDevice getDevice() { return device; }

	VkQueue getGraphicsQueue() { return graphicsQueue; }
	
	VkDescriptorPool* getDescriptorPool() { return &descriptorPool; }

	std::vector<VkCommandBuffer> getCommandBuffers() { return commandBuffers; }
	std::vector<VkCommandBuffer> getImguiCommandBuffers() { return imguiCommandBuffers; }

	VkCommandBuffer getCurrentCommandBuffer() const 
	{ 
		assert(frameStarted && "Cannot get command buffer while frame is not in progress");
		return commandBuffers[currentFrameIndex]; 
	}

	VkCommandPool getCommandPool() { return commandPool; }

	void setRenderMode(RenderMode mode);

	static void compileShaders();

	static std::vector<char> readBinaryFile(const std::string& filename);

	static void DrawVec3Control(const char* label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f);
	static void DrawVec3Control(const char* label, glm::vec4& values, float resetValue = 0.0f, float columnWidth = 100.0f);
	static void DrawFloatControl(const char* label, float& value, float resetValue = 1.0f, float columnWidth = 100.0f);

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
		glm::vec3 position;
		glm::vec3 rotation;
		glm::vec3 scale;
	};

	Renderer(Window* appWindow);

	std::unique_ptr<Window> window;

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
	
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;
	VkPipeline graphicsPipeline;
	VkCommandPool commandPool;
	VkCommandPool imguiCommandPool;
	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<VkCommandBuffer> wireframeCommandBuffers;
	std::vector<VkCommandBuffer> imguiCommandBuffers;
	std::vector<VkDescriptorSet> descriptorSets;

	//Pipeline gPipeline;
	//Pipeline wireframePipeline;
	
	// Rework //
	Device mDevice{*window};
	std::unique_ptr <SwapChain> mSwapChain;
	std::vector<GameObject> gameObjects;
	RenderSystem renderSystem {mDevice};
	////////////

	RenderMode renderMode = DEFAULT_LIT;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	std::vector<VkFence> imagesInFlight;
	size_t currentFrame = 0;
	bool framebufferResized = false;
	bool canResizeWindow = false;

	uint32_t currentImageIndex;
	int currentFrameIndex = 0;
	bool frameStarted = false;

	VertexSTDVector vertices;
	IndexSTDVector indices;

	VkClearColorValue clearColor = { {0.01f, 0.01f, 0.01f, 1.0f} };

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

	Mesh teapotMesh;

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