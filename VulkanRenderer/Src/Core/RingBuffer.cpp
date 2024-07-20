#include "RingBuffer.h"

#include "Context.h"
#include "Buffer.h"

RingBuffer::RingBuffer()
{

}

RingBuffer::RingBuffer(uint32_t inRingSize, Context* inContext, size_t buffSize, const std::string& name)
	:ringSize{inRingSize}, bufferSize{buffSize}, debugName{name}
{
	init(ringSize, inContext, bufferSize, debugName);
}



void RingBuffer::init(uint32_t inRingSize, Context* inContext, size_t buffSize, const std::string& name)
{
	ringSize = inRingSize;
	bufferSize = buffSize;
	debugName = name;

	for (uint32_t i = 0; i < ringSize; i++)
	{
		VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
#if VK_KHR_buffer_device_address && _WIN32
		usageFlags |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
#endif
		std::shared_ptr<Buffer> buffer = inContext->createPersistantBuffer(bufferSize, usageFlags, debugName + " " + std::to_string(i));
		ringBuffer.emplace_back(buffer);
	}
}

void RingBuffer::advanceBuffer()
{
	ringIndex++;
	if(ringIndex > ringSize)
	{
		ringIndex = 0;
	}
}
