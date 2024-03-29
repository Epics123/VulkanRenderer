#pragma once

#include <iostream>
#include "Renderer.h"
#include "Window.h"
#include "RenderSystems/RenderSystem.h"

class Application
{
public:
	static Application* applicationInstance;

	Application();
	Application(const char* appName, uint32_t appWidth, uint32_t appHeight);

	static Application* getInstance() { return applicationInstance; }
	static Application* initInstance(const char* appName, uint32_t appWidth, uint32_t appHeight);
	static void cleanupInstance();

	Window* getApplicationWindow() { return window; }

	void run();

private:
	// Update application
	void update();

	// Clean up application
	void cleanup();

	void processInput(GLFWwindow* window);

	void calculateFramerate();

	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
	static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

private:
	Renderer*vulkanRenderer;
	Window* window;

	const char* name;
	uint32_t width;
	uint32_t height;

	float dt = 0.0f;
	float lastFrame = 0.0f;

	float lastX = width / 2.0f;
	float lastY = height / 2.0f;

	bool framebufferResized = false;

	float lastMouseX, lastMouseY;
	double mouseX, mouseY;
	float mouseOffsetX, mouseOffsetY;
	bool firstMouse;
	bool firstKeyPress;

	double prevTime = 0.0;
	double curTime = 0.0;
	double timeDif;
	uint32_t frameCounter = 0;
	uint32_t framerateIndex = 0;
	
	static inline bool moveCamera = false;
};