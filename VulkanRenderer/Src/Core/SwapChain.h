#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>
#include <memory>

//#include "Context.h"
#include "RenderPass.h"

class Context;
class PhysicalDevice;
class Texture;

// DEFERRED RENDERING REFACTOR

class Swapchain final
{
public:
    explicit Swapchain() = default;
    explicit Swapchain(const Context& context, const PhysicalDevice& physicalDevice, VkSurfaceKHR surface, VkQueue presentQueue, 
                       VkSurfaceFormatKHR surfaceFormat, VkPresentModeKHR presentMode, VkExtent2D extent, const std::string& name = "");

    ~Swapchain();

    uint32_t getNumImages() const { return static_cast<uint32_t>(swapchainImages.size()); }

private:
    void createSwapchain(const Context& context, const PhysicalDevice& physicalDevice, VkSurfaceKHR surface, VkFormat imageFormat, 
                         VkColorSpaceKHR imageColorSpace, VkPresentModeKHR presentMode, VkExtent2D extent);

    void createSwaphainImages(const Context& context, VkFormat imageFormat, const VkExtent2D& extent);
    void createSemaphores();
    void createFence();

private:
	VkDevice device = VK_NULL_HANDLE;
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	VkQueue presentQueue = VK_NULL_HANDLE;
	std::vector<std::shared_ptr<Texture>> swapchainImages;
	VkSemaphore imageAvailable = VK_NULL_HANDLE;
	VkSemaphore imageRendered = VK_NULL_HANDLE;
	uint32_t imageIndex = 0;
	VkExtent2D extent;
	VkFormat imageFormat;
	VkFence acquireFence = VK_NULL_HANDLE;
};

// END DEFERRED RENDERING REFACTOR

class SwapChain
{
public:
	static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    explicit SwapChain() = default;
    explicit SwapChain(const Context& inContext, const PhysicalDevice& physicalDevice, VkSurfaceKHR surface, 
                       VkQueue inPresentQueue, VkFormat imageFormat, VkColorSpaceKHR imageColorSpace, 
                       VkPresentModeKHR presentMode, VkExtent2D inExtent, const std::string& name = "");

    SwapChain(Context& deviceRef, VkExtent2D windowExtent);
    SwapChain(Context& deviceRef, VkExtent2D windowExtent, std::shared_ptr<SwapChain> previousSwapChain);
    ~SwapChain();

    //SwapChain(const SwapChain&) = delete;
    //SwapChain& operator=(const SwapChain&) = delete;

    RenderPass& getRenderPass() { return renderPass; }
    size_t imageCount() { return swapChainImages.size(); }
    VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
    VkExtent2D getSwapChainExtent() { return swapChainExtent; }
    uint32_t getWidth() { return swapChainExtent.width; }
    uint32_t getHeight() { return swapChainExtent.height; }

    float extentAspectRatio()
    {
        return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
    }
    VkFormat findDepthFormat();

    VkResult acquireNextImage(uint32_t* imageIndex);
    VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

    bool compareSwapFormats(const SwapChain& swapChain) const
    {
        return swapChain.swapChainDepthFormat == swapChainDepthFormat && swapChain.swapChainImageFormat == swapChainImageFormat;
    }

private:
    void init();
    void createSwapChain();
    void createImageViews();
    void createRenderPass();
    void createSyncObjects();

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    // DEFERRED RENDERING REFACTOR
    void createSwapChain(const PhysicalDevice& physicalDevice);

    // END DEFERRED RENDERING REFACTOR

private:
	// DEFERRED RENDERING REFACTOR

	VkDevice device = VK_NULL_HANDLE;
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	VkQueue presentQueue = VK_NULL_HANDLE;
	std::vector<std::shared_ptr<Texture_>> SwapchainImages;
	VkSemaphore imageAvailable = VK_NULL_HANDLE;
	VkSemaphore imageRendered = VK_NULL_HANDLE;
	uint32_t imageIndex = 0;
	VkExtent2D extent;
	VkFormat imageFormat;
	VkFence acquireFence = VK_NULL_HANDLE;

	// END DEFERRED RENDERING REFACTOR

    VkFormat swapChainImageFormat;
    VkFormat swapChainDepthFormat;
    VkExtent2D swapChainExtent;

    RenderPass renderPass;
	std::vector<VkImage> swapChainImages;

    Context& context;
    VkExtent2D windowExtent;

    VkSwapchainKHR swapChain;
    std::shared_ptr<SwapChain> oldSwapChain;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    size_t currentFrame = 0;
};