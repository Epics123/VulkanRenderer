#pragma once

#include "../Device.h"
#include "../GameObject.h"
#include "../FrameInfo.h"

#include <vector>
#include <memory>

class SpotLightSystem
{
public:
	SpotLightSystem(Device& device);
	~SpotLightSystem();

	SpotLightSystem(const SpotLightSystem&) = delete;
	SpotLightSystem& operator=(const SpotLightSystem&) = delete;

	void init(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);

	void update(FrameInfo& frameInfo, LightUbo& ubo);
	void render(FrameInfo& frameInfo, LightUbo& ubo);

private:
	void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
	void createPipeline(VkRenderPass renderPass);

	Device& device;

	std::unique_ptr<class Pipeline> pipeline;
	VkPipelineLayout pipelineLayout;
};