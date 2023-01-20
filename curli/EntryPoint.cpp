#include <glm/glm.hpp>
#include <ImguiHelpers.h>
#include <Renderer.h>
#include <stdio.h>

int main(int argc, char const *argv[])
{
	//Create a glfw window
	auto& windowManager = GLFWHandler::GetInstance();
	windowManager.InitAndCreateWindow(1280, 720, "CuRLI");

	AnimatedBGRenderer renderer;
	
	windowManager.SetKeyboardCallback([](GLFWwindow* window, int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	});
	
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

    return 0;
}
