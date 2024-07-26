#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>
#include <functional>

class Context;
class Buffer;

class CommandQueueManager final
{
public:
	CommandQueueManager() {}

	CommandQueueManager(const Context& context, VkDevice inDevice, uint32_t count, 
						uint32_t numConcurrentCommands, uint32_t inQueueFamilyIndex, 
						VkQueue inQueue, VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, // default to reset/record command buffer every frame
						const std::string& name = "");

	~CommandQueueManager();

	void submit(const VkSubmitInfo* submitInfo);
	
	void moveToNextCommandBuffer();

	void waitForSubmission();
	void waitForAllSubmissions();

	void disposeWhenSubmitCompletes(std::shared_ptr<Buffer> buffer);
	void disposeWhenSubmitCompletes(std::function<void()>&& deallocator);

	VkCommandBuffer getAndBeginCmdBuffer();
	VkCommandBuffer getCmdBufferFromPool();
	void endCmdBuffer(VkCommandBuffer cmdBuffer);

	uint32_t getQueueFamilyIndex() const { return queueFamilyIndex; }

private:
	void createCommandPool(VkDevice device, VkCommandPoolCreateFlags flags, uint32_t count);
	void createCommandBuffers(uint32_t count);
	void createFences();

	void deallocateResources();

private:
	uint32_t commandsInFlight = 2;
	uint32_t queueFamilyIndex = 0;

	VkQueue queue = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	VkCommandPool commandPool = VK_NULL_HANDLE;

	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<VkFence> fences;

	std::vector<bool> isSubmittedList;

	uint32_t currentFenceIndex = 0;
	uint32_t currentCommandBufferIndex = 0;

	std::vector<std::vector<std::shared_ptr<Buffer>>> buffersToDispose;
	// fence that needs to be released
	std::vector<std::vector<std::function<void()>>> deallocators;

	std::string debugName;
};

