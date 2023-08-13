#include "Model.h"

#include "Utils.h"
#include "Log.h"

#include <cassert>
#include <unordered_map>
#include <iostream>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

const std::string Model::modelDir = "MainApp/resources/vulkan/models/";

namespace std
{
	template<>
	struct hash<Model::Vertex>
	{
		size_t operator()(Model::Vertex const &vertex) const
		{
			size_t seed = 0;
			Utils::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv, vertex.tangent, vertex.bitangent);
			return seed;
		}
	};
}

Model::Model(Device& device, const Builder& builder) : device{device}
{
	createVertexBuffers(builder.vertices);
	createIndexBuffers(builder.indices);
}

Model::~Model()
{

}

std::unique_ptr<Model> Model::createModelFromFile(Device& device, const std::string& filename)
{
	Builder builder{};
	const std::string& path = modelDir + filename;
	builder.loadModel(path);

	CORE_INFO("Vertex Count: {0}", builder.vertices.size())

	std::unique_ptr<Model>modelPtr = std::make_unique<Model>(device, builder);
	modelPtr->setModelPath(filename);

	return modelPtr;
}

void Model::bind(VkCommandBuffer commandBuffer)
{
	VkBuffer buffers[] = {vertexBuffer->getBuffer()};
	VkDeviceSize offsets[] = {0};
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

	if(hasIndexBuffer)
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
}

void Model::draw(VkCommandBuffer commandBuffer)
{
	if(hasIndexBuffer)
		vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
	else
		vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
}

void Model::createVertexBuffers(const std::vector<Vertex>& vertices)
{
	vertexCount = static_cast<uint32_t>(vertices.size());
	assert(vertexCount >= 3 && "Vertex count must be at least 3");

	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
	uint32_t vertexSize = sizeof(vertices[0]);

	Buffer stagingBuffer(device, vertexSize, vertexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	stagingBuffer.map();
	stagingBuffer.writeToBuffer((void*)vertices.data());

	vertexBuffer = std::make_unique<Buffer>(device, vertexSize, vertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	device.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
}

void Model::createIndexBuffers(const std::vector<uint32_t>& indicies)
{
	indexCount = static_cast<uint32_t>(indicies.size());
	hasIndexBuffer = indexCount > 0;

	if(!hasIndexBuffer)
		return;

	VkDeviceSize bufferSize = sizeof(indicies[0]) * indexCount;
	uint32_t indexSize = sizeof(indicies[0]);

	Buffer stagingBuffer(device, indexSize, indexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	stagingBuffer.map();
	stagingBuffer.writeToBuffer((void*)indicies.data());

	indexBuffer = std::make_unique<Buffer>(device, indexSize, indexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	device.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
}

std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindingDescriptions()
{
	std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
	bindingDescriptions[0].binding = 0;
	bindingDescriptions[0].stride = sizeof(Vertex);
	bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions()
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

	attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT , offsetof(Vertex, position)});
	attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT , offsetof(Vertex, color)});
	attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT , offsetof(Vertex, normal)});
	attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT , offsetof(Vertex, uv)});
	attributeDescriptions.push_back({4, 0, VK_FORMAT_R32G32B32_SFLOAT , offsetof(Vertex, tangent)});
	attributeDescriptions.push_back({5, 0, VK_FORMAT_R32G32B32_SFLOAT , offsetof(Vertex, bitangent)});

	return attributeDescriptions;
}

void Model::Builder::loadModel(const std::string& filepath)
{
	tinyobj::attrib_t attributes; // stores position, color, normal, uv, etc
	std::vector<tinyobj::shape_t> shapes; // stores the index values for each face element
	std::vector<tinyobj::material_t> materials;	// .obj files can define specific materials per face, we currently ignore this
	std::string warn, err;

	// TinyObjectLoader has MTL compatibility
	// 	   TODO:: implement MTL loading via TOL
	// if(!tinyobj::LoadMtl())

	if (!tinyobj::LoadObj(&attributes, &shapes, &materials, &warn, &err, filepath.c_str()))
	{
		throw std::runtime_error(warn + err);
	}

	vertices.clear();
	indices.clear();

	std::unordered_map<Vertex, uint32_t> uniqueVerticies{};

	// The load function already triangulates faces; at this point it can be assumed that there are 3 vertices on each face
	for (std::vector<tinyobj::shape_t>::iterator shapeIter = shapes.begin(); shapeIter != shapes.end(); shapeIter++)
	{
		for (const tinyobj::index_t index : shapeIter._Ptr->mesh.indices)	// I would like to not be using foreach but good lord the types
		{
			Vertex vertex{};

			if (index.vertex_index >= 0)
			{
				vertex.position = {
					attributes.vertices[3 * index.vertex_index + 0],
					attributes.vertices[3 * index.vertex_index + 1],
					attributes.vertices[3 * index.vertex_index + 2]
				};

				vertex.color = {
					attributes.colors[3 * index.vertex_index + 0],
					attributes.colors[3 * index.vertex_index + 1],
					attributes.colors[3 * index.vertex_index + 2]
				};
			}
			
			if (index.normal_index >= 0)
			{
				vertex.normal = {
					attributes.normals[3 * index.normal_index + 0],
					attributes.normals[3 * index.normal_index + 1],
					attributes.normals[3 * index.normal_index + 2]
				};
			}

			if (index.texcoord_index >= 0)
			{
				vertex.uv = {
					attributes.texcoords[2 * index.texcoord_index + 0],
					attributes.texcoords[2 * index.texcoord_index + 1]
				};
			}

			// check if vertex is unique
			if (uniqueVerticies.count(vertex) == 0)
			{
				uniqueVerticies[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}
			
			indices.push_back(uniqueVerticies[vertex]);
		}
	}

	// calculate tangent basis
	for(int i = 0; i < indices.size(); i+=3)
	{
		Vertex& v0 = vertices[indices[i]];
		Vertex& v1 = vertices[indices[i + 1]];
		Vertex& v2 = vertices[indices[i + 2]];

		glm::vec3 edge1 = v1.position - v0.position;
		glm::vec3 edge2 = v2.position - v0.position;

		glm::vec2 deltaUV1 = v1.uv - v0.uv;
		glm::vec2 deltaUV2 = v2.uv - v0.uv;

		float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);

		glm::vec3 tangent = (edge1 * deltaUV2.y - edge2 * deltaUV1.y) * r;
		glm::vec3 bitangent = (edge2 * deltaUV1.x - edge1 * deltaUV2.x) * r;

		v0.tangent += tangent;
		v1.tangent += tangent;
		v2.tangent += tangent;

		v0.bitangent += bitangent;
		v1.bitangent += bitangent;
		v2.bitangent += bitangent;
	}
}
