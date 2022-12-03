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
#include "RenderSystems/RenderSystem.h"
#include "RenderSystems/PointLightSystem.h"
#include "RenderSystems/WireframeSystem.h"
#include "RenderSystems/ImGuiSystem.h"
#include "RenderSystems/UnlitSystem.h"
#include "Buffer.h"
#include "Descriptors.h"
#include "Enums.h"

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

	void createCommandBuffers();
	void freeCommandBuffers();


	void recreateSwapChain();
	VkRenderPass getSwapChainRenderPass() const { return mSwapChain->getRenderPass(); }

	void loadGameObjects();

	bool hasStencilComponent(VkFormat format);

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

	static void compileShaders();

	static std::vector<char> readBinaryFile(const std::string& filename);

	float currentFramerate;
	float currentFrametime;

private:
	Renderer(Window* appWindow);

	std::unique_ptr<Window> window;

	Camera mainCamera;

	std::string modelPath;
	std::string texturePath;
	
	std::vector<VkCommandBuffer> commandBuffers;
	
	Device mDevice{*window};
	std::unique_ptr <SwapChain> mSwapChain;

	std::unique_ptr<DescriptorPool> globalDescriptorPool{};
	std::unique_ptr<DescriptorPool> imguiDescriptorPool{};
	std::vector<VkDescriptorSet> globalDescriptorSets;
	std::vector<std::unique_ptr<Buffer>> uboBuffers;
	GameObject::Map gameObjects;

	RenderSystem renderSystem {mDevice};
	PointLightSystem pointLightSystem {mDevice};
	WireframeSystem wireframeSystem {mDevice};
	ImGuiSystem imguiSystem {mDevice};
	UnlitSystem unlitSystem {mDevice};

	RenderMode renderMode = DEFAULT_LIT;

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
};

#endif // !RENDERER_H