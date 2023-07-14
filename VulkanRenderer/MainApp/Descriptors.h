#pragma once

 #include "Device.h"

#include <memory>
#include <unordered_map>
#include <vector>

class DescriptorSetLayout
{
public:
	class Builder
	{
	public:
		Builder(Device& device) : mDevice{ device } {}

		Builder& addBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t count = 1);
		std::unique_ptr<DescriptorSetLayout> build() const;

	private:
		Device& mDevice;
		std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
	};

	DescriptorSetLayout(Device& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
	~DescriptorSetLayout();
	DescriptorSetLayout(const DescriptorSetLayout&) = delete;
	DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;

	VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

private:
	Device& mDevice;
	VkDescriptorSetLayout descriptorSetLayout;
	std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

	friend class DescriptorWriter;
};

class DescriptorPool
{
public:
	class Builder
	{
	public:
		Builder(Device& device) : mDevice{ device } {}

		Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
		Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
		Builder& setMaxSets(uint32_t count);
		std::unique_ptr<DescriptorPool> build() const;

	private:
		Device& mDevice;
		std::vector<VkDescriptorPoolSize> poolSizes{};
		uint32_t maxSets = 1000;
		VkDescriptorPoolCreateFlags poolFlags = 0;
	};

	DescriptorPool(Device& device, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize>& poolSizes);
	~DescriptorPool();
	DescriptorPool(const DescriptorPool&) = delete;
	DescriptorPool& operator=(const DescriptorPool&) = delete;

	bool allocateDescriptorSet(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

	void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

	void resetPool();

	VkDescriptorPool getDescriptorPool() { return descriptorPool; }

private:
	Device& mDevice;
	VkDescriptorPool descriptorPool;

	friend class DescriptorWriter;
};

class DescriptorWriter
{
public:
	DescriptorWriter(DescriptorSetLayout& setLayout, DescriptorPool& pool);

	DescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
	DescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);
	DescriptorWriter& writeImageAtIndex(uint32_t binding, uint32_t index, VkDescriptorImageInfo* imageInfo);

	bool build(VkDescriptorSet& set);
	void overwrite(VkDescriptorSet& set);

private:
	DescriptorSetLayout& setLayout;
	DescriptorPool& pool;
	std::vector<VkWriteDescriptorSet> writes;
};