#include "Renderer.h"
#include "Log.h"
//#include "GameObject.h"
#include "SceneSerializer.h"

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
		.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
		.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6)
		.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, SwapChain::MAX_FRAMES_IN_FLIGHT)
		.build();

	imguiDescriptorPool =
		DescriptorPool::Builder(mDevice)
		.setMaxSets(1000)
		.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
		.setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
		.build();

	createCommandBuffers();

	imguiInit();

	uboBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < uboBuffers.size(); i++)
	{
		uboBuffers[i] = std::make_unique<Buffer>(mDevice, sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		uboBuffers[i]->map();
	}

	lightUboBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < lightUboBuffers.size(); i++)
	{
		lightUboBuffers[i] = std::make_unique<Buffer>(mDevice, sizeof(LightUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		lightUboBuffers[i]->map();
	}

	// highest set common to all shaders
	std::unique_ptr<DescriptorSetLayout> globalSetLayout = DescriptorSetLayout::Builder(mDevice)
		.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
		.addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
		.build();

	std::unique_ptr<DescriptorSetLayout> materialSetLayout = DescriptorSetLayout::Builder(mDevice)
		.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, MAX_TEXTURE_BINDINGS)
		.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, MAX_TEXTURE_BINDINGS)
		.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, MAX_TEXTURE_BINDINGS)
		.addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, MAX_TEXTURE_BINDINGS)
		.addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, MAX_TEXTURE_BINDINGS)
		.addBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, MAX_TEXTURE_BINDINGS)
		.addBinding(6, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_ALL_GRAPHICS) // per material ubo
		.build();

	globalDescriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < globalDescriptorSets.size(); i++)
	{
		VkDescriptorBufferInfo bufferInfo = uboBuffers[i]->descriptorInfo();
		VkDescriptorBufferInfo lightBufferInfo = lightUboBuffers[i]->descriptorInfo();
		DescriptorWriter(*globalSetLayout, *globalDescriptorPool)
			.writeBuffer(0, &bufferInfo)
			.writeBuffer(1, &lightBufferInfo)
			.build(globalDescriptorSets[i]);
	}

	materialDescriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

	CORE_WARN("Loading Game Objects...")
	SceneSerializer serializer;
	if(!serializer.deserialize("MainApp/resources/scenes/untitled.scene", mDevice, sceneData))
	{
		CORE_ERROR("Failed to load scene!")
	}
	GameObject dirLight = GameObject::createGameObject();
	dirLight.directionalLight = std::make_unique<DirectionalLightComponent>();
	dirLight.directionalLight->direction = glm::vec3(0.0f, 0.0f, -1.0f);
	dirLight.directionalLight->color = glm::vec3(1.0f, 1.0f, 1.0f);
	dirLight.setObjectName("Directional Light");
	sceneData.objects.emplace(dirLight.getID(), std::move(dirLight));
	CORE_WARN("Game Object Load Complete!")

	materialUboBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
	minUboAlignment = mDevice.properties.limits.minUniformBufferOffsetAlignment;
	for (size_t i = 0; i < materialUboBuffers.size(); i++)
	{
		materialUboBuffers[i] = std::make_unique<Buffer>(mDevice, sizeof(MaterialUbo), sceneData.materialCount, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, minUboAlignment);
		materialUboBuffers[i]->map(materialUboBuffers[i]->getBufferSize());
	}	

	CORE_WARN("Loading Materials...")
	loadMaterials(*materialSetLayout);
	CORE_WARN("Material Load Finished!")
	

	renderSystem.init(getSwapChainRenderPass().renderPass, globalSetLayout->getDescriptorSetLayout(), materialSetLayout->getDescriptorSetLayout());
	pointLightSystem.init(getSwapChainRenderPass().renderPass, globalSetLayout->getDescriptorSetLayout());
	wireframeSystem.init(getSwapChainRenderPass().renderPass, globalSetLayout->getDescriptorSetLayout());
	unlitSystem.init(getSwapChainRenderPass().renderPass, globalSetLayout->getDescriptorSetLayout(), materialSetLayout->getDescriptorSetLayout());
	gridSystem.init(getSwapChainRenderPass().renderPass, globalSetLayout->getDescriptorSetLayout());
	spotLightSystem.init(getSwapChainRenderPass().renderPass, globalSetLayout->getDescriptorSetLayout());
	//shadowSystem.init(depthPass.renderPass, globalSetLayout->getDescriptorSetLayout());

	mainCamera = Camera();
	mainCamera.updateModel(0.0f);
	mainCamera.setPerspectiveProjection(mainCamera.fov, mSwapChain->extentAspectRatio(), 0.1f, 50.0f);
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

void Renderer::loadMaterials(DescriptorSetLayout& layout)
{
	for (uint32_t i = 0; i < (uint32_t)materialDescriptorSets.size(); i++)
	{
		VkDescriptorBufferInfo bufferInfo = materialUboBuffers[i]->descriptorInfo(materialUboBuffers[i]->getAlignmentSize());
		DescriptorWriter(layout, *globalDescriptorPool).writeBuffer(6, &bufferInfo).build(materialDescriptorSets[i]);
	}

	uint32_t textureIndex = 0;
	for (std::pair<std::string, std::shared_ptr<Material>> material : sceneData.materials)
	{
		ShaderParameters& params = material.second->getShaderParameters();
		if (params.toggleTexture)
		{
			for (std::pair<uint32_t, Texture> texture : params.materialTextures)
			{
				uint32_t binding = texture.first;
				Texture& tex = texture.second;

				VkDescriptorImageInfo imageInfo{};
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo.imageView = tex.getTextureImageView();
				imageInfo.sampler = tex.getTextureSampler();

				for (uint32_t j = 0; j < materialDescriptorSets.size(); j++)
				{
					DescriptorWriter(layout, *globalDescriptorPool).writeImageAtIndex(binding, textureIndex, &imageInfo).overwrite(materialDescriptorSets[j]);
				}
			}
			params.textureIndex = textureIndex;
			sceneData.materials[material.first]->setShaderParameters(params);
			textureIndex++;
		}
	}
}

void Renderer::cleanupTextures()
{
	for(auto& mat : sceneData.materials)
	{
		mat.second->cleanup(mDevice);
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
			globalDescriptorSets[frameIndex], materialDescriptorSets[frameIndex], sceneData.objects,
			0, 0
		};

		frameInfo.numObjs = totalObjects;
		frameInfo.dynamicOffset = materialUboBuffers[frameIndex]->getAlignmentSize();
		
		// update ubos
		GlobalUbo ubo{};
		ubo.projection = mainCamera.proj;
		ubo.view = mainCamera.view;
		ubo.inverseView = mainCamera.invView;
		uboBuffers[frameIndex]->writeToBuffer(&ubo);
		uboBuffers[frameIndex]->flush();

		LightUbo lightUbo{};
		pointLightSystem.update(frameInfo, lightUbo);
		spotLightSystem.update(frameInfo, lightUbo);
		lightUboBuffers[frameIndex]->writeToBuffer(&lightUbo);
		lightUboBuffers[frameIndex]->flush();

		// render
		beginSwapChainRenderPass(commandBuffer);
		mainCamera.updateModel(dt);

		switch (renderMode)
		{
		case DEFAULT_LIT:
			// order matters for transparency
			renderSystem.update(frameInfo, materialUboBuffers[frameIndex].get());
			renderSystem.render(frameInfo);
			pointLightSystem.render(frameInfo, lightUbo);
			spotLightSystem.render(frameInfo, lightUbo);
			break;
		case WIREFRAME:
			wireframeSystem.render(frameInfo);
			break;
		case UNLIT:
			unlitSystem.update(frameInfo, materialUboBuffers[frameIndex].get());
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