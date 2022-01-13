# VulkanRenderer

This is a 3D Renderer using the Vulkan API, and written in C/C++. The goal of this project is to learn about and experiment with building a custom rendering framework from scratch. Future plans include adding support for loading 3D models, different lighting methods (including ray tracing), skeletal animation support, and much more.

***

## Project Setup
This project has been built using Visual Studio 2019, and is currently only built to run on Windows. Cross-platform capabilities may be added in the future.

<ins>**1. Downloading the repository:**</ins>

Clone the repository with `git clone https://github.com/Epics123/VulkanRenderer.git`.

<ins>**2. Dependencies:**</ins>

1. Run the [Setup.bat](https://github.com/Epics123/VulkanRenderer/blob/dev/scripts/Setup.bat) found in the `scripts` folder. This will download all the required prerequisites for the project if they are no present yet.
2. The main prerequisite is the Vulkan SDK. If it is not installed, the script will prompt the user to install the SDK.
3. After the SDK is installed, run the [Setup.bat](https://github.com/Epics123/VulkanRenderer/blob/dev/scripts/Setup.bat) file again.
4. After all dependencies are downloaded, the [Win-GenProject.bat](https://github.com/Epics123/VulkanRenderer/blob/dev/scripts/Win-GenProject.bat) will run automatically, which will generate a Visual Studio solution file that the user can use.

If any changes are made to the [premake5.lua](https://github.com/Epics123/VulkanRenderer/blob/dev/premake5.lua) file, rerun the [Win-GenProject.bat](https://github.com/Epics123/VulkanRenderer/blob/dev/scripts/Win-GenProject.bat) script found in the `scripts` folder. You will not need to touch any of the python files.
