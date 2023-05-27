#pragma once

#include "../Device.h"
#include "../GameObject.h"
#include "../FrameInfo.h"

#include <vector>
#include <memory>

struct SimplePushConstantData
{
	glm::mat4 modelMatrix{ 1.0f };
	glm::mat4 normalMatrix{ 1.0f };
};

class RenderSystem
{
public:
	RenderSystem(Device& device);
	~RenderSystem();

	RenderSystem(const RenderSystem&) = delete;
	RenderSystem& operator=(const RenderSystem&) = delete;

	void init(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout textureSetLayout = VK_NULL_HANDLE);

	void renderGameObjects(FrameInfo& frameInfo);

private:
	void createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout textureSetLayout);
	void createPipeline(VkRenderPass renderPass);

	Device& device;

	std::unique_ptr<class Pipeline> pipeline;
	VkPipelineLayout pipelineLayout;
};