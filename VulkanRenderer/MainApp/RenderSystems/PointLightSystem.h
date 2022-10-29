#pragma once

#include "../Device.h"
//#include "../Pipeline.h"
#include "../GameObject.h"
#include "../FrameInfo.h"

#include <vector>
#include <memory>

class PointLightSystem
{
public:
	PointLightSystem(Device& device);
	~PointLightSystem();

	PointLightSystem(const PointLightSystem&) = delete;
	PointLightSystem& operator=(const PointLightSystem&) = delete;

	void init(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);

	void render(FrameInfo& frameInfo);

private:
	void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
	void createPipeline(VkRenderPass renderPass);

	Device& device;

	std::unique_ptr<class Pipeline> pipeline;
	VkPipelineLayout pipelineLayout;
};