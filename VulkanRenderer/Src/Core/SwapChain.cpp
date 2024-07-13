#include "SwapChain.h"
#include "Context.h"
#include "PhysicalDevice.h"
#include "Texture.h"
#include "Log.h"

#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>

// DEFERRED RENDERING REFACTOR

Swapchain::Swapchain(const Context& context, const PhysicalDevice& physicalDevice, VkSurfaceKHR surface, VkQueue presentQueue, 
                     VkSurfaceFormatKHR surfaceFormat, VkPresentModeKHR presentMode, VkExtent2D extent, const std::string& name /*= ""*/)
    :device{context.getDevice()}, presentQueue{presentQueue}, extent{extent}
{
    createSwapchain(context, physicalDevice, surface, surfaceFormat.format, surfaceFormat.colorSpace, presentMode, extent);
}


Swapchain::~Swapchain()
{
   VkResult result = vkWaitForFences(device, 1, &acquireFence, VK_TRUE, UINT64_MAX);
   if(result != VK_SUCCESS)
   {
	   CORE_CRITICAL("Failed to wait for fence! Error code: {0}", result);
	   throw std::runtime_error("");
   }

   vkDestroyFence(device, acquireFence, nullptr);
   vkDestroySemaphore(device, imageRendered, nullptr);
   vkDestroySemaphore(device, imageAvailable, nullptr);
   vkDestroySwapchainKHR(device, swapchain, nullptr);
}

void Swapchain::createSwapchain(const Context& context, const PhysicalDevice& physicalDevice, VkSurfaceKHR surface, VkFormat imageFormat, 
                                VkColorSpaceKHR imageColorSpace, VkPresentModeKHR presentMode, VkExtent2D extent)
{
	const uint32_t minImageCount = physicalDevice.getSurfaceCapabilities().minImageCount;
	const uint32_t numImages = std::clamp(minImageCount + 1, minImageCount, physicalDevice.getSurfaceCapabilities().maxImageCount);
    
   const std::optional<uint32_t> presentationFamilyIndex = physicalDevice.getPresentationFamilyIndex();
   ASSERT(presentationFamilyIndex.has_value(), "There are no presentation queues available for the swapchain!");

   const bool isPresentationQueueShared = physicalDevice.getGraphicsFamilyIndex().value() == presentationFamilyIndex.value();

   std::array<uint32_t, 2> familyIndices{physicalDevice.getGraphicsFamilyIndex().value(), presentationFamilyIndex.value()};

   VkSwapchainCreateInfoKHR createInfo;
   createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
   createInfo.flags = VkSwapchainCreateFlagsKHR();
   createInfo.surface = surface;
   createInfo.minImageCount = numImages;
   createInfo.imageFormat = imageFormat;
   createInfo.imageColorSpace = imageColorSpace;
   createInfo.imageExtent = extent;
   createInfo.imageArrayLayers = 1;
   createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
   createInfo.imageSharingMode = isPresentationQueueShared ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
   createInfo.queueFamilyIndexCount = isPresentationQueueShared ? 0u : 2u;
   createInfo.pQueueFamilyIndices = isPresentationQueueShared ? nullptr : familyIndices.data();
   createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
   createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
   createInfo.presentMode = presentMode;
   createInfo.clipped = VK_TRUE;
   createInfo.oldSwapchain = VK_NULL_HANDLE;
   createInfo.pNext = VK_NULL_HANDLE;

   VkResult result = vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain);
   if(result != VK_SUCCESS)
   {
	    CORE_CRITICAL("Failed to create swapchain! Error code: {0}", result);
        throw std::runtime_error("");
   }

   createSwaphainImages(context, imageFormat, extent);
   createSemaphores();
   createFence();
}

void Swapchain::createSwaphainImages(const Context& context, VkFormat imageFormat, const VkExtent2D& extent)
{
	// we only specified a minimum number of images in the swap chain, so the implementation is
	// allowed to create a swap chain with more. That's why we'll first query the final number of
	// images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
	// retrieve the handles.
    uint32_t imageCount = 0;
	vkGetSwapchainImagesKHR(context.getDevice(), swapchain, &imageCount, nullptr);
    std::vector<VkImage> images(imageCount);
	vkGetSwapchainImagesKHR(context.getDevice(), swapchain, &imageCount, images.data());

    swapchainImages.reserve(imageCount);
    for(size_t i = 0; i < imageCount; ++i)
    {
       VkExtent3D extents;
       extents.width = extent.width;
       extents.height = extent.height;
       extents.depth = 1;

       const std::string debugName = "Swapchain image " + std::to_string(i);

       std::shared_ptr<Texture> swapchainImage = std::make_shared<Texture>(context, device, images[i], imageFormat, extents, 1, false, debugName);

       swapchainImages.emplace_back(swapchainImage);
    }
}

void Swapchain::createSemaphores()
{
    VkSemaphoreCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    createInfo.pNext = VK_NULL_HANDLE;
    createInfo.flags = 0;

    VkResult result = vkCreateSemaphore(device, &createInfo, nullptr, &imageAvailable);
    if(result != VK_SUCCESS)
    {
		CORE_CRITICAL("Failed to create image available semaphore! Error code: {0}", result);
		throw std::runtime_error("");
    }

    result = vkCreateSemaphore(device, &createInfo, nullptr, &imageRendered);
	if (result != VK_SUCCESS)
	{
		CORE_CRITICAL("Failed to create image rendered semaphore! Error code: {0}", result);
		throw std::runtime_error("");
	}
}

void Swapchain::createFence()
{
    VkFenceCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    createInfo.flags = 0;
    createInfo.pNext = VK_NULL_HANDLE;

    VkResult result = vkCreateFence(device, &createInfo, nullptr, &acquireFence);
    if(result != VK_SUCCESS)
    {
		CORE_CRITICAL("Failed to create image aquire fence! Error code: {0}", result);
		throw std::runtime_error("");
    }
}

// END DEFERRED RENDERING REFACTOR

SwapChain::SwapChain(Context& deviceRef, VkExtent2D windowExtent)
    : context{deviceRef}, windowExtent{ windowExtent }
{
    init();
}

SwapChain::SwapChain(Context& deviceRef, VkExtent2D windowExtent, std::shared_ptr<SwapChain> previousSwapChain)
	: context{ deviceRef }, windowExtent{ windowExtent }, oldSwapChain{ previousSwapChain }
{
	init();
	oldSwapChain = nullptr;
}

SwapChain::SwapChain(const Context& inContext, const PhysicalDevice& physicalDevice, VkSurfaceKHR surface, VkQueue inPresentQueue, 
                     VkFormat imageFormat, VkColorSpaceKHR imageColorSpace, VkPresentModeKHR presentMode, VkExtent2D inExtent, const std::string& name)
    :context{const_cast<Context&>(inContext)}, device{ inContext.getDevice() }, presentQueue{inPresentQueue}, extent{ inExtent }
{
    createSwapChain();
}

void SwapChain::init()
{
    createSwapChain();
    createRenderPass();
    createSyncObjects();
}

SwapChain::~SwapChain()
{
    renderPass.cleanup(context);

    if (swapChain != nullptr)
    {
        vkDestroySwapchainKHR(context.getDevice(), swapChain, nullptr);
        swapChain = nullptr;
    }

    // cleanup synchronization objects
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(context.getDevice(), renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(context.getDevice(), imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(context.getDevice(), inFlightFences[i], nullptr);
    }
}

VkResult SwapChain::acquireNextImage(uint32_t* imageIndex)
{
    vkWaitForFences(
        context.getDevice(),
        1,
        &inFlightFences[currentFrame],
        VK_TRUE,
        std::numeric_limits<uint64_t>::max());

    VkResult result = vkAcquireNextImageKHR(
        context.getDevice(),
        swapChain,
        std::numeric_limits<uint64_t>::max(),
        imageAvailableSemaphores[currentFrame],  // must be a not signaled semaphore
        VK_NULL_HANDLE,
        imageIndex);

    return result;
}

VkResult SwapChain::submitCommandBuffers(
    const VkCommandBuffer* buffers, uint32_t* imageIndex)
{
    if (imagesInFlight[*imageIndex] != VK_NULL_HANDLE)
    {
        vkWaitForFences(context.getDevice(), 1, &imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
    }
    imagesInFlight[*imageIndex] = inFlightFences[currentFrame];

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = buffers;

    VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(context.getDevice(), 1, &inFlightFences[currentFrame]);
    if (vkQueueSubmit(context.graphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) !=
        VK_SUCCESS)
    {
        CORE_CRITICAL("Failed to submit draw command buffer!")
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = imageIndex;

    auto result = vkQueuePresentKHR(context.presentQueue(), &presentInfo);

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    return result;
}

// DEFERRED RENDERING REFACTOR

void SwapChain::createSwapChain(const PhysicalDevice& physicalDevice)
{
	const uint32_t minImageCount = physicalDevice.getSurfaceCapabilities().minImageCount;
	const uint32_t numImages = std::clamp(minImageCount + 1, minImageCount, physicalDevice.getSurfaceCapabilities().maxImageCount);
}

// END DEFERRED RENDERING REFACTOR

void SwapChain::createSwapChain()
{
    SwapChainSupportDetails swapChainSupport = context.getSwapChainSupport();

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = context.surface();

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    renderPass.setImageFormat(surfaceFormat.format);
    renderPass.setDepthFormat(findDepthFormat());
    renderPass.setShouldDestroyColorImage(false);

    QueueFamilyIndices indices = context.findPhysicalQueueFamilies();
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };

    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;      // Optional
        createInfo.pQueueFamilyIndices = nullptr;  // Optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = oldSwapChain == nullptr ? VK_NULL_HANDLE : oldSwapChain->swapChain;

    if (vkCreateSwapchainKHR(context.getDevice(), &createInfo, nullptr, &swapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swap chain!");
    }

    // we only specified a minimum number of images in the swap chain, so the implementation is
    // allowed to create a swap chain with more. That's why we'll first query the final number of
    // images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
    // retrieve the handles.
    vkGetSwapchainImagesKHR(context.getDevice(), swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    renderPass.colors.resize(imageCount);
    vkGetSwapchainImagesKHR(context.getDevice(), swapChain, &imageCount, swapChainImages.data());

    for(uint32_t i = 0; i < (uint32_t)swapChainImages.size(); i++)
    {
        renderPass.colors[i].image = swapChainImages[i];
    }

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void SwapChain::createRenderPass()
{
    renderPass.createRenderPass(context, getWidth(), getHeight());
}

void SwapChain::createSyncObjects()
{
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(imageCount(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(context.getDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(context.getDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(context.getDevice(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR SwapChain::chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    /*for (const auto& availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            std::cout << "Present mode: Mailbox" << std::endl;
            return availablePresentMode;
        }
    }*/

    CORE_INFO("Present Mode: V-Sync")
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        VkExtent2D actualExtent = windowExtent;
        actualExtent.width = std::max(
            capabilities.minImageExtent.width,
            std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(
            capabilities.minImageExtent.height,
            std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

VkFormat SwapChain::findDepthFormat()
{
    return context.findSupportedFormat(
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}
