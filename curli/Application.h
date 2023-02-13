#pragma once
//#include <Scene.h>
#include "Renderer.h"
#include <ImguiHelpers.h>


template <class R>
class Application
{
public:
	Application(int argc, char const* argv[]) 
	{
		scene = std::make_shared<Scene>();
		ParseArguments(argc, argv);
		
		scene->CreatePointLight(glm::vec3(0, 0, 20), 1, glm::vec3(0.7, 0.8, 0.05));//TODO
		scene->CreatePointLight(glm::vec3(10, 10, 0), .1, glm::vec3(0.8, 0.1, 0.0));//TODO
		scene->CreatePointLight(glm::vec3(-20, 0, 0), .5, glm::vec3(0.01, 0.1, 0.9));

		scene->GetComponent<CRigidBody>(scene->GetSceneObject("sphere")).drag = 0.005f;
		
		renderer = std::make_unique<R>(scene);
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
	std::unique_ptr<R> renderer;
	std::shared_ptr<Scene> scene;
	
	
	inline void ParseArguments(int argc, char const* argv[])
	{
		//parse arguments
		for (int i = 1; i < argc; i++)
		{
			if (std::string(argv[i]).compare("-model")==0)
			{
				i++;
				bool hasRB = false;
				std::string path;
				for (; i < argc; i++)
				{
					if (std::string(argv[i]).compare("--rb") == 0)
					{
						hasRB = true;
					}
					else if (std::string(argv[i]).compare("--path") == 0)
					{
						i++;
						path = argv[i];
					}
					else
					{
						i--;
						break;
					}
				}
				if (hasRB)
					scene->registry.emplace<CRigidBody>(scene->CreateModelObject(path), 1.0f, glm::vec3(0), glm::vec3(0));
				else
					scene->CreateModelObject(path), 1.0f, glm::vec3(0), glm::vec3(0);
			}
		}
	}
};