$vulkanPath
@echo off
@echo ********************IMPORTANT*****************************
@echo MAKE SURE THAT THE VULKAN SDK IS INSTALLED BEFORE RUNNING!
cd \
for /d %%a in ("%cd%\*") do (
if %%~fa == %cd%VulkanSDK (set "vulkanPath=%%~fa" echo Vulkan sdk found!)
)
@echo Vulkan SDK Path: %vulkanPath%
if exist VulkanSDK\ (echo Vulkan sdk found!) else (echo Need to install vulkan sdk!)
cd %~dp0
pause