include "Dependencies.lua"

workspace "VulkanRenderer"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}={cfg.architecture}"

project "VulkanRenderer"
	location "VulkanRenderer"
	kind "ConsoleApp"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/MainApp/**.h",
		"%{prj.name}/MainApp/**.cpp",
		"%{prj.name}/MainApp/resources/**.vert",
		"%{prj.name}/MainApp/resources/**.frag",
	}

	includedirs
	{
		"%{IncludeDir.VulkanSDK}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ObjLoader}"
	}

	libdirs
	{
		"%{LibraryDir.VulkanSDK}",
		"%{LibraryDir.GLFW}"
		--"%{Library.VulkanUtils}"
	}

	links
	{
		"glfw3_mt.lib",
		"vulkan-1.lib",
		"kernel32.lib",
		"user32.lib",
		"gdi32.lib",
		"winspool.lib",
		"comdlg32.lib",
		"advapi32.lib",
		"shell32.lib",
		"ole32.lib",
		"oleaut32.lib",
		"uuid.lib",
		"odbc32.lib",
		"odbccp32.lib"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

	filter "configurations:Debug"
		symbols "On"

	filter "configurations:Release"
		optimize "On"