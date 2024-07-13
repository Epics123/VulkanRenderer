#include "Context.h"
#include "Log.h"

#include "SwapChain.h"
#include "RenderPass.h"

#include <cstring>
#include <iostream>
#include <set>
#include <unordered_set>
#include <algorithm>

// local callback functions
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    CORE_ERROR("Validaiton Layer: {0}", pCallbackData->pMessage)

    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance,
        "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance,
        "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

PhysicalDeviceFeatures Context::physicalDeviceFeatures = PhysicalDeviceFeatures();

// class member functions
Context::Context(Window& window, VkQueueFlags requestedQueueTypes) 
    : window{ window }, requestedQueues{requestedQueueTypes}
{
	createInstance();
	setupDebugMessenger();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();

	//createCommandPool();
}

Context::~Context()
{
    vkDeviceWaitIdle(device_);
    swapchain.reset();

    vkDestroyCommandPool(device_, commandPool, nullptr);
    vkDestroyDevice(device_, nullptr);

    if(surface_ != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(instance, surface_, nullptr);
    }

	if (enableValidationLayers)
	{
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}

    vkDestroyInstance(instance, nullptr);
}

void Context::createInstance()
{
    if (enableValidationLayers && !checkValidationLayerSupport())
    {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "VulkanRenderer";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create instance!");
    }

    hasGflwRequiredInstanceExtensions();
}

void Context::pickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0)
    {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }
    CORE_INFO("Device Count: {0}", deviceCount)
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto& device : devices)
    {
        if (isDeviceSuitable(device))
        {
            physicalDevice = device;
			// DEFERRED_RENDERING_REWORK

            physicalDevice_ = PhysicalDevice(device, surface_);

	        // END_DEFERRED_RENDERING_REWORK
            break;
        }
    }

    // DEFERRED_RENDERING_REWORK

    if(!physicalDevice_.isDeviceValid())
    {
        throw std::runtime_error("Failed to find a suitable GPU!");
    }

    CORE_INFO("Physical Device: {0}", physicalDevice_.getDeviceProperties().deviceName);

    // Always request a graphics queue
    physicalDevice_.reserveQueues(requestedQueues | VK_QUEUE_GRAPHICS_BIT, surface_);

    // END_DEFERRED_RENDERING_REWORK

    /*if (physicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("failed to find a suitable GPU!");
    }

    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    CORE_INFO("Physical Device: {0}", properties.deviceName)*/
}

void Context::createLogicalDevice()
{
    // DEFERRED_RENDERING_REWORK
    const auto familyIndices = physicalDevice_.findQueueFamilies();

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::vector<std::vector<float>> prioritiesForAllFamilies(familyIndices.size());

    size_t index = 0;
    for(QueueFamilyPair queueFamily : familyIndices)
    {
        const uint32_t queueFamilyIndex = queueFamily.first;
        const uint32_t queueCount = queueFamily.second;

        prioritiesForAllFamilies[index] = std::vector<float>(queueCount, 1.0f);

        VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
		queueCreateInfo.queueCount = queueCount;
		queueCreateInfo.pQueuePriorities = prioritiesForAllFamilies[index].data();
		queueCreateInfos.emplace_back(queueCreateInfo);

        ++index;
    }

    VkPhysicalDeviceFeatures2 deviceFeatures{};
    deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    deviceFeatures.features = physicalDeviceFeatures.physicalDeviceFeatures;

    VulkanFeatureChain<> featureChain;
    featureChain.pushBack(deviceFeatures);
    featureChain.pushBack(physicalDeviceFeatures.vulkan11Features);
    featureChain.pushBack(physicalDeviceFeatures.vulkan12Features);

	if (physicalDevice_.isRayTracingSupported() && shouldSupportRayTracing)
	{
		featureChain.pushBack(physicalDeviceFeatures.accelStructFeatures);
		featureChain.pushBack(physicalDeviceFeatures.rayTracingPipelineFeatures);
		featureChain.pushBack(physicalDeviceFeatures.rayQueryFeatures);
	}

	if (physicalDevice_.isMultiviewSupported())
	{
		physicalDeviceFeatures.vulkan11Features.multiview = VK_TRUE;
	}

	if (physicalDevice_.isFragmentDensityMapSupported())
	{
		featureChain.pushBack(physicalDeviceFeatures.fragmentDensityMapFeatures);
	}

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = featureChain.firstNextPtr();
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (enableValidationLayers)
	{
        deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
	}

    VkResult result = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device_);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create logical device!");
	}

    resizeQueues();

    // END_DEFERRED_RENDERING_REWORK

 //   QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

 //   std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
 //   std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

 //   float queuePriority = 1.0f;
 //   for (uint32_t queueFamily : uniqueQueueFamilies)
 //   {
 //       VkDeviceQueueCreateInfo queueCreateInfo = {};
 //       queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
 //       queueCreateInfo.queueFamilyIndex = queueFamily;
 //       queueCreateInfo.queueCount = 1;
 //       queueCreateInfo.pQueuePriorities = &queuePriority;
 //       queueCreateInfos.push_back(queueCreateInfo);
 //   }

	//VkPhysicalDeviceDescriptorIndexingFeatures physicalDeviceDescriptorIndexingFeatures{};
	//physicalDeviceDescriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
	//physicalDeviceDescriptorIndexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
	//physicalDeviceDescriptorIndexingFeatures.runtimeDescriptorArray = VK_TRUE;
	//physicalDeviceDescriptorIndexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;
 //   physicalDeviceDescriptorIndexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
 //   physicalDeviceDescriptorIndexingFeatures.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;

 //   VkPhysicalDeviceFeatures deviceFeatures = {};
 //   deviceFeatures.samplerAnisotropy = VK_TRUE;
 //   deviceFeatures.fillModeNonSolid = VK_TRUE;
 //   deviceFeatures.shaderSampledImageArrayDynamicIndexing = VK_TRUE;

 //   VkPhysicalDeviceFeatures2 deviceFeatures2{};
 //   deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
 //   deviceFeatures2.features = deviceFeatures;
 //   deviceFeatures2.pNext = &physicalDeviceDescriptorIndexingFeatures;

 //   VkDeviceCreateInfo createInfo = {};
 //   createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

 //   createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
 //   createInfo.pQueueCreateInfos = queueCreateInfos.data();

 //   //createInfo.pEnabledFeatures = &deviceFeatures;
 //   createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
 //   createInfo.ppEnabledExtensionNames = deviceExtensions.data();
 //   createInfo.pNext = &deviceFeatures2;

 //   // might not really be necessary anymore because device specific validation layers
 //   // have been deprecated
 //   if (enableValidationLayers)
 //   {
 //       createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
 //       createInfo.ppEnabledLayerNames = validationLayers.data();
 //   }
 //   else
 //   {
 //       createInfo.enabledLayerCount = 0;
 //   }

 //   if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device_) != VK_SUCCESS)
 //   {
 //       throw std::runtime_error("failed to create logical device!");
 //   }

 //   vkGetDeviceQueue(device_, indices.graphicsFamily, 0, &graphicsQueue_);
 //   vkGetDeviceQueue(device_, indices.presentFamily, 0, &presentQueue_);
}

void Context::createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = findPhysicalQueueFamilies();

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
    poolInfo.flags =
        VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool!");
    }
}

void Context::createSurface() { window.createWindowSurface(instance, &surface_); }

bool Context::isDeviceSuitable(VkPhysicalDevice device)
{
    QueueFamilyIndices indices = findQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return indices.isComplete() && extensionsSupported && swapChainAdequate &&
        supportedFeatures.samplerAnisotropy;
}

void Context::populateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;  // Optional
}

void Context::setupDebugMessenger()
{
    if (!enableValidationLayers) return;
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);
    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

bool Context::checkValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers)
    {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            return false;
        }
    }

    return true;
}

std::vector<const char*> Context::getRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

void Context::hasGflwRequiredInstanceExtensions()
{
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    CORE_INFO("Available Extensions:")
    std::unordered_set<std::string> available;
    for (const auto& extension : extensions)
    {
        CORE_INFO("\t{0}", extension.extensionName)
        available.insert(extension.extensionName);
    }

    CORE_WARN("Required Extensions:")
    auto requiredExtensions = getRequiredExtensions();
    for (const auto& required : requiredExtensions)
    {
        CORE_WARN("\t{0}", required)
        if (available.find(required) == available.end())
        {
            throw std::runtime_error("Missing required glfw extension");
        }
    }
}

bool Context::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(
        device,
        nullptr,
        &extensionCount,
        availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

QueueFamilyIndices Context::findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
            indices.graphicsFamilyHasValue = true;
        }
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &presentSupport);
        if (queueFamily.queueCount > 0 && presentSupport)
        {
            indices.presentFamily = i;
            indices.presentFamilyHasValue = true;
        }
        if (indices.isComplete())
        {
            break;
        }

        i++;
    }

    return indices;
}

SwapChainSupportDetails Context::querySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            device,
            surface_,
            &presentModeCount,
            details.presentModes.data());
    }
    return details;
}

void Context::resizeQueues()
{
    if(physicalDevice_.getGraphicsFamilyIndex().has_value() && physicalDevice_.graphicsFamilyCount() > 0)
    {
        graphicsQueues.resize(physicalDevice_.graphicsFamilyCount(), VK_NULL_HANDLE);
        for(size_t i = 0; i < graphicsQueues.size(); i++)
        {
            vkGetDeviceQueue(device_, physicalDevice_.getGraphicsFamilyIndex().value(), uint32_t(i), &graphicsQueues[i]);
        }
    }
    if(physicalDevice_.getComputeFamilyIndex().has_value() && physicalDevice_.computeFamilyCount() > 0)
    {
        computeQueues.resize(physicalDevice_.computeFamilyCount(), VK_NULL_HANDLE);
		for (size_t i = 0; i < computeQueues.size(); i++)
		{
			vkGetDeviceQueue(device_, physicalDevice_.getComputeFamilyIndex().value(), uint32_t(i), &computeQueues[i]);
		}
    }
    if(physicalDevice_.getTransferFamilyIndex().has_value() && physicalDevice_.transferFamilyCount() > 0)
    {
        transferQueues.resize(physicalDevice_.transferFamilyCount(), VK_NULL_HANDLE);
		for (size_t i = 0; i < transferQueues.size(); i++)
		{
			vkGetDeviceQueue(device_, physicalDevice_.getTransferFamilyIndex().value(), uint32_t(i), &transferQueues[i]);
		}
    }
    if(physicalDevice_.getPresentationFamilyIndex().has_value())
    {
        vkGetDeviceQueue(device_, physicalDevice_.getPresentationFamilyIndex().value(), 0, &presentQueue_);
    }
}

VkFormat Context::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
        {
            return format;
        }
        else if (
            tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }
    throw std::runtime_error("failed to find supported format!");
}

uint32_t Context::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void Context::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device_, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create vertex buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device_, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device_, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate vertex buffer memory!");
    }

    vkBindBufferMemory(device_, buffer, bufferMemory, 0);
}

VkCommandBuffer Context::beginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device_, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    return commandBuffer;
}

void Context::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue_, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue_);

    vkFreeCommandBuffers(device_, commandPool, 1, &commandBuffer);
}

void Context::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;  // Optional
    copyRegion.dstOffset = 0;  // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}

void Context::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = layerCount;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { width, height, 1 };

    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region);
    endSingleTimeCommands(commandBuffer);
}

void Context::createImageWithInfo(const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
    if (vkCreateImage(device_, &imageInfo, nullptr, &image) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device_, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device_, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate image memory!");
    }

    if (vkBindImageMemory(device_, image, imageMemory, 0) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to bind image memory!");
    }
}

void Context::transitionImageLayout(VkImage& image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkCommandBuffer cmdBuffer = beginSingleTimeCommands();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	VkPipelineStageFlags srcStage;
	VkPipelineStageFlags dstStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else
	{
		throw std::invalid_argument("Unsupported layout transition");
	}

	vkCmdPipelineBarrier(
        cmdBuffer,
        srcStage,
        dstStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

    endSingleTimeCommands(cmdBuffer);
}

// DEFERRED_RENDERING_REWORK

void Context::createSwapchain(VkFormat format, VkSurfaceFormatKHR surfaceFormat, VkPresentModeKHR presentMode, const VkExtent2D& extent)
{
    ASSERT(surface_ != VK_NULL_HANDLE, "Trying to create a swapchain without a surface! The context must be provided a valid surface to create a swapchain.")
    swapchain = std::make_unique<Swapchain>(*this, physicalDevice_, surface_, presentQueue_, surfaceFormat, presentMode, extent);
}


VkSurfaceFormatKHR Context::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR Context::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, VkPresentModeKHR desiredPresentMode)
{
    for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == desiredPresentMode)
		{
            switch (availablePresentMode)
            {
            case VK_PRESENT_MODE_FIFO_KHR:
                CORE_INFO("Present Mode: V-Sync");
                break;
            default:
                CORE_INFO("Present Mode: {0}", availablePresentMode);
                break;
            }

			return availablePresentMode;
		}
	}

    CORE_INFO("Present Mode: V-Sync");
    return defaultPresentMode;
}

VkExtent2D Context::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D windowExtent)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	else
	{
		VkExtent2D actualExtent = windowExtent;
		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

CommandQueueManager Context::createGraphicsCommandQueue(uint32_t count, uint32_t numConcurrentCommands, const std::string& name, int graphicsQueueIndex)
{
    if(graphicsQueueIndex != -1)
    {
        ASSERT(graphicsQueueIndex < graphicsQueues.size(), "Not enough graphics queues available, specify a smaller queue index");
    }

    const uint32_t graphicsFamilyIndex = physicalDevice_.getGraphicsFamilyIndex().value();
    const VkQueue queue = graphicsQueueIndex != -1 ? graphicsQueues[graphicsQueueIndex] : graphicsQueues[0];

    return CommandQueueManager(*this, device_, count, numConcurrentCommands, graphicsFamilyIndex, queue, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, name);
}

std::shared_ptr<class FRenderPass> Context::createRenderPass(const std::vector<RenderPassInitInfo>& initInfos, const std::vector<std::shared_ptr<class Texture_>>& resolveAttachments)
{
    return std::make_shared<FRenderPass>(*this, initInfos, resolveAttachments);
}

void Context::endableDefaultFeatures()
{
	physicalDeviceFeatures.vulkan12Features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
	physicalDeviceFeatures.vulkan12Features.shaderStorageImageArrayNonUniformIndexing = VK_TRUE;

	physicalDeviceFeatures.vulkan12Features.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
	physicalDeviceFeatures.vulkan12Features.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
	physicalDeviceFeatures.vulkan12Features.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;
	physicalDeviceFeatures.vulkan12Features.descriptorBindingPartiallyBound = VK_TRUE;
	physicalDeviceFeatures.vulkan12Features.descriptorBindingVariableDescriptorCount = VK_TRUE;
	physicalDeviceFeatures.vulkan12Features.descriptorIndexing = VK_TRUE;
	physicalDeviceFeatures.vulkan12Features.runtimeDescriptorArray = VK_TRUE;
}

void Context::enableIndirectRenderingFeature()
{
    physicalDeviceFeatures.vulkan11Features.shaderDrawParameters = VK_TRUE;
    physicalDeviceFeatures.vulkan12Features.drawIndirectCount = VK_TRUE;
    physicalDeviceFeatures.physicalDeviceFeatures.multiDrawIndirect = VK_TRUE;
    physicalDeviceFeatures.physicalDeviceFeatures.drawIndirectFirstInstance = VK_TRUE;
}

void Context::enableSyncronizationFeature()
{
    physicalDeviceFeatures.vullkan13Features.synchronization2 = VK_TRUE;
}

void Context::enableBufferDeviceAddressFeature()
{
    physicalDeviceFeatures.vulkan12Features.bufferDeviceAddress;
    physicalDeviceFeatures.vulkan12Features.bufferDeviceAddressCaptureReplay = VK_TRUE;
}

// END_DEFERRED_RENDERING_REWORK