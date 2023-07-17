#include "Renderer.h"
#include "Log.h"
#include "GameObject.h"

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
	Log::init();
	CORE_WARN("Log initialized!");

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
	compileShaders();
	init();
}

void Renderer::init()
{
	recreateSwapChain();

	globalDescriptorPool =
		DescriptorPool::Builder(mDevice)
		.setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT * 2)
		.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
		.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6)
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
														   .build();

	std::unique_ptr<DescriptorSetLayout> textureSetLayout = DescriptorSetLayout::Builder(mDevice)
															.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, MAX_TEXTURE_BINDINGS)
															.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, MAX_TEXTURE_BINDINGS)
															.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, MAX_TEXTURE_BINDINGS)
															.addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, MAX_TEXTURE_BINDINGS)
															.addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, MAX_TEXTURE_BINDINGS)
															//.addBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, MAX_TEXTURE_BINDINGS)
															.build();

	globalDescriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < globalDescriptorSets.size(); i++)
	{
		VkDescriptorBufferInfo bufferInfo = uboBuffers[i]->descriptorInfo();
		DescriptorWriter(*globalSetLayout, *globalDescriptorPool)
			.writeBuffer(0, &bufferInfo)
			.build(globalDescriptorSets[i]);
	}

	textureDescriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
	CORE_WARN("Loading Textures...")
	loadTextures(*textureSetLayout);
	CORE_WARN("Texture Load Finished!")

	renderSystem.init(getSwapChainRenderPass().renderPass, globalSetLayout->getDescriptorSetLayout(), textureSetLayout->getDescriptorSetLayout());
	pointLightSystem.init(getSwapChainRenderPass().renderPass, globalSetLayout->getDescriptorSetLayout());
	wireframeSystem.init(getSwapChainRenderPass().renderPass, globalSetLayout->getDescriptorSetLayout());
	unlitSystem.init(getSwapChainRenderPass().renderPass, globalSetLayout->getDescriptorSetLayout(), textureSetLayout->getDescriptorSetLayout());
	gridSystem.init(getSwapChainRenderPass().renderPass, globalSetLayout->getDescriptorSetLayout());
	spotLightSystem.init(getSwapChainRenderPass().renderPass, globalSetLayout->getDescriptorSetLayout());
	//shadowSystem.init(depthPass.renderPass, globalSetLayout->getDescriptorSetLayout());

	createCommandBuffers();

	CORE_WARN("Loading Game Objects...")
	loadGameObjects();
	CORE_WARN("Game Object Load Complete!")

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
	CORE_WARN("Compiling Shaders:")
	system("CompileShaders.bat");
	CORE_WARN("Shader Compilation Complete!")
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

	if(depthPass.renderPass)
		depthPass.cleanup(mDevice);

	vkDeviceWaitIdle(mDevice.getDevice());
	mSwapChain = nullptr;
	if (mSwapChain == nullptr)
	{
		mSwapChain = std::make_unique<SwapChain>(mDevice, extent);
		depthPass.createRenderPass(mDevice, mSwapChain->getWidth(), mSwapChain->getHeight());
	}
	else
	{
		std::shared_ptr<SwapChain> oldSwapChain = std::move(mSwapChain); // std::move makes a copy of ptr and sets mSwapChain to nullptr
		mSwapChain = std::make_unique<SwapChain>(mDevice, extent, oldSwapChain); 
		depthPass.createRenderPass(mDevice, mSwapChain->getWidth(), mSwapChain->getHeight());

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
	teapot.setMaterial(0);
	teapot.transform.translation = {-0.5f, 0.0f, 0.0f};
	teapot.transform.rotation = {0.0f, 0.0f, 0.0f};
	teapot.transform.scale = {1.0f, 1.0f, 1.0f};
	teapot.setObjectName("Teapot");
	gameObjects.emplace(teapot.getID(), std::move(teapot));

	model = Model::createModelFromFile(mDevice, "MainApp/resources/vulkan/models/smoothVase/smooth_vase.obj");
	GameObject smoothVase = GameObject::createGameObject();
	smoothVase.model = model;
	smoothVase.setMaterial(1);
	smoothVase.transform.translation = { 0.5f, 0.0f, 0.0f };
	smoothVase.transform.rotation = { 0.0f, 0.0f, 0.0f };
	smoothVase.transform.scale = { 3.0f, 3.0f, 3.0f };
	smoothVase.setObjectName("SmoothVase");
	gameObjects.emplace(smoothVase.getID(), std::move(smoothVase));

	model = Model::createModelFromFile(mDevice, "MainApp/resources/vulkan/models/quad/quad.obj");
	GameObject floor = GameObject::createGameObject();
	floor.model = model;
	floor.setMaterial(0);
	floor.transform.translation = { 0.0f, 0.0f, -0.3f };
	floor.transform.rotation = { 0.0f, 0.0f, 0.0f };
	floor.transform.scale = { 3.0f, 3.0f, 1.0f };
	floor.setObjectName("Floor");
	gameObjects.emplace(floor.getID(), std::move(floor));

	std::vector<glm::vec3> lightColors{
	 {1.f, .1f, .1f},
	 {.1f, .1f, 1.f},
	 {.1f, 1.f, .1f},
	 {1.f, 1.f, .1f},
	 {.1f, 1.f, 1.f},
	 {1.f, 1.f, 1.f}
	};

	for (int i = 0; i < lightColors.size(); i++)
	{
		GameObject pointLight = GameObject::makePointLight();
		pointLight.color = lightColors[i];
		pointLight.pointLight->intensity = 5.0f;
		glm::mat4 lightRot = glm::rotate(glm::mat4(1.0f), (i * glm::two_pi<float>()) / lightColors.size(), {0.0f, 0.0f, 1.0f});
		pointLight.transform.translation = glm::vec3(lightRot * glm::vec4(0.0f, 1.5f, 1.5f, 1.0f));
		pointLight.setObjectName("PointLight" + std::to_string(i));
		gameObjects.emplace(pointLight.getID(), std::move(pointLight));
	}

	GameObject spotLight = GameObject::makeSpotLight(10.0f, 15.0f, glm::vec3(0.1f, 1.0f, 0.1f));
	spotLight.transform.translation = {2.0f, -2.0f, 1.0f};
	spotLight.setObjectName("SpotLight");
	gameObjects.emplace(spotLight.getID(), std::move(spotLight));

	spotLight = GameObject::makeSpotLight(10.0f, 25.0f, glm::vec3(1.0f, 1.0f, 0.0f));
	spotLight.transform.translation = { -2.0f, -2.0f, 1.0f };
	spotLight.setObjectName("SpotLight2");
	gameObjects.emplace(spotLight.getID(), std::move(spotLight));
}

void Renderer::loadTextures(DescriptorSetLayout& layout)
{
	std::vector<Texture> textures;

	Texture bricks;
	Utils::loadImageFromFile(mDevice, "MainApp/resources/vulkan/textures/bricks/Bricks_basecolor.png", bricks);
	bricks.createTextureImageView(mDevice);
	bricks.createTextureSampler(mDevice);
	bricks.setNameInternal("bricks_basecolor");
	loadedTextures["bricks_basecolor"] = bricks;
	textures.push_back(bricks);

	Texture bricksNrm;
	bricksNrm.setTextureFormat(VK_FORMAT_R8G8B8A8_UNORM);
	Utils::loadImageFromFile(mDevice, "MainApp/resources/vulkan/textures/bricks/Bricks_normal.png", bricksNrm, VK_FORMAT_R8G8B8A8_UNORM);
	bricksNrm.createTextureImageView(mDevice);
	bricksNrm.createTextureSampler(mDevice);
	bricksNrm.setNameInternal("bricks_nrm");
	loadedTextures["bricks_nrm"] = bricksNrm;
	textures.push_back(bricksNrm);

	Texture bricksRoughness;
	Utils::loadImageFromFile(mDevice, "MainApp/resources/vulkan/textures/bricks/Bricks_roughness.png", bricksRoughness);
	bricksRoughness.createTextureImageView(mDevice);
	bricksRoughness.createTextureSampler(mDevice);
	bricksRoughness.setNameInternal("bricks_roughness");
	loadedTextures["bricks_roughness"] = bricksRoughness;
	textures.push_back(bricksRoughness);

	Texture bricksAO;
	Utils::loadImageFromFile(mDevice, "MainApp/resources/vulkan/textures/bricks/Bricks_ambientocclusion.png", bricksAO);
	bricksAO.createTextureImageView(mDevice);
	bricksAO.createTextureSampler(mDevice);
	bricksAO.setNameInternal("bricks_ao");
	loadedTextures["bricks_ao"] = bricksAO;
	textures.push_back(bricksAO);

	Texture bricksHeight;
	Utils::loadImageFromFile(mDevice, "MainApp/resources/vulkan/textures/bricks/Bricks_height.png", bricksHeight);
	bricksHeight.createTextureImageView(mDevice);
	bricksHeight.createTextureSampler(mDevice);
	bricksHeight.setNameInternal("bricks_height");
	loadedTextures["bricks_height"] = bricksHeight;
	textures.push_back(bricksHeight);

	Texture stoneFloor;
	Utils::loadImageFromFile(mDevice, "MainApp/resources/vulkan/textures/stone_ground/ground_0042_color_1k.jpg", stoneFloor);
	stoneFloor.createTextureImageView(mDevice);
	stoneFloor.createTextureSampler(mDevice);
	stoneFloor.setNameInternal("stoneFloor_basecolor");
	loadedTextures["stoneFloor_basecolor"] = stoneFloor;
	textures.push_back(stoneFloor);

	Texture stoneFloorNrm;
	stoneFloorNrm.setTextureFormat(VK_FORMAT_R8G8B8A8_UNORM);
	Utils::loadImageFromFile(mDevice, "MainApp/resources/vulkan/textures/stone_ground/ground_0042_normal_opengl_1k.png", stoneFloorNrm, VK_FORMAT_R8G8B8A8_UNORM);
	stoneFloorNrm.createTextureImageView(mDevice);
	stoneFloorNrm.createTextureSampler(mDevice);
	stoneFloorNrm.setNameInternal("stoneFloor_nrm");
	loadedTextures["stoneFloor_nrm"] = stoneFloorNrm;
	textures.push_back(stoneFloorNrm);

	Texture stoneFloorRoughness;
	Utils::loadImageFromFile(mDevice, "MainApp/resources/vulkan/textures/stone_ground/ground_0042_roughness_1k.jpg", stoneFloorRoughness);
	stoneFloorRoughness.createTextureImageView(mDevice);
	stoneFloorRoughness.createTextureSampler(mDevice);
	stoneFloorRoughness.setNameInternal("stoneFloor_roughness");
	loadedTextures["stoneFloor_roughness"] = stoneFloorRoughness;
	textures.push_back(stoneFloorRoughness);

	Texture stoneFloorAO;
	Utils::loadImageFromFile(mDevice, "MainApp/resources/vulkan/textures/stone_ground/ground_0042_ao_1k.jpg", stoneFloorAO);
	stoneFloorAO.createTextureImageView(mDevice);
	stoneFloorAO.createTextureSampler(mDevice);
	stoneFloorAO.setNameInternal("stoneFloor_ao");
	loadedTextures["stoneFloor_ao"] = stoneFloorAO;
	textures.push_back(stoneFloorAO);

	Texture stoneFloorHeight;
	Utils::loadImageFromFile(mDevice, "MainApp/resources/vulkan/textures/stone_ground/ground_0042_height_1k.png", stoneFloorHeight);
	stoneFloorHeight.createTextureImageView(mDevice);
	stoneFloorHeight.createTextureSampler(mDevice);
	stoneFloorHeight.setNameInternal("stoneFloor_height");
	loadedTextures["stoneFloor_height"] = stoneFloorHeight;
	textures.push_back(stoneFloorHeight);

	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = stoneFloor.getTextureImageView();
	imageInfo.sampler = stoneFloor.getTextureSampler();

	for (uint32_t i = 0; i < textureDescriptorSets.size(); i++)
	{
		DescriptorWriter(layout, *globalDescriptorPool).build(textureDescriptorSets[i]);
	}

	uint32_t n = textures.size() / MAX_TEXTURE_BINDINGS;
	uint32_t textureIndex = 0;
	int k = 0;
	for(uint32_t i = 0; i < textures.size(); i++)
	{
		Texture& texture = textures[i];

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = texture.getTextureImageView();
		imageInfo.sampler = texture.getTextureSampler();

		uint32_t binding = i % n;//(n + 1);

		if(k == n)
		{
			textureIndex++;
			k = 0;
		}
		else
		{
			k++;
		}
		
		for(uint32_t j = 0; j < textureDescriptorSets.size(); j++)
		{
			DescriptorWriter(layout, *globalDescriptorPool).writeImageAtIndex(binding, textureIndex, &imageInfo).overwrite(textureDescriptorSets[j]);
		}
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

		FrameInfo frameInfo 
		{ 
			frameIndex, currentFrametime, currentFramerate, dt, 
			showGrid, renderMode, commandBuffer, mainCamera, 
			globalDescriptorSets[frameIndex], textureDescriptorSets[frameIndex], gameObjects
		};
		
		// update ubos
		GlobalUbo ubo{};
		ubo.projection = mainCamera.proj;
		ubo.view = mainCamera.view;
		ubo.inverseView = mainCamera.invView;
		pointLightSystem.update(frameInfo, ubo);
		spotLightSystem.update(frameInfo, ubo);
		uboBuffers[frameIndex]->writeToBuffer(&ubo);
		uboBuffers[frameIndex]->flush();

		// render
		beginSwapChainRenderPass(commandBuffer);
		mainCamera.updateModel(dt);

		switch (renderMode)
		{
		case DEFAULT_LIT:
			// order matters for transparency
			renderSystem.render(frameInfo);
			pointLightSystem.render(frameInfo, ubo);
			spotLightSystem.render(frameInfo, ubo);
			break;
		case WIREFRAME:
			wireframeSystem.render(frameInfo);
			break;
		case UNLIT:
			unlitSystem.render(frameInfo);
			break;
		default:
			break;
		}

		if(showGrid)
			gridSystem.render(frameInfo, ubo);

		drawImGui(frameInfo);
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

		endSwapChainRenderPass(commandBuffer);

		//depthPass.begin(commandBuffer);
		//shadowSystem.render(frameInfo);
		//depthPass.end(commandBuffer);

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

	std::vector<VkCommandBuffer> submitBuffers { commandBuffer };
	VkResult res = submit(submitBuffers);

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

VkResult Renderer::submit(std::vector<VkCommandBuffer> commadBuffers)
{
	return mSwapChain->submitCommandBuffers(commadBuffers.data(), &currentImageIndex);
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
	depthPass.cleanup(mDevice);

	renderSystem.cleanup();
	unlitSystem.cleanup();
	wireframeSystem.cleanup();

	freeCommandBuffers();
	window->cleanupWindow();
	globalDescriptorPool = nullptr;
	imguiDescriptorPool = nullptr;

	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
}