#pragma once

#include "../Device.h"
#include "../GameObject.h"
#include "../FrameInfo.h"

#include <vector>
#include <memory>

struct UnlitPushConstantData
{
	glm::mat4 modelMatrix{ 1.0f };
	glm::mat4 normalMatrix{ 1.0f };
};

class UnlitSystem
{
public:
	UnlitSystem(Device& device);
	~UnlitSystem();

	UnlitSystem(const UnlitSystem&) = delete;
	UnlitSystem& operator=(const UnlitSystem&) = delete;

	void init(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);

	void renderGameObjects(FrameInfo& frameInfo);

private:
	void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
	void createPipeline(VkRenderPass renderPass);

	Device& device;

	std::unique_ptr<class Pipeline> pipeline;
	VkPipelineLayout pipelineLayout;
};