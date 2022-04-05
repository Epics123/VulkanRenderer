#include "Application.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#define GLFW_INCLUDE_VULKAN
#include <glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


Application::Application()
{
	name = "Vulkan App";
	width = 800;
	height = 600;

	dt = 0.0f;
	lastFrame = 0.0f;

	lastMouseX = 0.0f;
	lastMouseY = 0.0f;
	mouseX = 0.0;
	mouseY = 0.0;
	mouseOffsetX = 0.0f;
	mouseOffsetY = 0.0f;

	firstMouse = true;

	window = new Window(name, width, height);
}

Application::Application(const char* appName, uint32_t appWidth, uint32_t appHeight)
{
	name = appName;
	width = appWidth;
	height = appHeight;

	dt = 0.0f;
	lastFrame = 0.0f;

	lastMouseX = 0.0f;
	lastMouseY = 0.0f;
	mouseX = 0.0;
	mouseY = 0.0;
	mouseOffsetX = 0.0f;
	mouseOffsetY = 0.0f;

	firstMouse = true;

	window = new Window(name, width, height);
}

void Application::run()
{
	window->initWindow(keyCallback, cursorPosCallback, mouseButtonCallback, scrollCallback,framebufferResizeCallback, this);
	vulkanRenderer = Renderer::initInstance(window);
	update();
	cleanup();
}

void Application::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
	app->framebufferResized = true;
}

void Application::update()
{
	while (!glfwWindowShouldClose(window->getWindow()))
	{
		float currentFrame = (float)glfwGetTime();
		dt = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();
		processInput(window->getWindow());

		vulkanRenderer->drawFrame(dt);
	}

	vulkanRenderer->deviceWaitIdle();
}

void Application::cleanup()
{
	Renderer::cleanupInstance();

	window->cleanupWindow();

	//ImGui_ImplVulkan_Shutdown();
	//ImGui::DestroyContext();
	glfwTerminate();

	// Call renderer cleanup
	delete window;
}

void Application::processInput(GLFWwindow* window)
{
	// Keyboard Camera controls
	float cameraSpeed = 2.5f * dt;
	if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		vulkanRenderer->getActiveCamera().updatePositon(GLFW_KEY_W, cameraSpeed);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		vulkanRenderer->getActiveCamera().updatePositon(GLFW_KEY_S, cameraSpeed);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		vulkanRenderer->getActiveCamera().updatePositon(GLFW_KEY_A, cameraSpeed);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		vulkanRenderer->getActiveCamera().updatePositon(GLFW_KEY_D, cameraSpeed);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		vulkanRenderer->getActiveCamera().updatePositon(GLFW_KEY_E, cameraSpeed);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		vulkanRenderer->getActiveCamera().updatePositon(GLFW_KEY_Q, cameraSpeed);
	
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
	{
		vulkanRenderer->setRenderMode(DEFAULT_LIT);
		//vulkanRenderer->recreateSwapChain();
	}
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
	{
		vulkanRenderer->setRenderMode(WIREFRAME);
		//vulkanRenderer->recreateSwapChain();
	}

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)
		firstMouse = true;

	glfwGetCursorPos(window, &mouseX, &mouseY);

	if (moveCamera)
	{
		if (firstMouse)
		{
			lastMouseX = (float)mouseX;
			lastMouseY = (float)mouseY;
			firstMouse = false;
		}

		mouseOffsetX = (float)mouseX - lastMouseX;
		mouseOffsetY = lastMouseY - (float)mouseY;
		lastMouseX = (float)mouseX;
		lastMouseY = (float)mouseY;

		vulkanRenderer->getActiveCamera().updateCameraRotation(mouseOffsetX, -mouseOffsetY, 0.0f);
	}
}

void Application::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (key == GLFW_KEY_T && action == GLFW_PRESS)
		Renderer::compileShaders();
}

void Application::cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
	
}

void Application::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		moveCamera = true;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
	{
		moveCamera = false;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

void Application::scrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
	Renderer::rendererInstance->getActiveCamera().zoomCamera(yOffset);
}
