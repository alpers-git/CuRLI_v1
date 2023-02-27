#pragma once
//#include <Scene.h>
#include "Renderer.h"
#include <GUIManager.h>
#include <PhysicsIntegrator.h>


template <class R, class G, class P>
class Application
{
public:
	Application(int argc, char const* argv[]) 
	{
		scene = std::make_shared<Scene>();
		renderer = std::make_unique<R>(scene);
		guiManager = std::make_unique<G>(scene);
		physicsIntegrator = std::make_unique<P>(scene);
		ParseArguments(argc, argv);
		renderer->ParseArguments(argc, argv);
		
		scene->CreatePointLight(glm::vec3(0, 0, 20), 1, glm::vec3(0.85, 0.8, 0.95));//TODO
		auto* tr = scene->registry.try_get<CTransform>(scene->GetSceneObject("plane"));
		auto& tex = scene->registry.emplace<CImageMaps>(scene->GetSceneObject("plane"));
		//tex.AddImageMap(ImageMap::BindingSlot::T_DIFFUSE, Camera(), {500,500});
		if (tr)
		{
			tr->SetPosition(glm::vec3(0, -10, 0));
			tr->SetScale(glm::vec3(50, 50, 1));
		}

		std::string paths[] = {
			"../assets/images/cubemap/cubemap_posx.png",
			"../assets/images/cubemap/cubemap_negx.png",
			"../assets/images/cubemap/cubemap_posy.png",
			"../assets/images/cubemap/cubemap_negy.png",
			"../assets/images/cubemap/cubemap_posz.png",
			"../assets/images/cubemap/cubemap_negz.png"
		};
		scene->registry.emplace<CSkyBox>(scene->CreateSceneObject("skybox"), paths);

		auto& imaps = scene->registry.emplace<CImageMaps>(scene->GetSceneObject("sphere"));
		//imaps.AddImageMap(ImageMap::BindingSlot::ENV_MAP, paths);
	}
	~Application() {}

	void Run() 
	{
		//Create a window with glfw
		auto& windowManager = GLFWHandler::GetInstance();
		windowManager.InitAndCreateWindow(1280, 720, "CuRLI");

		renderer->Initialize();

		//Init imgui
		guiManager->Initialize(windowManager.GetWindowPointer());

		//Create a rendering loop with glfw
		while (windowManager.IsRunning())
		{
			//Update Physics
			physicsIntegrator->Update();

			//Update the scene
			scene->Update();
			
			//Render the scene
			renderer->Render();

			//Draw the GUI
			guiManager->DrawGUI();
			//renderer->DrawGUI();

			//Event handling
			windowManager.DispatchEvents(*renderer);
		}

		guiManager->Terminate();
		renderer->Terminate();
	}
private:
	std::unique_ptr<R> renderer;
	std::unique_ptr<G> guiManager;
	std::unique_ptr<P> physicsIntegrator;
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