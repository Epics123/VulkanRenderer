#include "PhysicalDevice.h"
#include "Utils/Utils.h"

#include <algorithm>
#include <iterator>
#include <set>

PhysicalDevice::PhysicalDevice(VkPhysicalDevice device, VkSurfaceKHR surface, bool enableRayTracing)
	:physicalDevice{device}
{
	initFeatures();
	initProperties();

	memoryProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
	//vkGetPhysicalDeviceMemoryProperties2(physicalDevice, &memoryProperties);

	{
		uint32_t propertyCount = 0;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &propertyCount, nullptr);
		
		std::vector<VkExtensionProperties> properties(propertyCount);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &propertyCount, properties.data());
	}

	{
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
		queueFamilyProperties.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());
	}

	if(surface != VK_NULL_HANDLE)
	{
		enumerateSurfaceFormats(surface);
		enumerateSurfaceCapabilities(surface);
		enumeratePresentationModes(surface);
	}
}

void PhysicalDevice::reserveQueues(VkQueueFlags requestedQueueTypes, VkSurfaceKHR surface)
{
	/*Below is from The Modern Vulkan Cookbook:
	* - Only share queues with presentation, vulkan queues can support multiple 
	*	operations (graphics, compute, transfer, etc).
	* - If supporting multiple operations, a queue can only be used on one thread
	* - The below code treats each queue as independent, and it can only be used for
	*	either graphics/compute/transfer or sparse. This should help w/ multithreading
	* - If the device only has one queue for everything, then we may nor be able to 
	*	create compute/transfer queues
	*/

	ASSERT(requestedQueueTypes > 0, "Requested queue types cannot be empty!");

	for(size_t queueFamilyIndex = 0; queueFamilyIndex < queueFamilyProperties.size(); ++queueFamilyIndex)
	{
		VkQueueFlags queueFamilyPropertiesFlags = queueFamilyProperties[queueFamilyIndex].queueFlags;

		// Present queue
		if(!presentationFamilyIndex.has_value() && surface != VK_NULL_HANDLE)
		{
			VkBool32 bSupportsPresetQueue = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, &bSupportsPresetQueue);
			if(bSupportsPresetQueue == VK_TRUE)
			{
				presentationFamilyIndex = queueFamilyIndex;
				presentationQueueCount = queueFamilyProperties[queueFamilyIndex].queueCount;
			}
		}

		// Graphics queue
		if(!graphicsFamilyIndex.has_value() && (requestedQueueTypes & queueFamilyPropertiesFlags) & VK_QUEUE_GRAPHICS_BIT)
		{
			graphicsFamilyIndex = queueFamilyIndex;
			graphicsQueueCount = queueFamilyProperties[queueFamilyIndex].queueCount;
			requestedQueueTypes &= ~VK_QUEUE_GRAPHICS_BIT;
			continue;
		}

		// Compute queue
		if(!computeFamilyIndex.has_value() && (requestedQueueTypes & queueFamilyPropertiesFlags) & VK_QUEUE_COMPUTE_BIT)
		{
			computeFamilyIndex = queueFamilyIndex;
			computeQueueCount = queueFamilyProperties[queueFamilyIndex].queueCount;
			requestedQueueTypes &= ~VK_QUEUE_COMPUTE_BIT;
			continue;
		}

		// Transfer queue
		if(!transferFamilyIndex.has_value() && (requestedQueueTypes & queueFamilyPropertiesFlags) & VK_QUEUE_TRANSFER_BIT)
		{
			transferFamilyIndex = queueFamilyIndex;
			transferQueueCount = queueFamilyProperties[queueFamilyIndex].queueCount;
			requestedQueueTypes &= ~VK_QUEUE_TRANSFER_BIT;
			continue;
		}
	}

	ASSERT(graphicsFamilyIndex.has_value() || computeFamilyIndex.has_value() || transferFamilyIndex.has_value(), "No suitable queue(s) found!");
	ASSERT(surface == VK_NULL_HANDLE || presentationFamilyIndex.has_value(), "No queues with presentation capabilities found!");
}

bool PhysicalDevice::isRayTracingSupported() const
{
	return (accelStructFeature.accelerationStructure && rayTracingFeature.rayTracingPipeline && rayQueryFeature.rayQuery);
}

std::vector<QueueFamilyPair> PhysicalDevice::findQueueFamilies() const
{
	std::set<QueueFamilyPair> familyIndices; // using a set here to keep queues in order

	if(graphicsFamilyIndex.has_value())
	{
		familyIndices.insert({graphicsFamilyIndex.value(), graphicsQueueCount});
	}
	if(computeFamilyIndex.has_value())
	{
		familyIndices.insert({computeFamilyIndex.value(), computeQueueCount});
	}
	if(transferFamilyIndex.has_value())
	{
		familyIndices.insert({transferFamilyIndex.value(), transferQueueCount });
	}
	if(presentationFamilyIndex.has_value())
	{
		familyIndices.insert({presentationFamilyIndex.value(), presentationQueueCount});
	}

	std::vector<QueueFamilyPair> queueFamilies(familyIndices.begin(), familyIndices.end());
	return queueFamilies;
}

void PhysicalDevice::initFeatures()
{
	features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	features.pNext = &features12;

	features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	features12.pNext = (void*)&bufferDeviceAddressFeatures;

	bufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
	bufferDeviceAddressFeatures.pNext = (void*)&descIndexFeature;

	descIndexFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
	descIndexFeature.pNext = (void*)&accelStructFeature;

	accelStructFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
	accelStructFeature.pNext = (void*)&rayTracingFeature;

	rayTracingFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
	rayTracingFeature.pNext = (void*)&rayQueryFeature;

	rayQueryFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
	rayQueryFeature.pNext = (void*)&meshShaderFeature;

	meshShaderFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV;
	meshShaderFeature.pNext = (void*)&timelineSemaphoreFeature;

	timelineSemaphoreFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
	timelineSemaphoreFeature.pNext = &multiviewFeature;

	multiviewFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES;
	multiviewFeature.pNext = &fragmentDensityMapFeature;

	fragmentDensityMapFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_FEATURES_EXT;
	fragmentDensityMapFeature.pNext = nullptr;

	vkGetPhysicalDeviceFeatures2(physicalDevice, &features);
}

void PhysicalDevice::initProperties()
{
	properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	properties.pNext = &rayTracingPipelineProperties;

	rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
	rayTracingPipelineProperties.pNext = &fragmentDensityMapProperties;

	fragmentDensityMapProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_PROPERTIES_EXT;
	fragmentDensityMapProperties.pNext = nullptr;

	vkGetPhysicalDeviceProperties2(physicalDevice, &properties);
}

void PhysicalDevice::enumerateSurfaceFormats(VkSurfaceKHR surface)
{
	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
	surfaceFormats.resize(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, surfaceFormats.data());
}

void PhysicalDevice::enumerateSurfaceCapabilities(VkSurfaceKHR surface)
{
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);
}

void PhysicalDevice::enumeratePresentationModes(VkSurfaceKHR surface)
{
	uint32_t presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

	presentModes.resize(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());
}
