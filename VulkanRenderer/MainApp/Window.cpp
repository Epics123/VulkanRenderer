#include "Window.h"

Window::Window(const char* name, uint32_t width, uint32_t height) :
	name(name),
	width(width),
	height(height)
{
	window = nullptr;
}

void Window::initWindow(GLFWkeyfun keyCallback, GLFWcursorposfun cursorPosCallback, GLFWmousebuttonfun mouseButtonCallback, GLFWscrollfun mouseScrollCallback, GLFWframebuffersizefun framebufferResizeCallback, void* user)
{
	// Init GLFW
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	// Create Window
	window = glfwCreateWindow(width, height, name, nullptr, nullptr);

	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, cursorPosCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetScrollCallback(window, mouseScrollCallback);

	glfwSetWindowUserPointer(window, user);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void Window::cleanupWindow()
{
	glfwDestroyWindow(window);
}

void Window::getFrameBufferSize(int* width, int* height)
{
	glfwGetFramebufferSize(window, width, height);
}