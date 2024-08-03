#pragma once

#include <functional>
#include <unordered_set>

#include <vulkan/vk_enum_string_helper.h>

#include "Context.h"
#include "Texture.h"
#include "Log.h"

namespace Utils
{
	// from: https://stackoverflow.com/a/57595105
	template <typename T, typename... Rest>
	void hashCombine(std::size_t& seed, const T& v, const Rest&... rest)
	{
		seed ^= std::hash<T>{}(v)+0x9e3779b9 + (seed << 6) + (seed >> 2);
		(hashCombine(seed, rest), ...);
	};

	bool loadImageFromFile(Context& device, const char* filepath, Texture_& outTexture, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB);

	std::string getCPUName();

	class FileDialogs
	{
	public:
		static std::string openFile(const char* filter);
		static std::string saveFile(const char* filter);
	};

	std::unordered_set<std::string> filterExtensions(
		std::vector<std::string> availableExtensions,
		std::vector<std::string> requestedExtensions);

	std::vector<char> readFile(const std::string& filepath, bool isBinary);

	bool fileEndsWith(const char* filepath, const char* extension);

#define VK_CHECK(func, errorMsg) Utils::CheckVkResult(func, errorMsg)

	static void CheckVkResult(VkResult result, const char* errorMessage)
	{
		if (result != VK_SUCCESS)
		{
			CORE_CRITICAL("{0}. Error Code: {1}", errorMessage, string_VkResult(result));
			assert(false);
		}
	}
}

