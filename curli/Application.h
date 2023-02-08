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
		scene = std::make_shared<Scene>();
		
		scene->registry.emplace<CRigidBody>(scene->CreateModelObject(argv[1]), 10.0f, glm::vec3(0), glm::vec3(0));
		
		scene->AddPointLight(glm::vec3(0, 0, 20), 1, glm::vec3(0.7, 0.8, 0.05));
		scene->AddPointLight(glm::vec3(10, 10, 0), .1, glm::vec3(0.8, 0.1, 0.0));
		scene->AddPointLight(glm::vec3(-20, 0, 0), .5, glm::vec3(0.01, 0.1, 0.9));
		
		renderer = std::make_unique<T>(scene);
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
			//Update the scene
			scene->Update();
			
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
	std::unique_ptr<T> renderer;
	std::shared_ptr<Scene> scene;
};