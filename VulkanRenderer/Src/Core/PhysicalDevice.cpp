#include "PhysicalDevice.h"
#include "Utils/Utils.h"

#include <algorithm>
#include <iterator>

PhysicalDevice::PhysicalDevice(VkPhysicalDevice device, VkSurfaceKHR surface, bool enableRayTracing)
	:physicalDevice{device}
{
	initFeatures();
	initProperties();

	memoryProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
	vkGetPhysicalDeviceMemoryProperties2(physicalDevice, &memoryProperties);

	{
		uint32_t propertyCount = 0;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &propertyCount, nullptr);
		
		std::vector<VkExtensionProperties> properties(propertyCount);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &propertyCount, properties.data());
	}

	if(surface != VK_NULL_HANDLE)
	{
		enumerateSurfaceFormats(surface);
		enumerateSurfaceCapabilities(surface);
		enumeratePresentationModes(surface);
	}
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
