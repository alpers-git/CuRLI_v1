#include "Application.h"
#include "Renderer.h"
#include <ImguiHelpers.h>

Application::Application(int argc, char const* argv[])
{
	renderer.ParseArguments(argc, argv);
}

Application::~Application()
{
}

void Application::Run()
{	
	auto& windowManager = GLFWHandler::GetInstance();
	windowManager.InitAndCreateWindow(1280, 720, "CuRLI");

	GLFWHandler::GetInstance().SetKeyboardCallback([](GLFWwindow* window, int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		});
	//TODO
	
	renderer.Initialize();
	
	//Init imgui
	gui::InitImgui(windowManager.GetWindowPointer());

	//Create a rendering loop with glfw
	while (windowManager.IsRunning())
	{
		//Render the scene
		renderer.Render();

		//Draw the GUI
		renderer.DrawGUI();

	}

	gui::TerminateImgui();
	renderer.Terminate();
}