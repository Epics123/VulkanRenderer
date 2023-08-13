#pragma once

#include "Device.h"
#include "Buffer.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>
#include <memory>

class Model
{
public:
	
	struct Vertex
	{
		glm::vec3 position{};
		glm::vec3 color{};
		glm::vec3 normal{};
		glm::vec2 uv{};
		glm::vec3 tangent{};
		glm::vec3 bitangent{};

		static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
		static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

		bool operator==(const Vertex& other) const
		{
			return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
		}
	};

	struct Builder
	{
		std::vector<Vertex> vertices{};
		std::vector<uint32_t> indices{};

		void loadModel(const std::string& filepath);
	};


	Model(Device& device, const Builder& builder);
	~Model();

	Model(const Model&) = delete;
	void operator=(const Model&) = delete;

	static std::unique_ptr<Model> createModelFromFile(Device& device, const std::string& filename);

	void bind(VkCommandBuffer commandBuffer);
	void draw(VkCommandBuffer commandBuffer);

	void setModelPath(const std::string& path) { modelPath = path; }
	const std::string& getModelPath() { return modelPath; }

public:
	static const std::string modelDir;

private:
	void createVertexBuffers(const std::vector<Vertex>& vertices);
	void createIndexBuffers(const std::vector<uint32_t>& indicies);

	Device& device;

	std::unique_ptr<Buffer> vertexBuffer;
	uint32_t vertexCount;

	bool hasIndexBuffer = false;
	std::unique_ptr<Buffer> indexBuffer;
	uint32_t indexCount;

	std::string modelPath;
};