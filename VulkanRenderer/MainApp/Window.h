#ifndef WINDOW_H
#define WINDOW_H


#include <stdexcept>

//#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
//#include <glm/glm.hpp>

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

private:
	const char* name;
	uint32_t width;
	uint32_t height;

	bool resizeable;

	GLFWwindow* window;
};

#endif // !WINDOW_H
