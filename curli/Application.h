#pragma once
//#include <Scene.h>
#include "Renderer.h"
#include <ImguiHelpers.h>


template <class T>
class Application
{
public:
	Application(int argc, char const* argv[]) 
	{
		Scene scene;
		renderer = new T(scene);
		renderer->ParseArguments(argc, argv);
	}
	~Application() {}

	void Run() 
	{
		//Create a window with glfw
		auto& windowManager = GLFWHandler::GetInstance();
		windowManager.InitAndCreateWindow(1280, 720, "CuRLI");

		renderer->Initialize();

		//Init imgui
		gui::InitImgui(windowManager.GetWindowPointer());

		//Create a rendering loop with glfw
		while (windowManager.IsRunning())
		{
			//Render the scene
			renderer->Render();

			//Draw the GUI
			renderer->DrawGUI();

			//Event handling
			windowManager.DispatchEvents(*renderer);
		}

		gui::TerminateImgui();
		renderer->Terminate();
	}
private:
	T* renderer;
};