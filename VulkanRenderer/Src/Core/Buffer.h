#pragma once

//#include "Context.h"
#include "../Common/Defines.h"

#include <vma/vk_mem_alloc.h>

#include <string>
#include <unordered_map>

class Context;
class Texture;

class Buffer final
{
public:
	MOVABLE_ONLY(Buffer);

	// This will create a staging buffer by default
	explicit Buffer(const Context* inContext, VmaAllocator vmaAllocator, VkDeviceSize deviceSize, VkBufferUsageFlags usage,
					Buffer* actualBuffer, const std::string& name = "");
	
	// This will only create a non-staging buffer
	explicit Buffer(const Context* inContext, VmaAllocator vmaAllocator, const VkBufferCreateInfo& createInfo,
					const VmaAllocationCreateInfo& allocInfo, const std::string& name = "");

	~Buffer();

	VkDeviceSize getSize() const { return size; }
	VkBuffer getBuffer() const { return buffer; }
	VkDeviceAddress getDeviceAddress() const;

	void upload(VkDeviceSize offset = 0) const;
	void upload(VkDeviceSize offset, VkDeviceSize bufferSize) const;

	// Uploads staging buffer to the GPU
	void uploadStagingBuffer(const VkCommandBuffer& cmdBuffer, uint64_t srcOffset, uint64_t dstOffset);

	void writeToBuffer(const void* data, size_t size);

	VkBufferView requestBufferView(VkFormat viewFormat);

private:
	const Context* context = nullptr;
	VmaAllocator allocator;
	VkDeviceSize size;
	VkBufferUsageFlags usage;
	VkBuffer buffer = VK_NULL_HANDLE;
	Buffer* actualBufferIfStaging = nullptr;
	
	VmaAllocationCreateInfo allocCreateInfo;
	VmaAllocation allocation = nullptr;
	VmaAllocationInfo allocationInfo = {};

	mutable VkDeviceAddress bufferDeviceAddress = 0;
	mutable void* mappedMemory = nullptr;
	std::unordered_map<VkFormat, VkBufferView> bufferViews;

	std::string debugName = "";
};

//class Buffer
//{
//public:
//    Buffer(Context& device, VkDeviceSize instanceSize, uint32_t instanceCount, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize minOffsetAlignment = 1);
//    ~Buffer();
//
//    Buffer(const Buffer&) = delete;
//    Buffer& operator=(const Buffer&) = delete;
//
//    VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
//    void unmap();
//
//    void writeToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
//    VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
//    VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
//    VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
//
//    void writeToIndex(void* data, int index);
//    VkResult flushIndex(int index);
//    VkDescriptorBufferInfo descriptorInfoForIndex(int index);
//    VkResult invalidateIndex(int index);
//
//    VkBuffer getBuffer() const { return buffer; }
//    void* getMappedMemory() const { return mapped; }
//    uint32_t getInstanceCount() const { return instanceCount; }
//    VkDeviceSize getInstanceSize() const { return instanceSize; }
//    VkDeviceSize getAlignmentSize() const { return alignmentSize; }
//    VkBufferUsageFlags getUsageFlags() const { return usageFlags; }
//    VkMemoryPropertyFlags getMemoryPropertyFlags() const { return memoryPropertyFlags; }
//    VkDeviceSize getBufferSize() const { return bufferSize; }
//
//private:
//     static VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);
//
//    Context& mDevice;
//    void* mapped = nullptr;
//    VkBuffer buffer = VK_NULL_HANDLE;
//    VkDeviceMemory memory = VK_NULL_HANDLE;
//
//    VkDeviceSize bufferSize;
//    uint32_t instanceCount;
//    VkDeviceSize instanceSize;
//    VkDeviceSize alignmentSize;
//    VkBufferUsageFlags usageFlags;
//    VkMemoryPropertyFlags memoryPropertyFlags;
//};