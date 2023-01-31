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

		auto entity = scene->CreateEntity();
		const auto view = scene->registry.view<CTriMesh>();
		scene->AddComponent<CTriMesh>(entity);
		scene->GetComponent<CTriMesh>(entity).LoadObj(argv[1]);
		
		printf("Loaded %d vertices and %d faces\n", 
			scene->GetComponent<CTriMesh>(entity).GetNumVertices(),
			scene->GetComponent<CTriMesh>(entity).GetNumFaces());
		
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