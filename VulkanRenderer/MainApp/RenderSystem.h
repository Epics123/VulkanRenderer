#pragma once

#include "Device.h"
#include "Pipeline.h"
#include "GameObject.h"
#include "Camera.h"

#include <vector>
#include <memory>

struct SimplePushConstantData
{
	glm::mat4 transform{ 1.0f };
	alignas(16)glm::vec3 color;
};

class RenderSystem
{
public:
	RenderSystem(Device& device);
	~RenderSystem();

	RenderSystem(const RenderSystem&) = delete;
	RenderSystem& operator=(const RenderSystem&) = delete;

	void init(VkRenderPass renderPass);

	void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<GameObject>& gameObjects, Camera& camera);

private:
	void createPipelineLayout();
	void createPipeline(VkRenderPass renderPass);

	Device& device;

	std::unique_ptr<Pipeline> pipeline;
	VkPipelineLayout pipelineLayout;
};