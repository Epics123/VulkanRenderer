#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <string>
#include <unordered_set>

class PhysicalDevice
{
public:
	PhysicalDevice(){};
	PhysicalDevice(VkPhysicalDevice device, VkSurfaceKHR surface, bool enableRayTracing = false);

	VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }

private:
	void initFeatures();
	void initProperties();

	void enumerateSurfaceFormats(VkSurfaceKHR surface);
	void enumerateSurfaceCapabilities(VkSurfaceKHR surface);
	void enumeratePresentationModes(VkSurfaceKHR surface);

private:
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	std::vector<std::string> extensions;

	// Properties
	VkPhysicalDeviceFragmentDensityMapPropertiesEXT fragmentDensityMapProperties;
	VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties;
	VkPhysicalDeviceProperties2 properties;

	// Features
	VkPhysicalDeviceFragmentDensityMapFeaturesEXT fragmentDensityMapFeature;
	VkPhysicalDeviceMultiviewFeatures multiviewFeature;
	VkPhysicalDeviceTimelineSemaphoreFeatures timelineSemaphoreFeature;
	VkPhysicalDeviceMeshShaderFeaturesNV meshShaderFeature;
	VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeature;
	VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingFeature;
	VkPhysicalDeviceAccelerationStructureFeaturesKHR accelStructFeature;
	VkPhysicalDeviceDescriptorIndexingFeatures descIndexFeature;
	VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures;
	VkPhysicalDeviceVulkan12Features features12;
	VkPhysicalDeviceFeatures2 features;

	// Memory properties
	VkPhysicalDeviceMemoryProperties2 memoryProperties;

	std::vector<VkSurfaceFormatKHR> surfaceFormats;
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	std::vector<VkPresentModeKHR> presentModes;
};

