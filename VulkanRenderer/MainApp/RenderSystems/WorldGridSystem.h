#pragma once

#include "../Device.h"
#include "../GameObject.h"
#include "../FrameInfo.h"

#include <vector>
#include <memory>

class WorldGridSystem
{
public:
	WorldGridSystem(Device& device);
	~WorldGridSystem();

	WorldGridSystem(const WorldGridSystem&) = delete;
	WorldGridSystem& operator=(const WorldGridSystem&) = delete;

	void init(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);

	void update(FrameInfo& frameInfo, GlobalUbo& ubo);
	void render(FrameInfo& frameInfo, GlobalUbo& ubo);

private:
	void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
	void createPipeline(VkRenderPass renderPass);

	Device& device;

	std::unique_ptr<class Pipeline> pipeline;
	VkPipelineLayout pipelineLayout;

	float minClip = 0.1f;
	float maxClip = 500.0f;
};