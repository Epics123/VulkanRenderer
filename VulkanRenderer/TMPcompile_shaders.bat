@echo off
cd MainApp/resources/vulkan/shaders/
for /r %%i in (*.frag, *.vert) do C:/VulkanSDK/1.2.189.2/Bin/glslangValidator.exe -V %%i
pause