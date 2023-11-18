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
#include <unordered_map>

#include "VertexBuffer.h"
#include "Image.h"
#include "TextureSampler.h"
#include "Window.h"
#include "Camera.h"
#include "Mesh.h"
#include "SwapChain.h"
#include "Buffer.h"
#include "Descriptors.h"
#include "Enums.h"
#include "Utils.h"

#include "Scene/Scene.h"

// Render Systems
#include "RenderSystems/RenderSystem.h"
#include "RenderSystems/PointLightSystem.h"
#include "RenderSystems/WireframeSystem.h"
#include "RenderSystems/ImGuiSystem.h"
#include "RenderSystems/UnlitSystem.h"
#include "RenderSystems/WorldGridSystem.h"
#include "RenderSystems/SpotLightSystem.h"
#include "RenderSystems/ShadowSystem.h"

class Renderer
{
public:
	static Renderer* rendererInstance;

	static Renderer* initInstance(Window* window);
	static Renderer* getInstance(){ return rendererInstance; }
	static void cleanupInstance();

	Renderer() = delete;

	Renderer(Renderer const&) = delete;

	void operator=(Renderer const&) = delete;

	void init();
	void imguiInit();

	void createCommandBuffers();
	void freeCommandBuffers();

	Device& getDevice() { return mDevice; }

	void recreateSwapChain();
	RenderPass getSwapChainRenderPass() const { return mSwapChain->getRenderPass(); }

	void loadMaterials(DescriptorSetLayout& layout);
	void cleanupTextures();

	bool hasStencilComponent(VkFormat format);

	// Frame Drawing
	VkCommandBuffer beginFrame();
	void drawFrame(float dt);
	void endFrame();
	VkResult submit(std::vector<VkCommandBuffer> commadBuffers);
	void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
	void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

	bool isFrameInProgress() const { return frameStarted; }

	int getFrameIndex() const
	{
		assert(frameStarted && "Cannot get frame index while frame is not in progress");
		return currentFrameIndex;
	}

	void drawImGui(FrameInfo& frameInfo);

	// Clean up application
	void cleanup();

	void deviceWaitIdle();

	Camera& getActiveCamera() { return mainCamera; }

	VkCommandBuffer getCurrentCommandBuffer() const 
	{ 
		assert(frameStarted && "Cannot get command buffer while frame is not in progress");
		return commandBuffers[currentFrameIndex]; 
	}

	void setRenderMode(RenderMode mode);

	void setShowGrid(bool show) { showGrid = show; }
	bool getShowGrid() { return showGrid; }

	static void compileShaders();

	static std::vector<char> readBinaryFile(const std::string& filename);

	float currentFramerate;
	float currentFrametime;

	std::vector<Texture> textures;

private:
	Renderer(Window* appWindow);

	std::unique_ptr<Window> window;

	Camera mainCamera;

	std::string modelPath;
	std::string texturePath;
	
	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<VkCommandBuffer> shadowCommandBuffers;
	
	Device mDevice{*window};
	std::unique_ptr <SwapChain> mSwapChain;

	std::unique_ptr<DescriptorPool> globalDescriptorPool{};
	std::unique_ptr<DescriptorPool> imguiDescriptorPool{};
	std::vector<VkDescriptorSet> globalDescriptorSets;
	std::vector<VkDescriptorSet> materialDescriptorSets;
	std::vector<std::unique_ptr<Buffer>> uboBuffers;
	std::vector<std::unique_ptr<Buffer>> lightUboBuffers;
	std::vector<std::unique_ptr<Buffer>> materialUboBuffers;
	class GameObject::Map gameObjects;

	DepthPass depthPass;

	RenderSystem renderSystem {mDevice};
	PointLightSystem pointLightSystem {mDevice};
	WireframeSystem wireframeSystem {mDevice};
	ImGuiSystem imguiSystem {mDevice};
	UnlitSystem unlitSystem {mDevice};
	WorldGridSystem gridSystem {mDevice};
	SpotLightSystem spotLightSystem {mDevice};
	ShadowSystem shadowSystem {mDevice};

	RenderMode renderMode = DEFAULT_LIT;

	bool showGrid = false;

	size_t currentFrame = 0;
	bool framebufferResized = false;
	bool canResizeWindow = false;

	uint32_t currentImageIndex;
	int currentFrameIndex = 0;
	bool frameStarted = false;

	VkClearColorValue clearColor = { {0.01f, 0.01f, 0.01f, 1.0f} };

	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	SceneData sceneData;

	std::vector<Material> materials;
	size_t minUboAlignment;
	const uint32_t MAX_TEXTURE_BINDINGS = 3;
	uint32_t totalObjects = 0;
};

#endif // !RENDERER_H