#pragma once

#include "../Core/Context.h"
#include "Runtime/GameObject.h"
#include "Runtime/FrameInfo.h"

#include <vector>
#include <memory>

class SpotLightSystem
{
public:
	SpotLightSystem(Context& device);
	~SpotLightSystem();

	SpotLightSystem(const SpotLightSystem&) = delete;
	SpotLightSystem& operator=(const SpotLightSystem&) = delete;

	void init(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);

	void update(FrameInfo& frameInfo, LightUbo& ubo);
	void render(FrameInfo& frameInfo, LightUbo& ubo);

private:
	void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
	void createPipeline(VkRenderPass renderPass);

	Context& device;

	std::unique_ptr<class Pipeline_> pipeline;
	VkPipelineLayout pipelineLayout;
};