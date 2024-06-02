--Dependencies

VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["GLFW"] = "%{wks.location}/%{prj.name}/Libraries/glfw/include"
IncludeDir["glm"] = "%{wks.location}/%{prj.name}/Libraries/glm"
IncludeDir["ImGui"] = "%{wks.location}/%{prj.name}/Libraries/ImGui"
IncludeDir["ObjLoader"] = "%{wks.location}/%{prj.name}/Libraries/ObjLoader"
IncludeDir["ImgLoader"] = "%{wks.location}/%{prj.name}/Libraries/ImgLoader"
IncludeDir["spdlog"] = "%{wks.location}/%{prj.name}/Libraries/spdlog/include"
IncludeDir["yaml"] = "%{wks.location}/%{prj.name}/Libraries/yaml/include"
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"
IncludeDir["Core"] = "%{wks.location}/%{prj.name}/Src/Core"
IncludeDir["Engine"] = "%{wks.location}/%{prj.name}/Src/Engine/"
IncludeDir["Runtime"] = "%{wks.location}/%{prj.name}/Src/Engine/Runtime"
IncludeDir["Renderer"] = "%{wks.location}/%{prj.name}/Src/Engine/Renderer"
IncludeDir["RendererPass"] = "%{wks.location}/%{prj.name}/Src/Engine/Renderer/RenderPasses"
IncludeDir["Logging"] = "%{wks.location}/%{prj.name}/Src/Engine/Logging"
IncludeDir["Serialization"] = "%{wks.location}/%{prj.name}/Src/Engine/Serialization"
IncludeDir["Utils"] = "%{wks.location}/%{prj.name}/Src/Engine/Utils"
IncludeDir["RenderSystems"] = "%{wks.location}/%{prj.name}/Src/RenderSystems"
IncludeDir["Scene"] = "%{wks.location}/%{prj.name}/Src/Engine/Scene"
IncludeDir["Common"] = "%{wks.location}/%{prj.name}/Src/Engine/Common"

LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"
LibraryDir["GLFW"] = "%{wks.location}/%{prj.name}/Libraries/glfw/lib-vc2019"

Library = {}
Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["VulkanUtils"] = "%{LibraryDir.VulkanSDK}/VKLayer_utils.lib"