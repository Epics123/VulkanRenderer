/*
 * Encapsulates a vulkan buffer
 *
 * Initially based off VulkanBuffer by Sascha Willems -
 * https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanBuffer.h
 */

#include "Buffer.h"
#include "Context.h"
#include "Logging/Log.h"

#include <cassert>
#include <cstring>
#include <stdexcept>

// DEFERRED RENDERING REWORK

Buffer::Buffer(const Context* inContext, VmaAllocator vmaAllocator, VkDeviceSize deviceSize, VkBufferUsageFlags usage, Buffer* actualBuffer, const std::string& name)
	:context{inContext}, allocator{vmaAllocator}, size{deviceSize}, actualBufferIfStaging{actualBuffer}
{
	ASSERT(actualBufferIfStaging, "Actual buffer must not be null when creating a staging buffer");
	ASSERT(actualBufferIfStaging->usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT, "Actual buffer mush be a dst buffer when creating a staging buffer");
	ASSERT(actualBufferIfStaging->allocCreateInfo.usage == VMA_MEMORY_USAGE_GPU_ONLY, "Actual buffer must be GPU only when creating a staging buffer, "
			"staging buffer will upload from the CPU to this GPU buffer");

	VkBufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.pNext = VK_NULL_HANDLE;
	createInfo.flags = {};
	createInfo.size = size;
	createInfo.usage = usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.queueFamilyIndexCount = {};
	createInfo.pQueueFamilyIndices = {};

	allocCreateInfo = { VMA_ALLOCATION_CREATE_MAPPED_BIT, VMA_MEMORY_USAGE_CPU_ONLY };

	VkResult result = vmaCreateBuffer(allocator, &createInfo, &allocCreateInfo, &buffer, &allocation, nullptr);
	if(result != VK_SUCCESS)
	{
		CORE_CRITICAL("Failed to create staging buffer! Error code: {0}", result);
		throw std::runtime_error("");
	}
	vmaGetAllocationInfo(allocator, allocation, &allocationInfo);

	debugName = "Staging Buffer: " + name;
}

Buffer::Buffer(const Context* inContext, VmaAllocator vmaAllocator, const VkBufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocInfo, const std::string& name)
	:context{inContext}, allocator{vmaAllocator}, size{createInfo.size}, usage{createInfo.usage}, allocCreateInfo{allocInfo}
{
	VkResult result = vmaCreateBuffer(allocator, &createInfo, &allocCreateInfo, &buffer, &allocation, nullptr);
	if (result != VK_SUCCESS)
	{
		CORE_CRITICAL("Failed to create buffer! Error code: {0}", result);
		throw std::runtime_error("");
	}

	vmaGetAllocationInfo(allocator, allocation, &allocationInfo);

	debugName = "Buffer: " + name;
}

Buffer::~Buffer()
{
	if(mappedMemory)
	{
		vmaUnmapMemory(allocator, allocation);
	}

	for(auto& [bufferViewFormat, bufferView] : bufferViews)
	{
		vkDestroyBufferView(context->getDevice(), bufferView, nullptr);
	}

	vmaDestroyBuffer(allocator, buffer, allocation);
}

VkDeviceAddress Buffer::getDeviceAddress() const
{
	if(actualBufferIfStaging)
	{
		return actualBufferIfStaging->getDeviceAddress();
	}

#if defined(VK_KHR_buffer_device_address) && defined(_WIN32)
	if(!bufferDeviceAddress)
	{
		VkBufferDeviceAddressInfo addressInfo{};
		addressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
		addressInfo.buffer = buffer;
		addressInfo.pNext = VK_NULL_HANDLE;

		bufferDeviceAddress = vkGetBufferDeviceAddress(context->getDevice(), &addressInfo);
	}

	return bufferDeviceAddress;
#else
	return 0;
#endif
}

void Buffer::upload(VkDeviceSize offset) const
{
	upload(offset, size);
}

void Buffer::upload(VkDeviceSize offset, VkDeviceSize bufferSize) const
{
	VkResult result = vmaFlushAllocation(allocator, allocation, offset, bufferSize);
	if(result != VK_SUCCESS)
	{
		CORE_CRITICAL("Failed to upload buffer: {0}! Error code: {1}", debugName, result);
		throw std::runtime_error("");
	}
}

void Buffer::uploadStagingBuffer(const VkCommandBuffer& cmdBuffer, uint64_t srcOffset, uint64_t dstOffset)
{
	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = srcOffset;
	copyRegion.dstOffset = dstOffset;
	copyRegion.size = size;

	ASSERT(actualBufferIfStaging != nullptr, "Actual buffer can't be null when uploading a staging buffer!");

	vkCmdCopyBuffer(cmdBuffer, buffer, actualBufferIfStaging->getBuffer(), 1, &copyRegion);
}

void Buffer::writeToBuffer(const void* data, size_t size)
{
	if(!mappedMemory)
	{
		VkResult result = vmaMapMemory(allocator, allocation, &mappedMemory);
		if(result != VK_SUCCESS)
		{
			CORE_CRITICAL("Failed to map buffer memory for {0}! Error code {1}", debugName, result);
			throw std::runtime_error("");
		}
	}
}

VkBufferView Buffer::requestBufferView(VkFormat viewFormat)
{
	auto itr = bufferViews.find(viewFormat);
	if(itr != bufferViews.end())
	{
		return itr->second;
	}

	VkBufferViewCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
	createInfo.flags = 0;
	createInfo.buffer = buffer;
	createInfo.format = viewFormat;
	createInfo.offset = 0;
	createInfo.range = size;
	createInfo.pNext = VK_NULL_HANDLE;

	VkBufferView bufferView;
	VkResult result = vkCreateBufferView(context->getDevice(), &createInfo, nullptr, &bufferView);
	if(result != VK_SUCCESS)
	{
		CORE_CRITICAL("Failed to request buffer view for {0}! Error code {1}", debugName, result);
		throw std::runtime_error("");
	}

	bufferViews[viewFormat] = bufferView;
	return bufferView;
}

// END DEFERRED RENDERING REWORK

///**
// * Returns the minimum instance size required to be compatible with devices minOffsetAlignment
// *
// * @param instanceSize The size of an instance
// * @param minOffsetAlignment The minimum required alignment, in bytes, for the offset member (eg
// * minUniformBufferOffsetAlignment)
// *
// * @return VkResult of the buffer mapping call
// */
//VkDeviceSize Buffer::getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment)
//{
//    if (minOffsetAlignment > 0)
//    {
//        return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
//    }
//    return instanceSize;
//}
//
//Buffer::Buffer(Context& device, VkDeviceSize instanceSize, uint32_t instanceCount, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize minOffsetAlignment)
//    : mDevice{ device }, instanceSize{ instanceSize }, instanceCount{ instanceCount }, usageFlags{ usageFlags }, memoryPropertyFlags{ memoryPropertyFlags }
//{
//    alignmentSize = getAlignment(instanceSize, minOffsetAlignment);
//    bufferSize = alignmentSize * instanceCount;
//    device.createBuffer(bufferSize, usageFlags, memoryPropertyFlags, buffer, memory);
//}
//
//Buffer::~Buffer()
//{
//    unmap();
//    vkDestroyBuffer(mDevice.getDevice(), buffer, nullptr);
//    vkFreeMemory(mDevice.getDevice(), memory, nullptr);
//}
//
///**
// * Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
// *
// * @param size (Optional) Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete
// * buffer range.
// * @param offset (Optional) Byte offset from beginning
// *
// * @return VkResult of the buffer mapping call
// */
//VkResult Buffer::map(VkDeviceSize size, VkDeviceSize offset)
//{
//    assert(buffer && memory && "Called map on buffer before create");
//    return vkMapMemory(mDevice.getDevice(), memory, offset, size, 0, &mapped);
//}
//
///**
// * Unmap a mapped memory range
// *
// * @note Does not return a result as vkUnmapMemory can't fail
// */
//void Buffer::unmap()
//{
//    if (mapped)
//    {
//        vkUnmapMemory(mDevice.getDevice(), memory);
//        mapped = nullptr;
//    }
//}
//
///**
// * Copies the specified data to the mapped buffer. Default value writes whole buffer range
// *
// * @param data Pointer to the data to copy
// * @param size (Optional) Size of the data to copy. Pass VK_WHOLE_SIZE to flush the complete buffer
// * range.
// * @param offset (Optional) Byte offset from beginning of mapped region
// *
// */
//void Buffer::writeToBuffer(void* data, VkDeviceSize size, VkDeviceSize offset)
//{
//    assert(mapped && "Cannot copy to unmapped buffer");
//
//    if (size == VK_WHOLE_SIZE)
//    {
//        memcpy(mapped, data, bufferSize);
//    }
//    else
//    {
//        char* memOffset = (char*)mapped;
//        memOffset += offset;
//        memcpy(memOffset, data, size);
//    }
//}
//
///**
// * Flush a memory range of the buffer to make it visible to the device
// *
// * @note Only required for non-coherent memory
// *
// * @param size (Optional) Size of the memory range to flush. Pass VK_WHOLE_SIZE to flush the
// * complete buffer range.
// * @param offset (Optional) Byte offset from beginning
// *
// * @return VkResult of the flush call
// */
//VkResult Buffer::flush(VkDeviceSize size, VkDeviceSize offset)
//{
//    VkMappedMemoryRange mappedRange = {};
//    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
//    mappedRange.memory = memory;
//    mappedRange.offset = offset;
//    mappedRange.size = size;
//    return vkFlushMappedMemoryRanges(mDevice.getDevice(), 1, &mappedRange);
//}
//
///**
// * Invalidate a memory range of the buffer to make it visible to the host
// *
// * @note Only required for non-coherent memory
// *
// * @param size (Optional) Size of the memory range to invalidate. Pass VK_WHOLE_SIZE to invalidate
// * the complete buffer range.
// * @param offset (Optional) Byte offset from beginning
// *
// * @return VkResult of the invalidate call
// */
//VkResult Buffer::invalidate(VkDeviceSize size, VkDeviceSize offset)
//{
//    VkMappedMemoryRange mappedRange = {};
//    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
//    mappedRange.memory = memory;
//    mappedRange.offset = offset;
//    mappedRange.size = size;
//    return vkInvalidateMappedMemoryRanges(mDevice.getDevice(), 1, &mappedRange);
//}
//
///**
// * Create a buffer info descriptor
// *
// * @param size (Optional) Size of the memory range of the descriptor
// * @param offset (Optional) Byte offset from beginning
// *
// * @return VkDescriptorBufferInfo of specified offset and range
// */
//VkDescriptorBufferInfo Buffer::descriptorInfo(VkDeviceSize size, VkDeviceSize offset)
//{
//    return VkDescriptorBufferInfo{
//        buffer,
//        offset,
//        size,
//    };
//}
//
///**
// * Copies "instanceSize" bytes of data to the mapped buffer at an offset of index * alignmentSize
// *
// * @param data Pointer to the data to copy
// * @param index Used in offset calculation
// *
// */
//void Buffer::writeToIndex(void* data, int index)
//{
//    writeToBuffer(data, instanceSize, index * alignmentSize);
//}
//
///**
// *  Flush the memory range at index * alignmentSize of the buffer to make it visible to the device
// *
// * @param index Used in offset calculation
// *
// */
//VkResult Buffer::flushIndex(int index) { return flush(alignmentSize, index * alignmentSize); }
//
///**
// * Create a buffer info descriptor
// *
// * @param index Specifies the region given by index * alignmentSize
// *
// * @return VkDescriptorBufferInfo for instance at index
// */
//VkDescriptorBufferInfo Buffer::descriptorInfoForIndex(int index)
//{
//    return descriptorInfo(alignmentSize, index * alignmentSize);
//}
//
///**
// * Invalidate a memory range of the buffer to make it visible to the host
// *
// * @note Only required for non-coherent memory
// *
// * @param index Specifies the region to invalidate: index * alignmentSize
// *
// * @return VkResult of the invalidate call
// */
//VkResult Buffer::invalidateIndex(int index)
//{
//    return invalidate(alignmentSize, index * alignmentSize);
//}
