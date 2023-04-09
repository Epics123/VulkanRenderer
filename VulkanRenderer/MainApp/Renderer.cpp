#include "Renderer.h"

#include <set>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <iostream>
#include <string.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//#define TINYOBJLOADER_IMPLEMENTATION
//#include <tiny_obj_loader.h>

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
	:window(appWindow)
{
	modelPath = "MainApp/resources/vulkan/models/teapot/downScaledPot.obj";
	//modelPath = "MainApp/resources/vulkan/models/debug/defCube.obj";
	//modelPath = "MainApp/resources/vulkan/models/debug/tri.obj";
	//modelPath = "MainApp/resources/vulkan/models/room/room.obj";
	texturePath = "MainApp/resources/vulkan/textures/bricks/Bricks_basecolor.png";
	//texturePath = "MainApp/resources/vulkan/textures/room/viking_room.png";

	//window = appWindow;
	//mDevice = Device(window);
	compileShaders();
	init();
}

void Renderer::init()
{
	recreateSwapChain();

	globalDescriptorPool =
		DescriptorPool::Builder(mDevice)
		.setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
		.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
		.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SwapChain::MAX_FRAMES_IN_FLIGHT)
		.build();

	imguiDescriptorPool =
		DescriptorPool::Builder(mDevice)
		.setMaxSets(1000)
		.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
		.setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
		.build();

	uboBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < uboBuffers.size(); i++)
	{
		uboBuffers[i] = std::make_unique<Buffer>(mDevice, sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		uboBuffers[i]->map();
	}

	// highest set common to all shaders
	std::unique_ptr<DescriptorSetLayout> globalSetLayout = DescriptorSetLayout::Builder(mDevice)
														   .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
														   .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
														   .build();

	std::unique_ptr<DescriptorSetLayout> textureSetLayout = DescriptorSetLayout::Builder(mDevice).addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT).build();

	globalDescriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < globalDescriptorSets.size(); i++)
	{
		VkDescriptorBufferInfo bufferInfo = uboBuffers[i]->descriptorInfo();
		DescriptorWriter(*globalSetLayout, *globalDescriptorPool)
						.writeBuffer(0, &bufferInfo)
						.build(globalDescriptorSets[i]);
	}

	renderSystem.init(getSwapChainRenderPass().renderPass, globalSetLayout->getDescriptorSetLayout());
	pointLightSystem.init(getSwapChainRenderPass().renderPass, globalSetLayout->getDescriptorSetLayout());
	wireframeSystem.init(getSwapChainRenderPass().renderPass, globalSetLayout->getDescriptorSetLayout());
	unlitSystem.init(getSwapChainRenderPass().renderPass, globalSetLayout->getDescriptorSetLayout());
	gridSystem.init(getSwapChainRenderPass().renderPass, globalSetLayout->getDescriptorSetLayout());

	createCommandBuffers();

	loadTextures(*globalSetLayout);
	loadGameObjects();

	mainCamera = Camera();
	mainCamera.updateModel(0.0f);
	mainCamera.setPerspectiveProjection(mainCamera.fov, mSwapChain->extentAspectRatio(), 0.1f, 50.0f);

	imguiInit();
}

void Renderer::imguiInit()
{
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGuiIO& io = ImGui::GetIO();
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

	ImGui_ImplGlfw_InitForVulkan(window->getWindow(), true);

	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = mDevice.getInstance();
	init_info.PhysicalDevice = mDevice.getPhysicalDevice();
	init_info.Device = mDevice.getDevice();
	init_info.Queue = mDevice.graphicsQueue();
	init_info.DescriptorPool = imguiDescriptorPool->getDescriptorPool();
	init_info.MinImageCount = 3;
	init_info.ImageCount = 3;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	ImGui_ImplVulkan_Init(&init_info, getSwapChainRenderPass().renderPass);

	// Upload Fonts
	VkCommandBuffer fontCmdBuffer = mDevice.beginSingleTimeCommands();
	ImGui_ImplVulkan_CreateFontsTexture(fontCmdBuffer);
	mDevice.endSingleTimeCommands(fontCmdBuffer);

	//TEMP: Should make own key codes
	//io.KeyMap[ImGuiKey_]
}

void Renderer::deviceWaitIdle()
{
	vkDeviceWaitIdle(mDevice.getDevice());
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

void Renderer::createCommandBuffers()
{
	commandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = mDevice.getCommandPool();
	allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

	if (vkAllocateCommandBuffers(mDevice.getDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate command buffers!");
	}
}

void Renderer::freeCommandBuffers()
{
	vkFreeCommandBuffers(mDevice.getDevice(), mDevice.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
	commandBuffers.clear();
}

void Renderer::recreateSwapChain()
{	
	//Rework
	VkExtent2D extent = window->getExtent();

	// Check if window is minimized
	while (extent.width == 0 || extent.height == 0)
	{
		extent = window->getExtent();
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(mDevice.getDevice());
	mSwapChain = nullptr;
	if (mSwapChain == nullptr)
	{
		mSwapChain = std::make_unique<SwapChain>(mDevice, extent);
	}
	else
	{
		std::shared_ptr<SwapChain> oldSwapChain = std::move(mSwapChain); // std::move makes a copy of ptr and sets mSwapChain to nullptr
		mSwapChain = std::make_unique<SwapChain>(mDevice, extent, oldSwapChain); 

		if (!oldSwapChain->compareSwapFormats(*mSwapChain.get()))
		{
			throw std::runtime_error("Swap chain image or depth format has changed!");
		}
	}
}

void Renderer::loadGameObjects()
{
	// Rework test
	std::shared_ptr<Model> model = Model::createModelFromFile(mDevice, "MainApp/resources/vulkan/models/teapot/downScaledPot.obj");
	GameObject teapot = GameObject::createGameObject();
	teapot.model = model;
	teapot.transform.translation = {-0.5f, 0.0f, 0.0f};
	teapot.transform.rotation = {0.0f, 0.0f, 0.0f};
	teapot.transform.scale = {1.0f, 1.0f, 1.0f};
	teapot.setObjectName("Teapot");
	gameObjects.emplace(teapot.getID(), std::move(teapot));

	model = Model::createModelFromFile(mDevice, "MainApp/resources/vulkan/models/smoothVase/smooth_vase.obj");
	GameObject smoothVase = GameObject::createGameObject();
	smoothVase.model = model;
	smoothVase.transform.translation = { 0.5f, 0.0f, 0.0f };
	smoothVase.transform.rotation = { 0.0f, 0.0f, 0.0f };
	smoothVase.transform.scale = { 3.0f, 3.0f, 3.0f };
	smoothVase.setObjectName("SmoothVase");
	gameObjects.emplace(smoothVase.getID(), std::move(smoothVase));

	model = Model::createModelFromFile(mDevice, "MainApp/resources/vulkan/models/quad/quad.obj");
	GameObject floor = GameObject::createGameObject();
	floor.model = model;
	floor.transform.translation = { 0.0f, 0.0f, -0.3f };
	floor.transform.rotation = { 0.0f, 0.0f, 0.0f };
	floor.transform.scale = { 3.0f, 3.0f, 1.0f };
	floor.setObjectName("Floor");
	gameObjects.emplace(floor.getID(), std::move(floor));

	std::vector<glm::vec3> lightColors{
	 //{1.f, .1f, .1f},
	 {.1f, .1f, 1.f},
	 //{.1f, 1.f, .1f},
	 //{1.f, 1.f, .1f},
	 //{.1f, 1.f, 1.f},
	 //{1.f, 1.f, 1.f}
	};

	for (int i = 0; i < lightColors.size(); i++)
	{
		GameObject pointLight = GameObject::makePointLight();
		pointLight.color = lightColors[i];
		glm::mat4 lightRot = glm::rotate(glm::mat4(1.0f), (i * glm::two_pi<float>()) / lightColors.size(), {0.0f, 0.0f, 1.0f});
		pointLight.transform.translation = glm::vec3(lightRot * glm::vec4(0.0f, 1.5f, 1.5f, 1.0f));
		pointLight.setObjectName("PointLight" + std::to_string(i));
		gameObjects.emplace(pointLight.getID(), std::move(pointLight));
	}

	GameObject spotLight = GameObject::makeSpotLight();
	spotLight.transform.translation = {0.0f, 0.0f, 2.0f};
	spotLight.setObjectName("spotLight");
	gameObjects.emplace(spotLight.getID(), std::move(spotLight));
}

void Renderer::loadTextures(DescriptorSetLayout& layout)
{
	Texture bricks;
	Utils::loadImageFromFile(mDevice, "MainApp/resources/vulkan/textures/bricks/Bricks_basecolor.png", bricks);
	bricks.createTextureImageView(mDevice);
	bricks.createTextureSampler(mDevice);
	loadedTextures["bricks_basecolor"] = bricks;

	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = bricks.getTextureImageView();
	imageInfo.sampler = bricks.getTextureSampler();
	for (int i = 0; i < globalDescriptorSets.size(); i++)
	{
		DescriptorWriter(layout, *globalDescriptorPool).writeImage(1, &imageInfo).overwrite(globalDescriptorSets[i]);
	}
}

void Renderer::cleanupTextures()
{
	for(std::pair<std::string, Texture> texture : loadedTextures)
	{
		texture.second.cleanup(mDevice);
	}
}

bool Renderer::hasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkCommandBuffer Renderer::beginFrame()
{
	assert(!isFrameInProgress() && "Can't call beginFrame while frame is in progress!");
	
	VkResult res;

	res = mSwapChain->acquireNextImage(&currentImageIndex);

	if (res == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapChain();
		return nullptr;
	}

	if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR)
		throw std::runtime_error("Failed to acquire swap chain image!");

	frameStarted = true;

	VkCommandBuffer commandBuffer = getCurrentCommandBuffer();
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0; // Optional
	beginInfo.pInheritanceInfo = nullptr; // Optional

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to begin recording command buffer!");
	}

	return commandBuffer;
}

void Renderer::drawFrame(float dt)
{
	if (VkCommandBuffer commandBuffer = beginFrame())
	{
		int frameIndex = getFrameIndex();

		imguiSystem.setViewportInfo(window->getWidth() * 0.5f, window->getHeight() * 0.5f, window->getWidth(), window->getHeight());

		FrameInfo frameInfo { frameIndex, currentFrametime, currentFramerate, dt, showGrid, renderMode, commandBuffer, mainCamera, globalDescriptorSets[frameIndex], gameObjects};
		
		// update ubos
		GlobalUbo ubo{};
		ubo.projection = mainCamera.proj;
		ubo.view = mainCamera.view;
		ubo.inverseView = mainCamera.invView;
		pointLightSystem.update(frameInfo, ubo);
		uboBuffers[frameIndex]->writeToBuffer(&ubo);
		uboBuffers[frameIndex]->flush();

		//TODO: Objects should rotate around their origin and not the world origin

		// render
		beginSwapChainRenderPass(commandBuffer);
		mainCamera.updateModel(dt);

		switch (renderMode)
		{
		case DEFAULT_LIT:
			// order matters for transparancy
			renderSystem.renderGameObjects(frameInfo);
			pointLightSystem.render(frameInfo, ubo);
			break;
		case WIREFRAME:
			wireframeSystem.renderGameObjects(frameInfo);
			break;
		case UNLIT:
			unlitSystem.renderGameObjects(frameInfo);
			break;
		default:
			break;
		}

		if(showGrid)
			gridSystem.render(frameInfo, ubo);

		drawImGui(frameInfo);
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

		endSwapChainRenderPass(commandBuffer);
		endFrame();
	}
}

void Renderer::endFrame()
{
	assert(isFrameInProgress() && "Can't call endFrame while frame is not in progress!");
	VkCommandBuffer commandBuffer = getCurrentCommandBuffer();

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to record command buffer!");
	}

	VkResult res;
	res = mSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);

	// Was window just resized
	if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || window->wasWindowResized())
	{
		window->resetWindowResizedFlag();
		recreateSwapChain();
		frameStarted = false;
		return;
	}
	if (res != VK_SUCCESS)
		throw std::runtime_error("Failed to present swap chain image!");

	frameStarted = false;
	currentFrameIndex = (currentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
}

void Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
	assert(isFrameInProgress() && "Can't call beginSwapChainRenderPass while frame is not in progress!");
	assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame!");

	getSwapChainRenderPass().begin(commandBuffer, currentImageIndex);
}

void Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
	assert(isFrameInProgress() && "Can't call endSwapChainRenderPass while frame is not in progress!");
	assert(commandBuffer == getCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame!");

	getSwapChainRenderPass().end(commandBuffer);
	//vkCmdEndRenderPass(commandBuffer);
}

void Renderer::drawImGui(FrameInfo& frameInfo)
{
	imguiSystem.drawImGui(frameInfo);
}

void Renderer::cleanup()
{
	cleanupTextures();

	freeCommandBuffers();
	window->cleanupWindow();
	globalDescriptorPool = nullptr;
	imguiDescriptorPool = nullptr;

	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
}