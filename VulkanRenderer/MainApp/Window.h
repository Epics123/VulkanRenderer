#ifndef WINDOW_H
#define WINDOW_H

#include <glfw3.h>
#include <glfw3native.h>
#include <stdexcept>

//#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
//#include <glm/glm.hpp>

class Window 
{
public:
	Window() = delete;
	Window(const char* name, uint32_t width, uint32_t height);

	void initWindow();

private:
	const char* name;
	uint32_t width;
	uint32_t height;
};

#endif // !WINDOW_H
