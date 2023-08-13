#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include "Application.h"

int main()
{
	Application* app = Application::initInstance("Vulkan Renderer", 1280, 720);

	try
	{
		app->run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;

		app->cleanupInstance();
		app = nullptr;

		system("pause");

		return EXIT_FAILURE;
	}

	app->cleanupInstance();
	app = nullptr;

	system("pause");

	return EXIT_SUCCESS;
}