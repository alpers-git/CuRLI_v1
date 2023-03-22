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
			windowManager.DispatchEvents(*renderer, *physicsIntegrator);
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
				bool hasRB = false; //Rigidbody
				bool hasBC = false; //BoxCollider
				
				bool hasIM = false; //ImageMap
				ImageMap::BindingSlot bindingSlot = ImageMap::BindingSlot::T_DIFFUSE;
				std::string imPath;
				
				std::string path;
				for (; i < argc; i++)
				{
					if (std::string(argv[i]).compare("--rb") == 0)
					{
						//i++;
						hasRB = true;
					}
					else if (std::string(argv[i]).compare("--path") == 0)
					{
						i++;
						path = argv[i];
					}
					else if (std::string(argv[i]).compare("--bc") == 0)
					{
						//i++;
						hasBC = true;
					}
					else if (std::string(argv[i]).compare("--im") == 0)
					{
						i++;
						hasIM = true;
						if (std::string(argv[i]).compare("amb"))
							bindingSlot = ImageMap::BindingSlot::T_AMBIENT;
						else if (std::string(argv[i]).compare("diff"))
							bindingSlot = ImageMap::BindingSlot::T_DIFFUSE;
						else if (std::string(argv[i]).compare("spec"))
							bindingSlot = ImageMap::BindingSlot::T_SPECULAR;
						else if (std::string(argv[i]).compare("norm"))
							bindingSlot = ImageMap::BindingSlot::NORMAL;
						else if (std::string(argv[i]).compare("bump"))
							bindingSlot = ImageMap::BindingSlot::BUMP;
						else
							throw std::invalid_argument("image map requires a type");
						i++;
						//if the next argument does not start with - set it to imPath
						if(std::string(argv[i]).at(0) != '-' )
							imPath = std::string(argv[i]);
					}
					else
					{
						i--;
						break;
					}
				}
				entt::entity modelObj = scene->CreateModelObject(path);
				if (hasRB)
					scene->registry.emplace<CRigidBody>(modelObj, .5f);
				if (hasBC)
					scene->registry.emplace<CBoxCollider>( modelObj, 
						scene->registry.get<CTriMesh>(modelObj).GetBoundingBoxMin(), 
						scene->registry.get<CTriMesh>(modelObj).GetBoundingBoxMax() );
				if (hasIM)
				{
					auto imap = scene->registry.emplace<CImageMaps>(modelObj);
					if (imPath.empty())
						imap.AddImageMap(bindingSlot, glm::ivec2(800, 800), ImageMap::RenderImageMode::REFLECTION);
					else
						imap.AddImageMap(bindingSlot, imPath);
				}
					
			}
			else if (std::string(argv[i]).compare("-skybox") == 0)
			{
				i++;
				std::string paths[] = {
					argv[i],
					argv[i + 1],
					argv[i + 2],
					argv[i + 3],
					argv[i + 4],
					argv[i + 5]
				};
				scene->registry.emplace<CSkyBox>(scene->CreateSceneObject("skybox"), paths);
				i += 5;
			}
			else if (std::string(argv[i]).compare("-light") == 0)
			{
				i++;
				glm::vec3 pos(NAN);
				glm::vec3 dir(NAN);
				float cutoff = NAN;
				glm::vec3 color;
				float intensity;
				for (; i < argc; i++)
				{
					if (std::string(argv[i]).compare("--pos") == 0)
					{
						i++;
						pos = glm::vec3(std::stof(argv[i]), std::stof(argv[i + 1]), std::stof(argv[i + 2]));
						i += 2;
					}
					else if (std::string(argv[i]).compare("--color") == 0)
					{
						i++;
						color = glm::vec3(std::stof(argv[i]), std::stof(argv[i + 1]), std::stof(argv[i + 2]));
						i += 2;
					}
					else if (std::string(argv[i]).compare("--intensity") == 0)
					{
						i++;
						intensity = std::stof(argv[i]);
					}
					else if (std::string(argv[i]).compare("--dir") == 0)
					{
						i++;
						dir = glm::vec3(std::stof(argv[i]), std::stof(argv[i + 1]), std::stof(argv[i + 2]));
						i += 2;
					}
					else if (std::string(argv[i]).compare("--cutoff") == 0)
					{
						i++;
						cutoff = glm::radians(std::stof(argv[i]));
					}
					else
					{
						i--;
						break;
					}
				}
				if(glm::any(glm::isnan(pos)))
					scene->CreateDirectionalLight(dir, intensity, color);
				else if (glm::any(glm::isnan(dir)))
					scene->CreatePointLight(pos, intensity, color);
				else
					scene->CreateSpotLight(pos, dir, cutoff, intensity, color);
			}
		}
	}
};