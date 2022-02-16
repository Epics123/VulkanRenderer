#pragma once

#define GLFW_INCLUDE_VULKAN

#include <iostream>

#include "Renderer.h"
#include "Window.h"

//class Renderer;

class Application
{
public:
	Application();
	Application(const char* appName, uint32_t appWidth, uint32_t appHeight);

	void run();

private:
	// Update application
	void update();

	// Clean up application
	void cleanup();

	void processInput(GLFWwindow* window);

	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

	static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);

	static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

private:
	// REFACTOR
	Renderer*vulkanRenderer;
	Window* window;

	// PRE_REFACTOR
	const char* name;
	uint32_t width;
	uint32_t height;

	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	bool framebufferResized = false;
};