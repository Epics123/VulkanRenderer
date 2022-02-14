#include "Window.h"

#include <glfw3.h>
#include <glfw3native.h>

Window::Window(const char* name, uint32_t width, uint32_t height) :
	name(name),
	width(width),
	height(height)
{
	window = nullptr;
}

void Window::initWindow()
{

}