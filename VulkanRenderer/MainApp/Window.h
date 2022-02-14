#ifndef WINDOW_H
#define WINDOW_H


#include <stdexcept>

//#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
//#include <glm/glm.hpp>

struct GLFWwindow;

class Window 
{
public:
	Window() = delete;
	Window(const char* name, uint32_t width, uint32_t height);

	void initWindow();

	GLFWwindow* getWindow() { return window; };

private:
	const char* name;
	uint32_t width;
	uint32_t height;

	GLFWwindow* window;
};

#endif // !WINDOW_H
