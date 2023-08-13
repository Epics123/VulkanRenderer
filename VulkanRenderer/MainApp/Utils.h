#pragma once

#include <functional>

#include "Device.h"
#include "Texture.h"

namespace Utils
{
	// from: https://stackoverflow.com/a/57595105
	template <typename T, typename... Rest>
	void hashCombine(std::size_t& seed, const T& v, const Rest&... rest)
	{
		seed ^= std::hash<T>{}(v)+0x9e3779b9 + (seed << 6) + (seed >> 2);
		(hashCombine(seed, rest), ...);
	};

	bool loadImageFromFile(Device& device, const char* filepath, Texture& outTexture, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB);

	std::string getCPUName();

	class FileDialogs
	{
	public:
		static std::string openFile(const char* filter);
		static std::string saveFile(const char* filter);
	};
}

