#pragma once

#include "RenderSystemBase.h"

#include "../Core/Context.h"
#include "Runtime/GameObject.h"
#include "Runtime/FrameInfo.h"

#include <vector>
#include <memory>

struct SimplePushConstantData
{
	glm::mat4 modelMatrix{ 1.0f };
	glm::mat4 normalMatrix{ 1.0f };
	uint32_t textureIndex = 0;
	uint32_t toggleTexture = 0;
};

class RenderSystem : public RenderSystemBase
{
public:
	RenderSystem(Context& device);
	~RenderSystem();

	virtual void init(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout additionalLayout = VK_NULL_HANDLE) override;
	virtual void update(FrameInfo& frameInfo, Buffer* buffer) override;
	virtual void render(FrameInfo& frameInfo) override;

protected:
	virtual void createPipelineLayout(VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout additionalLayout = VK_NULL_HANDLE) override;
	virtual void createPipeline(VkRenderPass renderPass) override;

public:
	std::vector<VkDescriptorSet> textureDescriptorSets;

private:
	uint32_t dynamicOffset;
};