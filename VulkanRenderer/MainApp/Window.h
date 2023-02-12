#ifndef WINDOW_H
#define WINDOW_H


#include <stdexcept>

//#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
//#include <glm/glm.hpp>

#define GLFW_INCLUDE_VULKAN

#include <glfw3.h>
#include <glfw3native.h>

class Window 
{
public:
	Window() = delete;
	Window(const char* name, uint32_t width, uint32_t height, bool canResize = false);

	void initWindow(GLFWkeyfun keyCallback, GLFWcursorposfun cursorPosCallback, GLFWmousebuttonfun mouseButtonCallback, GLFWscrollfun mouseScrollCallback, GLFWframebuffersizefun framebufferResizeCallback, void*user);
	void cleanupWindow();

	void getFrameBufferSize(int* width, int* height);

	GLFWwindow* getWindow() { return window; };

	void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

	VkExtent2D getExtent() { return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; }

	bool wasWindowResized() { return framebufferResized; }
	void resetWindowResizedFlag() { framebufferResized = false; }
	void setFrameBufferResized(bool resized) { framebufferResized = resized; }

	void setWidth(uint32_t newWidth) { width = newWidth; }
	void setHeight(uint32_t newHeight) { width = newHeight; }

	const uint32_t getWidth() { return width; }
	const uint32_t getHeight() { return height; }

private:
	const char* name;
	uint32_t width;
	uint32_t height;

	bool resizeable;
	bool framebufferResized = false;

	GLFWwindow* window;
};

#endif // !WINDOW_H
