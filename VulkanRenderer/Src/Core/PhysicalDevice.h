#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <string>
#include <unordered_set>
#include <optional>

typedef std::pair<uint32_t, uint32_t> QueueFamilyPair;

class PhysicalDevice
{
public:
	PhysicalDevice(){};
	PhysicalDevice(VkPhysicalDevice device, VkSurfaceKHR surface, bool enableRayTracing = false);

	void reserveQueues(VkQueueFlags requestedQueueTypes, VkSurfaceKHR surface); // surface may be VK_NULL_HANDLE, as we may be rendering off screen

	VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }
	const VkSurfaceCapabilitiesKHR getSurfaceCapabilities() const { return surfaceCapabilities; }

	bool isDeviceValid() const { return !(physicalDevice == VK_NULL_HANDLE); }

	bool isRayTracingSupported() const;
	bool isMultiviewSupported() const { return multiviewFeature.multiview; }
	bool isFragmentDensityMapSupported() const { return fragmentDensityMapFeature.fragmentDensityMap == VK_TRUE; }

	std::optional<uint32_t> getComputeFamilyIndex() const { return computeFamilyIndex; }
	std::optional<uint32_t> getGraphicsFamilyIndex() const { return graphicsFamilyIndex; }
	std::optional<uint32_t> getTransferFamilyIndex() const { return transferFamilyIndex; }
	std::optional<uint32_t> getPresentationFamilyIndex() const {return presentationFamilyIndex; }

	uint32_t graphicsFamilyCount() const { return graphicsQueueCount; }
	uint32_t computeFamilyCount() const { return computeQueueCount; }
	uint32_t transferFamilyCount() const { return transferQueueCount; }
	uint32_t presentationFamilyCount() const { return presentationQueueCount; }

	const VkPhysicalDeviceProperties getDeviceProperties() const { return properties.properties; }
	const VkPhysicalDeviceProperties2 getDeviceProperties2() const { return properties; }

	std::vector<QueueFamilyPair> findQueueFamilies() const;

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

	std::vector<VkQueueFamilyProperties> queueFamilyProperties;

	std::optional<uint32_t> graphicsFamilyIndex;
	uint32_t graphicsQueueCount = 0;
	std::optional<uint32_t> computeFamilyIndex;
	uint32_t computeQueueCount = 0;
	std::optional<uint32_t> transferFamilyIndex;
	uint32_t transferQueueCount = 0;
	std::optional<uint32_t> presentationFamilyIndex;
	uint32_t presentationQueueCount = 0;
};

