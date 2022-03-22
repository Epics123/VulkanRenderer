#include "VertexAttributes.h"

VertexAttributes::VertexAttributes()
	:attributeCount(1)
{
	descriptions.resize(attributeCount);
}

VertexAttributes::VertexAttributes(int attribCount)
	:attributeCount(attribCount)
{
	descriptions.resize(attributeCount);
}

std::vector<VkVertexInputAttributeDescription> VertexAttributes::getAttributeDescriptions()
{
	return descriptions;
}

void VertexAttributes::setAttributeDescription(uint32_t index, uint32_t bindingLocation, VkFormat format, uint32_t offset)
{
	if (index < descriptions.size()) 
	{
		descriptions[index].binding = 0;
		descriptions[index].location = bindingLocation;
		descriptions[index].format = format;
		descriptions[index].offset = offset;
	}
}
