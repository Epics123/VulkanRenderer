#pragma once

#include "RenderPass.h"

#include <memory>

class Context;

class GBufferPass
{
public:
	GBufferPass();

	void init(Context* context, uint32_t width, uint32_t height);

private:
	Context* context = nullptr;

	std::shared_ptr<Texture> baseColorTexture;
	std::shared_ptr<Texture> normalTexture;
	std::shared_ptr<Texture> emissiveTexture;
	std::shared_ptr<Texture> specularTexture;
	std::shared_ptr<Texture> velocityTexture;
	std::shared_ptr<Texture> depthTexture;

	std::shared_ptr<RenderPass> renderPass;
	//std::unique_ptr<Framebuffer>
};