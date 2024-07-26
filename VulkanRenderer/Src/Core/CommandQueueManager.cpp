#include "CommandQueueManager.h"
#include "Logging/Log.h"

#include <stdexcept>

CommandQueueManager::CommandQueueManager(const Context& context, VkDevice inDevice, uint32_t count, uint32_t numConcurrentCommands, 
										 uint32_t inQueueFamilyIndex, VkQueue inQueue, VkCommandPoolCreateFlags flags, const std::string& name)
	:commandsInFlight{numConcurrentCommands}, queueFamilyIndex{inQueueFamilyIndex}, queue{inQueue}, device{inDevice}, debugName{name}
{
	fences.reserve(commandsInFlight);
	isSubmittedList.reserve(commandsInFlight);
	buffersToDispose.reserve(commandsInFlight);
	deallocators.reserve(commandsInFlight);

	createCommandPool(device, flags, count);
}

CommandQueueManager::~CommandQueueManager()
{
	deallocateResources();

	for(size_t i = 0; i < commandsInFlight; i++)
	{
		vkDestroyFence(device, fences[i], nullptr);
	}

	for(size_t j = 0; j < commandBuffers.size(); j++)
	{
		vkFreeCommandBuffers(device, commandPool, 1, &commandBuffers[j]);
	}

	vkDestroyCommandPool(device, commandPool, nullptr);
}

void CommandQueueManager::submit(const VkSubmitInfo* submitInfo)
{
	VkResult result = vkResetFences(device, 1, &fences[currentFenceIndex]);
	if(result != VK_SUCCESS)
	{
		CORE_CRITICAL("Command Queue Manager failed to reset fences! Error code: {0}", result);
		throw std::runtime_error("");
	}

	result = vkQueueSubmit(queue, 1, submitInfo, fences[currentFenceIndex]);
	if(result != VK_SUCCESS)
	{
		CORE_CRITICAL("Command Queue Manager failed to submit! Error code: {0}", result);
		throw std::runtime_error("");
	}

	isSubmittedList[currentFenceIndex] = true;
}

void CommandQueueManager::moveToNextCommandBuffer()
{
	currentCommandBufferIndex = (currentCommandBufferIndex + 1) % static_cast<uint32_t>(commandBuffers.size());
	currentFenceIndex = (currentFenceIndex + 1) % commandsInFlight;
}

void CommandQueueManager::waitForSubmission()
{
	if(!isSubmittedList[currentFenceIndex])
	{
		return;
	}

	VkResult result = vkWaitForFences(device, 1, &fences[currentFenceIndex], true, UINT32_MAX);
	if(result == VK_TIMEOUT)
	{
		CORE_WARN("Wait for fences timed out!");
		vkDeviceWaitIdle(device);
	}

	isSubmittedList[currentFenceIndex] = false;
	if (buffersToDispose.size() > 0)
	{
		buffersToDispose[currentFenceIndex].clear();
	}

	deallocateResources();
}

void CommandQueueManager::waitForAllSubmissions()
{
	for(size_t i = 0; i < fences.size(); i++)
	{
		VkFence& fence = fences[i];

		VkResult result = vkWaitForFences(device, 1, &fence, true, UINT32_MAX);
		if(result != VK_SUCCESS)
		{
			CORE_CRITICAL("Command Queue Manager failed to wait for fences! Error code: {0}", result);
			throw std::runtime_error("");
		}

		result = vkResetFences(device, 1, &fence);
		if(result != VK_SUCCESS)
		{
			CORE_CRITICAL("Command Queue Manager failed to reset fences! Error code: {0}", result);
			throw std::runtime_error("");
		}

		isSubmittedList[i] = false;
	}

	buffersToDispose.clear();
	deallocateResources();
}

void CommandQueueManager::disposeWhenSubmitCompletes(std::shared_ptr<Buffer> buffer)
{
	buffersToDispose[currentFenceIndex].push_back(std::move(buffer));
}

void CommandQueueManager::disposeWhenSubmitCompletes(std::function<void()>&& deallocator)
{
	deallocators[currentFenceIndex].push_back(std::move(deallocator));
}

VkCommandBuffer CommandQueueManager::getAndBeginCmdBuffer()
{
	VkResult result = vkWaitForFences(device, 1, &fences[currentFenceIndex], VK_TRUE, UINT32_MAX);
	if (result != VK_SUCCESS)
	{
		CORE_CRITICAL("Command Queue Manager failed to wait for fences! Error code: {0}", result);
		throw std::runtime_error("");
	}

	result = vkResetCommandBuffer(commandBuffers[currentCommandBufferIndex], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
	if(result != VK_SUCCESS)
	{
		CORE_CRITICAL("Command Queue Manager failed to reset command buffer! Error code: {0}", result);
		throw std::runtime_error("");
	}

	VkCommandBufferBeginInfo info{};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	info.pNext = VK_NULL_HANDLE;
	info.pInheritanceInfo = VK_NULL_HANDLE;

	result = vkBeginCommandBuffer(commandBuffers[currentCommandBufferIndex], &info);

	return commandBuffers[currentCommandBufferIndex];
}

VkCommandBuffer CommandQueueManager::getCmdBufferFromPool()
{
	VkCommandBufferAllocateInfo commandBufferInfo{};
	commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferInfo.commandPool = commandPool;
	commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferInfo.commandBufferCount = 1;
	commandBufferInfo.pNext = VK_NULL_HANDLE;

	VkCommandBuffer cmdBuffer{VK_NULL_HANDLE};
	VkResult result = vkAllocateCommandBuffers(device, &commandBufferInfo, &cmdBuffer);
	if(result != VK_SUCCESS)
	{
		CORE_CRITICAL("Failed to allocate command buffer from command pool! Error code: {0}", result);
		throw std::runtime_error("");
	}

	return cmdBuffer;
}

void CommandQueueManager::endCmdBuffer(VkCommandBuffer cmdBuffer)
{
	VkResult result = vkEndCommandBuffer(cmdBuffer);
	if(result != VK_SUCCESS)
	{
		CORE_CRITICAL("Failed to end command buffer! Error code: {0}", result);
		throw std::runtime_error("");
	}
}

void CommandQueueManager::createCommandPool(VkDevice device, VkCommandPoolCreateFlags flags, uint32_t count)
{
	VkCommandPoolCreateInfo commandPoolInfo{};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.flags = flags;
	commandPoolInfo.queueFamilyIndex = queueFamilyIndex;
	commandPoolInfo.pNext = VK_NULL_HANDLE;

	VkResult result = vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool);
	if(result != VK_SUCCESS)
	{
		CORE_CRITICAL("Failed to create command pool! Error code: {0}", result);
		throw std::runtime_error("");
	}

	createCommandBuffers(count);
	createFences();
}

void CommandQueueManager::createCommandBuffers(uint32_t count)
{
	VkCommandBufferAllocateInfo commandBufferInfo{};
	commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferInfo.commandPool = commandPool;
	commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferInfo.commandBufferCount = 1;
	commandBufferInfo.pNext = VK_NULL_HANDLE;

	for (size_t i = 0; i < count; i++)
	{
		VkCommandBuffer cmdBuffer;
		VkResult result = vkAllocateCommandBuffers(device, &commandBufferInfo, &cmdBuffer);
		if (result != VK_SUCCESS)
		{
			const std::string name = debugName + " " + std::to_string(i);
			CORE_CRITICAL("Failed to allocate command buffer: {0}! Error code: {1}", debugName, result);
			throw std::runtime_error("");
		}
		commandBuffers.push_back(cmdBuffer);
	}
}

void CommandQueueManager::createFences()
{
	for (size_t i = 0; i < commandsInFlight; i++)
	{
		VkFence fence;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		fenceInfo.pNext = VK_NULL_HANDLE;

		VkResult result = vkCreateFence(device, &fenceInfo, nullptr, &fence);
		if (result != VK_SUCCESS)
		{
			CORE_CRITICAL("Failed to create command buffer fence! Error code: {0}", result);
			throw std::runtime_error("");
		}

		fences.push_back(std::move(fence));
		isSubmittedList.push_back(false);
	}
}

void CommandQueueManager::deallocateResources()
{
	for(auto& deallocatorsList : deallocators)
	{
		for(auto& deallocator : deallocatorsList)
		{
			deallocator();
		}
	}
}
