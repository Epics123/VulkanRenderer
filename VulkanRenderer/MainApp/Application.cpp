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
	//InitImGui();
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

		//UpdateImGUI();
		vulkanRenderer->drawFrame(dt);
	}

	vulkanRenderer->deviceWaitIdle();
}

void Application::cleanup()
{
	//vulkanRenderer->cleanup();
	Renderer::cleanupInstance();

	//glfwDestroyWindow(window);
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

void Application::InitImGui()
{
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForVulkan(window->getWindow(), true);

	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = vulkanRenderer->getVulkanInstance();
	init_info.PhysicalDevice = vulkanRenderer->getPhysicalDevice();
	init_info.Device = vulkanRenderer->getDevice();
	init_info.Queue = vulkanRenderer->getGraphicsQueue();
	init_info.DescriptorPool = *vulkanRenderer->getDescriptorPool();
	init_info.MinImageCount = 3;
	init_info.ImageCount = 3;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	ImGui_ImplVulkan_Init(&init_info, vulkanRenderer->getRenderPass());


	ImGuiIO& io = ImGui::GetIO();
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

	// Upload Fonts
	{
		// Use any command queue
		VkCommandPool command_pool = vulkanRenderer->getCommandPool();
		VkCommandBuffer command_buffer = vulkanRenderer->getCommandBuffers()[0];

		VkResult err = vkResetCommandPool(vulkanRenderer->getDevice(), command_pool, 0);
		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		err = vkBeginCommandBuffer(command_buffer, &begin_info);

		ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

		VkSubmitInfo end_info = {};
		end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		end_info.commandBufferCount = 1;
		end_info.pCommandBuffers = &command_buffer;
		err = vkEndCommandBuffer(command_buffer);
		err = vkQueueSubmit(vulkanRenderer->getGraphicsQueue(), 1, &end_info, VK_NULL_HANDLE);

		err = vkDeviceWaitIdle(vulkanRenderer->getDevice());
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

	//TEMP: Should make own key codes
	//io.KeyMap[ImGuiKey_]
}

void Application::UpdateImGUI()
{
	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &show_another_window);

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), vulkanRenderer->getCommandBuffers()[0], *vulkanRenderer->getActivePipeline().getPipeline());
}

void Application::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	/*if(key == GLFW_KEY_E && action == GLFW_PRESS)
		printf("E Key Pressed! \n");*/
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
