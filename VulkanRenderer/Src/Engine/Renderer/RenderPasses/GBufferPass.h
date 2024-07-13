#pragma once

#include "../Core/Context.h"
#include "RenderPass.h"

#include <memory>

class GBufferPass
{
public:
	GBufferPass();

	void init(Context* context, uint32_t width, uint32_t height);

private:
	Context* context = nullptr;

	std::shared_ptr<Texture_> baseColorTexture;
	std::shared_ptr<Texture_> normalTexture;
	std::shared_ptr<Texture_> emissiveTexture;
	std::shared_ptr<Texture_> specularTexture;
	std::shared_ptr<Texture_> velocityTexture;
	std::shared_ptr<Texture_> depthTexture;

	std::shared_ptr<RenderPass> renderPass;
	//std::unique_ptr<Framebuffer>
};