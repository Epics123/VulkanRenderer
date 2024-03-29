#include "Descriptors.h"

#include <cassert>
#include <stdexcept>

// *************** Descriptor Set Layout Builder *********************

DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::addBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t count /*= 1*/)
{
	assert(bindings.count(binding) == 0 && "Binding already in use");

	VkDescriptorSetLayoutBinding layoutBinding{};
	layoutBinding.binding = binding;
	layoutBinding.descriptorType = descriptorType;
	layoutBinding.descriptorCount = count;
	layoutBinding.stageFlags = stageFlags;
	bindings[binding] = layoutBinding;
	return *this;
}

std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::build(bool unbound) const
{
	return std::make_unique<DescriptorSetLayout>(mDevice, bindings, unbound);
}


// *************** Descriptor Set Layout *********************

DescriptorSetLayout::DescriptorSetLayout(Device& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings, bool unbound)
	: mDevice{ device }, bindings{ bindings } 
{
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
	for (auto kv : bindings)
	{
		setLayoutBindings.push_back(kv.second);
	}

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
	descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
	descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();
	/*if (unbound)
	{
		std::vector<VkDescriptorBindingFlags> flags;
		flags.resize(setLayoutBindings.size());
		for(size_t i = 0; i < flags.size(); i++)
		{
			flags[i] = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;
		}

		VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlags{};
		bindingFlags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
		bindingFlags.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
		bindingFlags.pBindingFlags = flags.data();
		descriptorSetLayoutInfo.pNext = &bindingFlags;
	}*/

	if (vkCreateDescriptorSetLayout(mDevice.getDevice(), &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

DescriptorSetLayout::~DescriptorSetLayout()
{
	vkDestroyDescriptorSetLayout(mDevice.getDevice(), descriptorSetLayout, nullptr);
}

// *************** Descriptor Pool Builder *********************

DescriptorPool::Builder& DescriptorPool::Builder::addPoolSize(VkDescriptorType descriptorType, uint32_t count)
{
	poolSizes.push_back({ descriptorType, count });
	return *this;
}

DescriptorPool::Builder& DescriptorPool::Builder::setPoolFlags(VkDescriptorPoolCreateFlags flags)
{
	poolFlags = flags;
	return *this;
}
DescriptorPool::Builder& DescriptorPool::Builder::setMaxSets(uint32_t count)
{
	maxSets = count;
	return *this;
}

std::unique_ptr<DescriptorPool> DescriptorPool::Builder::build() const
{
	return std::make_unique<DescriptorPool>(mDevice, maxSets, poolFlags, poolSizes);
}

// *************** Descriptor Pool *********************

DescriptorPool::DescriptorPool(Device& device, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize>& poolSizes)
	: mDevice{ device }
{
	VkDescriptorPoolCreateInfo descriptorPoolInfo{};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	descriptorPoolInfo.pPoolSizes = poolSizes.data();
	descriptorPoolInfo.maxSets = maxSets;
	descriptorPoolInfo.flags = poolFlags;

	if (vkCreateDescriptorPool(mDevice.getDevice(), &descriptorPoolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

DescriptorPool::~DescriptorPool()
{
	vkDestroyDescriptorPool(mDevice.getDevice(), descriptorPool, nullptr);
}

bool DescriptorPool::allocateDescriptorSet(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const
{
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.pSetLayouts = &descriptorSetLayout;
	allocInfo.descriptorSetCount = 1;

	// Might want to create a "DescriptorPoolManager" class that handles this case, and builds
	// a new pool whenever an old pool fills up.
	if (vkAllocateDescriptorSets(mDevice.getDevice(), &allocInfo, &descriptor) != VK_SUCCESS)
	{
		return false;
	}
	return true;
}

void DescriptorPool::freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const
{
	vkFreeDescriptorSets(mDevice.getDevice(), descriptorPool, static_cast<uint32_t>(descriptors.size()), descriptors.data());
}

void DescriptorPool::resetPool()
{
	vkResetDescriptorPool(mDevice.getDevice(), descriptorPool, 0);
}

// *************** Descriptor Writer *********************

DescriptorWriter::DescriptorWriter(DescriptorSetLayout& setLayout, DescriptorPool& pool)
	: setLayout{ setLayout }, pool{ pool } {}

DescriptorWriter& DescriptorWriter::writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo)
{
	assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

	auto& bindingDescription = setLayout.bindings[binding];

	assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.descriptorType = bindingDescription.descriptorType;
	write.dstBinding = binding;
	write.pBufferInfo = bufferInfo;
	write.descriptorCount = 1;

	writes.push_back(write);
	return *this;
}

DescriptorWriter& DescriptorWriter::writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo)
{
	assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

	auto& bindingDescription = setLayout.bindings[binding];

	assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.descriptorType = bindingDescription.descriptorType;
	write.dstBinding = binding;
	write.pImageInfo = imageInfo;
	write.descriptorCount = 1;

	writes.push_back(write);
	return *this;
}

DescriptorWriter& DescriptorWriter::writeImageAtIndex(uint32_t binding, uint32_t index, VkDescriptorImageInfo* imageInfo)
{
	assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

	auto& bindingDescription = setLayout.bindings[binding];

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.descriptorType = bindingDescription.descriptorType;
	write.dstBinding = binding;
	write.dstArrayElement = index;
	write.pImageInfo = imageInfo;
	write.descriptorCount = 1;

	writes.push_back(write);
	return *this;
}

bool DescriptorWriter::build(VkDescriptorSet& set)
{
	bool success = pool.allocateDescriptorSet(setLayout.getDescriptorSetLayout(), set);
	if (!success)
	{
		return false;
	}
	overwrite(set);
	return true;
}

void DescriptorWriter::overwrite(VkDescriptorSet& set)
{
	for (auto& write : writes)
	{
		write.dstSet = set;
	}
	vkUpdateDescriptorSets(pool.mDevice.getDevice(), writes.size(), writes.data(), 0, nullptr);
}