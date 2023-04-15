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

		auto plane = scene->GetSceneObject("plane");
		if (plane != entt::tombstone)
		{
			scene->registry.get<CTransform>(plane).SetPosition(glm::vec3(0.0f, -10.f, 0.0f));
		}
		
		
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
		while (windowManager.IsRunning()) {
			// Update Physics
			std::future<void> physicsResult = std::async(std::launch::async, [this]() {
				physicsIntegrator->Update();
				});

			// Update the Scene
			std::future<void> sceneResult = std::async(std::launch::async, [this]() {
				scene->Update();
				});

			// Draw the GUI
			guiManager->DrawGUI();


			// Handle Events
			windowManager.DispatchEvents(*renderer, *physicsIntegrator);

			// Wait for the results of each function call to complete
			//physicsResult.get();
			//sceneResult.get();

			// Render the Scene
			renderer->Render();
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
				
				std::string path;//doubles as nodePath
				std::string elePath = "";
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
						//check if path ends with .ele or .node
						if (path.substr(path.find_last_of(".") + 1) == "ele" ||
							path.substr(path.find_last_of(".") + 1) == "node")
						{
							i++;
							elePath = argv[i];
							//if path ends ele swap the values with path2
							if (path.substr(path.find_last_of(".") + 1) == "ele")
							{
								std::string temp = path;
								path = elePath;
								elePath = temp;
							}
						}
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
						else if (std::string(argv[i]).compare("disp"))
							bindingSlot = ImageMap::BindingSlot::DISPLACEMENT;
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
				entt::entity modelObj;
				if (!elePath.empty())
					modelObj = scene->CreateModelObject(/*node*/path, elePath);
				else
					modelObj = scene->CreateModelObject(path);
				if (elePath.empty() && hasRB)
					scene->registry.emplace<CRigidBody>(modelObj, .5f);
				if (elePath.empty() && hasBC)
					scene->registry.emplace<CBoxCollider>( modelObj, 
						scene->registry.get<CTriMesh>(modelObj).GetBoundingBoxMin(), 
						scene->registry.get<CTriMesh>(modelObj).GetBoundingBoxMax() );
				if (hasIM)
				{
					auto imap = scene->registry.emplace<CImageMaps>(modelObj);
					if (imPath.empty())
						imap.AddImageMap(bindingSlot, glm::ivec2(800, 800), 
							ImageMap::RenderImageMode::REFLECTION);
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