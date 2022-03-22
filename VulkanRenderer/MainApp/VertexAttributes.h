#ifndef VERTEX_ATTRIBUTES_H
#define VERTEX_ATTRIBUTES_H
#endif // !VERTEX_ATTRIBUTES_H

#define GLFW_INCLUDE_VULKAN
#include <array>
#include <vector>
#include <glfw3.h>
#include <glm/glm.hpp>

struct VertexAttributes
{
	VertexAttributes();
	VertexAttributes(int attribCount);

	std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
	void setAttributeDescription(uint32_t index, uint32_t bindingLocation, VkFormat format, uint32_t offset);

	int attributeCount;

private:
	std::vector<VkVertexInputAttributeDescription> descriptions;
};
