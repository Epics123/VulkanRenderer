#pragma once

#include "Renderer/Window.h"
#include "../Common/Defines.h"
#include "PhysicalDevice.h"
#include "CommandQueueManager.h"

#include <string>
#include <vector>
#include <memory>
#include <any>

//#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices
{
    uint32_t graphicsFamily;
    uint32_t presentFamily;
    bool graphicsFamilyHasValue = false;
    bool presentFamilyHasValue = false;
    bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
};

template <size_t CHAIN_SIZE = 10>
class VulkanFeatureChain
{
public:
	VulkanFeatureChain() = default;
	MOVABLE_ONLY(VulkanFeatureChain);

    template<typename T>
	auto& pushBack(T nextVulkanChainStruct)
	{
		ASSERT(currentIndex_ < CHAIN_SIZE, "Chain is full");
		data_[currentIndex_] = nextVulkanChainStruct;

        // TODO: Probably don't need to be casting here anymore
		auto& next = std::any_cast<decltype(nextVulkanChainStruct)&>(data_[currentIndex_]);

		next.pNext = std::exchange(firstNext_, &next);
		currentIndex_++;

		return next;
	}

	[[nodiscard]] void* firstNextPtr() const { return firstNext_; };

private:
	std::array<std::any, CHAIN_SIZE> data_;
	VkBaseInStructure* root_ = nullptr;
	int currentIndex_ = 0;
	void* firstNext_ = VK_NULL_HANDLE;
};

struct PhysicalDeviceFeatures
{
    PhysicalDeviceFeatures()
    {
        physicalDeviceFeatures = {};
        physicalDeviceFeatures.independentBlend = VK_TRUE;
        physicalDeviceFeatures.vertexPipelineStoresAndAtomics = VK_TRUE;
        physicalDeviceFeatures.fragmentStoresAndAtomics = VK_TRUE;

        vulkan11Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;

        vulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;

        vullkan13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;

        accelStructFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;

        rayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR,

        rayQueryFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;

        multiviewFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES;

        fragmentDensityMapFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_FEATURES_EXT;
        fragmentDensityMapOffsetFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_OFFSET_FEATURES_QCOM;
    }

	VkPhysicalDeviceFeatures physicalDeviceFeatures;
	VkPhysicalDeviceVulkan11Features vulkan11Features{};
	VkPhysicalDeviceVulkan12Features vulkan12Features{};
    VkPhysicalDeviceVulkan13Features vullkan13Features{};

	VkPhysicalDeviceAccelerationStructureFeaturesKHR accelStructFeatures{};
	VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures{};
	VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures{};
	VkPhysicalDeviceMultiviewFeatures multiviewFeatures{};
	VkPhysicalDeviceFragmentDensityMapFeaturesEXT fragmentDensityMapFeatures{};
    VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM fragmentDensityMapOffsetFeatures{};
};

class Texture;
class Buffer;
struct TextureCreationInfo;

class Context
{
public:
#ifdef _DEBUG
    const bool enableValidationLayers = true;
#else
    const bool enableValidationLayers = false;
#endif

    Context(Window& window, VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT);
    ~Context();

    // Not copyable or movable
	//MOVABLE_ONLY(Context)

    VkCommandPool getCommandPool() { return commandPool; }
    VkDevice getDevice() const { return device_; }
    VkPhysicalDevice getRawPhysicalDevice() { return physicalDevice; }
    VkSurfaceKHR surface() { return surface_; }
    VkQueue graphicsQueue() { return graphicsQueue_; }
    VkQueue presentQueue() { return presentQueue_; }
    VkInstance getInstance() { return instance; }

    SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(physicalDevice); }
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(physicalDevice); }
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    // Buffer Helper Functions
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);

    void createImageWithInfo(const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    void transitionImageLayout(VkImage& image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

    // DEFERRED_RENDERING_REWORK

    const PhysicalDevice& getPhysicalDevice() { return physicalDevice_; }

    void createSwapchain(VkFormat format, VkSurfaceFormatKHR surfaceFormat, VkPresentModeKHR presentMode, const VkExtent2D& extent);
    void recreateSwapchain(const VkExtent2D& extent);
    void clearSwapchain();

    class Swapchain* getSwapchain() { return swapchain.get(); }
    inline VmaAllocator getMemoryAllocator() const { return allocator; }

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, VkPresentModeKHR desiredPresentMode);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D windowExtent);

    std::unique_ptr<CommandQueueManager> createGraphicsCommandQueue(uint32_t count, uint32_t numConcurrentCommands, const std::string& name, int graphicsQueueIndex = -1);

    std::shared_ptr<class RenderPass> createRenderPass(const std::vector<struct RenderPassInitInfo>& initInfos, 
                                                       const std::vector<std::shared_ptr<class Texture>>& resolveAttachments = {}, const std::string& name = "");

    std::shared_ptr<Texture> createTexture(const TextureCreationInfo& createInfo);

    std::shared_ptr<Buffer> createPersistantBuffer(size_t size, VkBufferUsageFlags flags, const std::string& name);

    static void endableDefaultFeatures();
    static void enableIndirectRenderingFeature();
    static void enableSyncronizationFeature();
    static void enableBufferDeviceAddressFeature();

    // END_DEFERRED_RENDERING_REWORK

    VkPhysicalDeviceProperties properties;

private:
    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createCommandPool();

    // helper functions
    bool isDeviceSuitable(VkPhysicalDevice device);
    std::vector<const char*> getRequiredExtensions();
    bool checkValidationLayerSupport();
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    void hasGflwRequiredInstanceExtensions();
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    // DEFERRED_RENDERING_REWORK
    void resizeQueues();

    void createMemoryAllocator();
    // END_DEFERRED_RENDERING_REWORK

private:
    // DEFERRED_RENDERING_REWORK

	VkApplicationInfo applicationInfo;

    PhysicalDevice physicalDevice_;
    static PhysicalDeviceFeatures physicalDeviceFeatures;

    VkQueueFlags requestedQueues;

    VmaAllocator allocator;

    std::unique_ptr<class Swapchain> swapchain;
    // END_DEFERRED_RENDERING_REWORK

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    Window& window;
    VkCommandPool commandPool;

    VkDevice device_;
    VkSurfaceKHR surface_;
    VkQueue graphicsQueue_;

    // the main queue
    VkQueue presentQueue_;

    // Optional queues
	std::vector<VkQueue> graphicsQueues;
	std::vector<VkQueue> computeQueues;
	std::vector<VkQueue> transferQueues;

    bool shouldSupportRayTracing = false;
    VkPresentModeKHR defaultPresentMode = VK_PRESENT_MODE_FIFO_KHR;

    const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
    const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_EXT_MEMORY_BUDGET_EXTENSION_NAME }; //, VK_KHR_RAY_QUERY_EXTENSION_NAME, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME };
	const std::vector<const char*> requestedInstanceExtensions = { VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME };
};