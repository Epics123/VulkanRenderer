@echo off
set vertName=Vert
set fragName=Frag
set spvExtension=.spv
cd MainApp/resources/vulkan/shaders/
SETLOCAL ENABLEDELAYEDEXPANSION
for %%i in (*.vert) do (
	set shaderName=%%~ni%vertName%%spvExtension%
	C:/VulkanSDK/1.2.189.2/Bin/glslangValidator.exe -V %%i -o !shaderName!
)
for %%j in (*.frag) do (
	set shaderName=%%~nj%fragName%%spvExtension%
	C:/VulkanSDK/1.2.189.2/Bin/glslangValidator.exe -V %%j -o !shaderName!
)