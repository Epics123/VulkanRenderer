#pragma once

#include "../Core/Context.h"
#include "Runtime/GameObject.h"
#include "Runtime/FrameInfo.h"

#include <vector>
#include <memory>

class PointLightSystem
{
public:
	PointLightSystem(Context& device);
	~PointLightSystem();

	PointLightSystem(const PointLightSystem&) = delete;
	PointLightSystem& operator=(const PointLightSystem&) = delete;

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