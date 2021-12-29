@echo off
cd ../VulkanRenderer/resources/vulkan/shaders/
for /r %%i in (*.frag, *.vert) do D:/VulkanSDK/Bin/glslangValidator.exe -V %%i
pause