#pragma once
#include <Scene.h>
#include <OpenGLProgram.h>
#include <glad/glad.h>
#include <GLFWHandler.h>
#include <iostream>
#include <cyTriMesh.h>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <imgui.h>
#include <ImguiHelpers.h>


template <class T>
class Renderer
{
	friend class GLFWHandler;
public:
	Renderer(std::shared_ptr<Scene> scene)
		:scene(scene)
	{}
	~Renderer() 
	{}
	//Called before render loop
	void Initialize()
	{
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			std::cout << "Failed to initialize OpenGL context" << std::endl;
			return;
		}
		//program = glCreateProgram();
		//glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
		program = std::make_unique<OpenGLProgram>();

		glEnable(GL_DEBUG_OUTPUT);
		static_cast<T*>(this)->Start();
	}
	
	//Renders the Scene and clears the Frame
	void Render()
	{
		GLFWHandler::GetInstance().SwapBuffers();

		//Scene changes
		static_cast<T*>(this)->PreUpdate();

		//glClear(clearFlags);
		program->Clear();
		//Rendering
		static_cast<T*>(this)->Update();
	}
	//Cleans up after render loop exits
	void Terminate()
	{
		static_cast<T*>(this)->End();
		GLFWHandler::GetInstance().Close();
	}
	
	void DrawGUI()
	{
		gui::GetNewImguiFrame();
		static_cast<T*>(this)->UpdateGUI();
		gui::RenderImgui();
	}
	
protected:
	//unsigned int VBO, VAO, EBO;
	std::unique_ptr<OpenGLProgram> program;
	std::shared_ptr<Scene> scene;

	struct vec5
	{
		int v[5] = { -1,-1,-1,-1,-1 };
	};
	std::unordered_map<entt::entity, unsigned int> entity2VAOIndex;
	std::unordered_map<entt::entity,vec5> entity2TextureIndices;

	/*
	* Parses arguments called when application starts
	*/
	void ParseArguments(int argc, char const* argv[]){}
//==============================================================
	/*
	* Called before render loop
	*/
	virtual void Start() = 0;
	/*
	* Called During Render after clear buffers calls
	*/
	virtual void PreUpdate() = 0;
	/*
	* Called During Render
	*/
	virtual void Update() = 0;
	/*
	* Called after render loop
	*/
	virtual void End() = 0;
//==============================================================
	/*
	* For Imgui components
	*/
	void UpdateGUI() {}
//==============================================================
	/*
	* Called by DispatchEvent when window is resized
	*/
	void OnWindowResize(int w, int h) {};
	/*
	* Called by DispatchEvent when window is moved
	*/
	void OnWindowMove(int x, int y) {};
	/*
	* Called by DispatchEvent when window is focused
	*/
	void OnWindowFocus(bool focused) {};
	/*
	* Called by DispatchEvent when window is iconified
	*/
	void OnWindowIconify(bool iconified) {};
	/*
	* Called by DispatchEvent when window is maximized
	*/
	void OnWindowMaximize(bool maximized) {};
	/*
	* Called by DispatchEvent when mouse buttons are used
	*/
	void OnMouseButton(int button, int action, int mods) {};
	/*
	* Called by DispatchEvent when mouse is scrolled
	*/
	void OnMouseScroll(double xoffset, double yoffset) {};
	/*
	* Called by DispatchEvent when mouse is moved
	*/
	void OnMouseMove(double xpos, double ypos) {};
	/*
	* Called by DispatchEvent when keyboard is used
	*/
	void OnKeyboard(int key, int scancode, int action, int mods) {};
	/*
	* Handles geometry updates
	*/
	void OnGeometryChange(entt::entity e)
	{
		if (entity2VAOIndex.find(e) != entity2VAOIndex.end())
		{
			program->vaos[entity2VAOIndex[e]].Delete();
			entity2VAOIndex.erase(e);
		}
		auto* mesh = scene->registry.try_get<CTriMesh>(e);
		program->vaos.push_back(VertexArrayObject());
		entity2VAOIndex[e] = program->vaos.size() - 1;
		VertexBufferObject vertexVBO(
			mesh->GetVertexDataPtr(),
			mesh->GetNumVertices(),
			GL_FLOAT,
			"pos",
			3,
			program->GetID());
		program->vaos.back().AddVBO(vertexVBO);

		if (mesh->GetNumNormals() > 0)
		{
			VertexBufferObject normalsVBO(
				mesh->GetNormalDataPtr(),
				mesh->GetNumNormals(),
				GL_FLOAT,
				"norm",
				3,
				program->GetID());
			program->vaos.back().AddVBO(normalsVBO);		
		}
		if (mesh->GetNumTextureVertices() > 0 && mesh->GetTextureDataPtr() != nullptr)
		{
			VertexBufferObject texVBO(
				mesh->GetTextureDataPtr(),
				mesh->GetNumTextureVertices(),
				GL_FLOAT,
				"texc",
				2,
				program->GetID());
			program->vaos.back().AddVBO(texVBO);

			//vao.SetRenderType(CVertexArrayObject::RenderType::PHONG_TEXTURE);
		}
		if(mesh->GetNumFaces() > 0)
		{
			program->vaos.back().CreateEBO(
				(unsigned int*)mesh->GetFaceDataPtr(),
				mesh->GetNumFaces()*3);
		}
		program->vaos.back().SetDrawMode(GL_TRIANGLES);
	};
	/*
	* Handles texture updates
	*/
	void OnTextureChange(entt::entity e, bool toBeRemoved)
	{
		if (entity2TextureIndices.find(e) != entity2TextureIndices.end())
		{
			for (int i = 0; i < 5; ++i)
				if (entity2TextureIndices[e].v[i] != -1)
					program->textures[entity2TextureIndices[e].v[i]].Delete();
			entity2TextureIndices.erase(e);
		}
		
		if (toBeRemoved)
			return;
		
		
		auto* imgMaps = scene->registry.try_get<CImageMaps>(e);
		
		//create an array of 5 ints to store the texture ids
		vec5 textureIDs;
		textureIDs.v[0] = -1;
		textureIDs.v[1] = -1;
		textureIDs.v[2] = -1;
		textureIDs.v[3] = -1;
		textureIDs.v[4] = -1;
		//iterate over all the image maps create texture objects for them
		for (auto it = imgMaps->mapsBegin(); it != imgMaps->mapsEnd(); ++it)
		{
			if (it->second.IsRenderedImage())
			{
				RenderedTexture2D renderedTexture(it->second.GetDims(), 
					GL_TEXTURE0 + (int)it->second.GetBindingSlot(), true, 
					GL_REPEAT, GL_REPEAT, GL_UNSIGNED_BYTE, GL_RGB, 0);
				renderedTexture.GetTexture().SetParami(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				renderedTexture.GetTexture().SetParami(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				renderedTexture.GetTexture().SetParamf(GL_TEXTURE_MAX_ANISOTROPY, 4.0f);
				//add the texture to the program
				program->textures.push_back(renderedTexture.GetTexture());
				program->renderedTextures.push_back(renderedTexture);
				it->second.glID = renderedTexture.GetTexture().GetGLID();
				//Needed for mapping between textures and CImageMaps
				it->second.SetProgramRenderedTexIndexIndex(program->renderedTextures.size() - 1);
			}
			else
			{
				Texture2D texture ((void*)&it->second.GetImage()[0], it->second.GetDims(),
					GL_TEXTURE0 + (int)it->second.GetBindingSlot());
				//add the texture to the program
				program->textures.push_back(texture);
				it->second.glID = texture.GetGLID();
			}
			textureIDs.v[(int)it->second.GetBindingSlot()] = program->textures.size() - 1;
		}
		//add the texture ids to the entity2TextureIndex map
		entity2TextureIndices[e] = textureIDs;
		imgMaps->dirty = false;
	}
};

class AnimatedBGRenderer : public Renderer<AnimatedBGRenderer>
{
public:
	AnimatedBGRenderer(std::shared_ptr<Scene> scene) :Renderer(scene) {}
	~AnimatedBGRenderer() {}

	void Start()
	{
		printf("Initializing AnimatedBGRenderer");
	}
	
	void PreUpdate()
	{
	}

	void Update()
	{
		const glm::vec3 clearColor1(0.09f, 0.30f, 0.55f);
		const glm::vec3 clearColor2(1.0f, 0.76f, 0.03f);
		float time = glfwGetTime() * 10.0f;
		//put time into a sin wave to get a value between 0 and 1
		float value = (sin(time) + 1.0f) / 2.0f;
		//lerp between the two colors
		program->SetClearColor(glm::vec4(glm::mix(
			clearColor1,
			clearColor2,
			value), 1.0f));
	}


	void End()
	{
		printf("Shutting down AnimatedBGRenderer");
	}
	
	void UpdateGUI()
	{
		ImGui::Begin("Test Window");
		ImGui::Text("Hello World");
		ImGui::End();
	}
	void OnKeyboard(int key, int scancode, int action, int mods)
	{
		// if GLFW_ESC is pressed exit the application
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(GLFWHandler::GetInstance().GetWindowPointer(), true);
	}
};

class PointRenderer : public Renderer<PointRenderer>
{
public:
	PointRenderer(std::shared_ptr<Scene> scene) :Renderer(scene) {}
	~PointRenderer() {}

	//override ParseArguments
	void ParseArguments(int argc, char const* argv[])
	{}

	void Start()
	{
		printf("Initializing Renderer\n");
		program->SetGLClearFlags(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		
		program->CreatePipelineFromFiles("../assets/shaders/simple/shader.vert",
			"../assets/shaders/simple/shader.frag");

		scene->registry.view<CTriMesh>()
			.each([&](const auto entity, auto& mesh)
				{
					OnGeometryChange(entity);
				});

		//Init camera
		int windowWidth, windowHeight;
		glfwGetWindowSize(GLFWHandler::GetInstance().GetWindowPointer(), &windowWidth, &windowHeight);
		scene->camera = Camera(glm::vec3(0.f, 0.f, 0.0f), glm::vec3(0.0f,0, 0), 1.f,
		45.f, 0.01f, 100000.f, (float)windowWidth / (float)windowHeight, true);

		LookAtMesh();
	}

	void PreUpdate()
	{
		//for each element with CTransform in the scene registry loop
		auto view = scene->registry.view<CTransform, CTriMesh>();

		view.each([&](auto& transform, auto& mesh)
			{
				transform.SetScale(glm::vec3(0.05f) *
				(scene->camera.IsPerspective() ? 1.f : 1.f / glm::length(scene->camera.GetLookAtEye())));
				transform.SetEulerRotation(glm::vec3(glm::radians(-90.f), 0, (float)glfwGetTime() * 0.5f));
				transform.SetPosition(-mesh.GetBoundingBoxCenter());
			});
	}

	void Update()
	{
		auto view = scene->registry.view<CTransform>();
		view.each([&](const auto& e, auto& transform)
			{

				const auto mvp = scene->camera.GetProjectionMatrix() * scene->camera.GetViewMatrix() * transform.GetModelMatrix();
				program->SetUniform("mvp", mvp);
				//bind GLSL program
				program->Use();
				program->vaos[entity2VAOIndex[e]].Draw(GL_POINTS);
			});
	}

	void End()
	{
		printf("Shutting down Renderer");
	}

	void UpdateGUI()
	{
		const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 10, main_viewport->WorkPos.y + 10));
		ImGui::Begin("Control panel", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
		if (ImGui::Button("Read Shader Files(F5)"))
		{
			program->SetVertexShaderSourceFromFile("../assets/shaders/phong/shader.vert");
			program->SetFragmentShaderSourceFromFile("../assets/shaders/phong/shader.frag");
		}
		if (ImGui::Button("Recompile Shaders(F6)"))
		{
			RecompileShaders();
		}
		if (ImGui::Button("Look at Teapot(F1)"))
		{
			LookAtMesh();
		}
		ImGui::End();
	}
	
	/*
	* Recompile the shaders
	*/
	inline void RecompileShaders()
	{
		if (program->CompileShaders())
		{
			printf("Shaders compiled successfully\n");
			program->AttachVertexShader();
			program->AttachFragmentShader();
		}
		else
			printf("Shaders compilation failed\n");
	}
	/*
	* Reloads and recompiles shaders
	*/
	inline void ReloadShaders()
	{
		program->SetVertexShaderSourceFromFile("../assets/shaders/simple/shader.vert");
		program->SetFragmentShaderSourceFromFile("../assets/shaders/simple/shader.frag");
		RecompileShaders();
	}
	
	/*
	* Points camera to mesh center
	*/
	inline void LookAtMesh()
	{
		scene->camera.SetOrbitDistance(3.f);
		scene->camera.SetCenter({0,0,0});
		scene->camera.SetOrbitAngles({ 0,0,0 });
	}

	void OnWindowResize(int w, int h)
	{
		scene->camera.SetAspectRatio((float)w / (float)h);
	}

	void OnKeyboard(int key, int scancode, int action, int mods)
	{
		// if GLFW_ESC is pressed exit the application
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(GLFWHandler::GetInstance().GetWindowPointer(), true);
		if (key == GLFW_KEY_F6 && action == GLFW_PRESS)
			RecompileShaders();
		if (key == GLFW_KEY_F5 && action == GLFW_PRESS)
			ReloadShaders();
		if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
			LookAtMesh();
		if (key == GLFW_KEY_P && action == GLFW_PRESS)
			scene->camera.SetPerspective(!scene->camera.IsPerspective());
	}
	
	//orbit camera
	void OnMouseButton(int button, int action, int mods)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT))
			m1Down = true;
		else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
			m1Down = false;

		if (button == GLFW_MOUSE_BUTTON_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT))
			m2Down = true;
		else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
			m2Down = false;
	}
	//orbit camera
	void OnMouseMove(double x, double y)
	{
		glm::vec2 deltaPos(prevMousePos.x - x, prevMousePos.y - y);
		if (m1Down)
			scene->camera.SetOrbitAngles(scene->camera.GetOrbitAngles() 
				- glm::vec3(deltaPos.y * 0.5f, -deltaPos.x * 0.4f, 0.f));
		if (m2Down)
			scene->camera.SetOrbitDistance(scene->camera.GetOrbitDistance() + deltaPos.y * 0.05f);
		prevMousePos = { x,y };
	}

private:
	//--orbit controls--//
	bool m1Down = false;
	bool m2Down = false;
	glm::vec2 prevMousePos;
	//--------------------//
};

class PhongRenderer : public Renderer<PhongRenderer>
{
public:
	PhongRenderer(std::shared_ptr<Scene> scene) :Renderer(scene) {}
	~PhongRenderer() {}

	//override ParseArguments
	void ParseArguments(int argc, char const* argv[])
	{}

	void Start()
	{
		printf("Initializing Renderer\n");
		program->SetGLClearFlags(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		program->CreatePipelineFromFiles("../assets/shaders/phong/shader.vert",
			"../assets/shaders/phong/shader.frag");
		program->SetClearColor({ 0.f,0.f,0.1f,1.f });

		scene->registry.view<CTriMesh>()
			.each([&](const auto entity, auto& mesh)
				{
					OnGeometryChange(entity);
				});

		//Init camera
		int windowWidth, windowHeight;
		glfwGetWindowSize(GLFWHandler::GetInstance().GetWindowPointer(), &windowWidth, &windowHeight);
		scene->camera = Camera(glm::vec3(0.f, 0.f, 0.0f), glm::vec3(0.0f, 0, 0), 1.f,
			45.f, 0.01f, 100000.f, (float)windowWidth / (float)windowHeight, true);

		ResetCamera();
		
		auto view3 = scene->registry.view<CTransform, CTriMesh>();
		view3.each([&](auto& transform, auto& mesh)
		{
			transform.SetScale(glm::vec3(0.05f) *
			(scene->camera.IsPerspective() ? 1.f : 1.f / glm::length(scene->camera.GetLookAtEye())));
			transform.SetEulerRotation(glm::vec3(glm::radians(-90.f), 0, 0));
			transform.SetPivot(mesh.GetBoundingBoxCenter());
		});

	}

	void PreUpdate()
	{
		int i = 0;
		auto view2 = scene->registry.view<CLight>();
		view2.each([&](auto& light)
			{
				std::string shaderName("light[" + std::to_string(i) + "].position");
				program->SetUniform(shaderName.c_str(), glm::vec3(scene->camera.GetViewMatrix() * glm::vec4(light.position, 1)));
				shaderName = std::string("light[" + std::to_string(i) + "].intensity");
				program->SetUniform(shaderName.c_str(), light.intensity);
				shaderName = std::string("light[" + std::to_string(i) + "].color");
				program->SetUniform(shaderName.c_str(), light.color);
				i++;
			});
		program->SetUniform("light_count", i);
	}

	void Update()
	{	
		scene->registry.view<CTriMesh>()
			.each([&](const auto& entity, auto& mesh)
			{
					program->vaos[entity2VAOIndex[entity]].visible = mesh.visible;
					CPhongMaterial* material = scene->registry.try_get<CPhongMaterial>(entity);
					CTransform* transform = scene->registry.try_get<CTransform>(entity);

					const glm::mat4 mv = scene->camera.GetViewMatrix() * (transform ?
						transform->GetModelMatrix() : glm::mat4(1.0f));
					const glm::mat4 mvp = scene->camera.GetProjectionMatrix() * mv;

					program->SetUniform("material.ka", material ? material->ambient : glm::vec3(0.0f));
					program->SetUniform("material.kd", material ? material->diffuse : glm::vec3(0.0f));
					program->SetUniform("material.ks", material ? material->specular : glm::vec3(0.0f));
					program->SetUniform("material.shininess", material ? material->shininess : 0.0f);

					program->SetUniform("to_screen_space", mvp);
					program->SetUniform("to_view_space", mv);
					program->SetUniform("normals_to_view_space",
						glm::transpose(glm::inverse(glm::mat3(mv))));

					//bind GLSL program
					program->Use();
					program->vaos[entity2VAOIndex[entity]].Draw();
			});
	}

	void End()
	{
		printf("Shutting down Renderer");
	}

	void UpdateGUI()
	{
		const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowSize(ImVec2(main_viewport->WorkSize.x/5, main_viewport->WorkSize.y/2));
		ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkSize.x - main_viewport->WorkSize.x/5 -5, main_viewport->WorkPos.y + 5));
		ImGui::Begin("Control panel", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
		if (ImGui::CollapsingHeader("Shaders", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::Button("Read Shaders(F5)"))
			{
				ReloadShaders();
			}
			ImGui::SameLine();
			if (ImGui::Button("Compile Shaders(F6)"))
			{
				RecompileShaders();
			}
		}
		if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::Button("Reset Camera(F1)"))
			{
				ResetCamera();
			}
			//ImGui::SameLine();
			glm::vec3 target = scene->camera.GetCenter();
			if (ImGui::DragFloat3("Target", &target[0], 0.01f))
			{
				scene->camera.SetCenter(target);
			}
		}
		if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen))
		{
			int i = 0;
			auto view = scene->registry.view<CPhongMaterial>();
			view.each([&](auto& material)
				{
					std::string field1(std::string("Specular##") + std::to_string(i));
					std::string field2(std::string("Diffuse##") + std::to_string(i));
					std::string field3(std::string("Ambient##") + std::to_string(i));
					std::string field4(std::string("Shininess##") + std::to_string(i));
					ImGui::ColorEdit3(field1.c_str(), &material.specular[0]);
					ImGui::ColorEdit3(field2.c_str(), &material.diffuse[0]);
					ImGui::ColorEdit3(field3.c_str(), &material.ambient[0]);
					ImGui::DragFloat(field4.c_str(), &material.shininess, 0.1f, 0.f, 500.f);
					ImGui::Separator();
					i++;
				});
		}
		if (ImGui::CollapsingHeader("Lights", ImGuiTreeNodeFlags_DefaultOpen))
		{
			int i  = 0;
			auto view = scene->registry.view<CLight>();
			view.each([&](auto& light)
				{
					std::string field1(std::string("Position##") + std::to_string(i));
					std::string field2(std::string("Intensity##") + std::to_string(i));
					std::string field3(std::string("Color##") + std::to_string(i));
					ImGui::DragFloat3( field1.c_str(), &light.position[0], 0.01f);
					ImGui::DragFloat(field2.c_str(), &light.intensity, 0.001f, 0.0f, 1.f);
					ImGui::ColorEdit3(field3.c_str(), &light.color[0], 0.01f);
					ImGui::Separator();
					i++;
				});
		}
		
		ImGui::End();
	}

	/*
	* Recompile the shaders
	*/
	inline void RecompileShaders()
	{
		if (program->CompileShaders())
		{
			printf("Shaders compiled successfully\n");
			program->AttachVertexShader();
			program->AttachFragmentShader();
		}
		else
			printf("Shaders compilation failed\n");
	}
	/*
	* Reloads and recompiles shaders
	*/
	inline void ReloadShaders()
	{
		program->SetVertexShaderSourceFromFile("../assets/shaders/phong/shader.vert");
		program->SetFragmentShaderSourceFromFile("../assets/shaders/phong/shader.frag");
		RecompileShaders();
	}

	/*
	* Points camera to mesh center
	*/
	inline void ResetCamera()
	{
		scene->camera.SetOrbitDistance(25.f);
		scene->camera.SetCenter({ 0,0,0 });
		scene->camera.SetOrbitAngles({ 90,0,0 });
	}

	void OnWindowResize(int w, int h)
	{
		scene->camera.SetAspectRatio((float)w / (float)h);
	}

	void OnKeyboard(int key, int scancode, int action, int mods)
	{
		// if GLFW_ESC is pressed exit the application
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(GLFWHandler::GetInstance().GetWindowPointer(), true);
		if (key == GLFW_KEY_F6 && action == GLFW_PRESS)
			RecompileShaders();
		if (key == GLFW_KEY_F5 && action == GLFW_PRESS)
			ReloadShaders();
		if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
			ResetCamera();
		if (key == GLFW_KEY_P && action == GLFW_PRESS)
			scene->camera.SetPerspective(!scene->camera.IsPerspective());
		if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS)
			ctrlDown = true;
		else if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_RELEASE)
			ctrlDown = false;
	}

	//orbit camera
	void OnMouseButton(int button, int action, int mods)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT))
			m1Down = true;
		else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
			m1Down = false;
		if (button == GLFW_MOUSE_BUTTON_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT))
			m2Down = true;
		else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
			m2Down = false;
	}
	//orbit camera
	void OnMouseMove(double x, double y)
	{
		bool updateMousePos = true;
		glm::vec2 deltaPos(prevMousePos.x - x, prevMousePos.y - y);
		if (m1Down && ctrlDown)
		{
			//UGLY CODE HERE
			//fetch the first light
			auto view2 = scene->registry.view<CLight>();
			int i = 0;
			view2.each([&](auto& light)
				{
					if (i == 0)
					{
						//get the angles around 0,0,0 based on position
						glm::vec3 angles = glm::vec3(
							asin((light.position.y) / glm::length(light.position)),
							atan2f((-light.position.z), (light.position.x)) + 1.57f,
							0.f);
						//calculate new spherical coordinates using angles - mouse delta 
						angles = angles + glm::vec3(deltaPos.y * .005f, -deltaPos.x * .004f, 0.f);
						if (angles.x > 89.5f)
							angles.x = 89.5f;
						if (angles.x < -89.5f)
							angles.x = -89.5f;
						const float theta = angles.x;
						const float phi = angles.y;
						glm::vec3 unitSpherePos = {
							cos(theta) * sin(phi),
							sin(theta),
							cos(theta) * cos(phi)
						};
						//Preserve the distance to center and set the new position
						light.position = unitSpherePos * glm::length(light.position);
					}
					i++;
				});

		}
		else if (m1Down)
			scene->camera.SetOrbitAngles(scene->camera.GetOrbitAngles()
				- glm::vec3(deltaPos.y * 0.5f, -deltaPos.x * 0.4f, 0.f));
		
		if (m2Down)
			scene->camera.SetOrbitDistance(scene->camera.GetOrbitDistance() + deltaPos.y * 0.05f);
		
		if(updateMousePos)
			prevMousePos = { x,y };


	}

private:
	//--orbit controls--//
	bool m1Down = false;
	bool m2Down = false;
	glm::vec2 prevMousePos;

	bool ctrlDown = false;
};

class TexturedPhongRenderer : public Renderer<TexturedPhongRenderer>
{
public:
	TexturedPhongRenderer(std::shared_ptr<Scene> scene) :Renderer(scene) {}
	~TexturedPhongRenderer() {}

	//override ParseArguments
	void ParseArguments(int argc, char const* argv[])
	{}

	void Start()
	{
		printf("Initializing Renderer\n");
		program->SetGLClearFlags(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		program->CreatePipelineFromFiles("../assets/shaders/phong_textured/shader.vert",
			"../assets/shaders/phong_textured/shader.frag");
		program->SetClearColor({ 0.01f,0.f,0.05f,1.f });
		
		scene->registry.view<CTriMesh>()
			.each([&](const auto entity, auto& mesh)
				{
					OnGeometryChange(entity);
				});
		
		scene->registry.view<CImageMaps>()
			.each([&](const auto entity, auto& maps)
				{
					OnTextureChange(entity, false);
				});

		//Init camera
		int windowWidth, windowHeight;
		glfwGetWindowSize(GLFWHandler::GetInstance().GetWindowPointer(), &windowWidth, &windowHeight);
		scene->camera = Camera(glm::vec3(0.f, 0.f, 0.0f), glm::vec3(0.0f, 0, 0), 1.f,
			45.f, 0.01f, 100000.f, (float)windowWidth / (float)windowHeight, true);

		ResetCamera();
		
		scene->registry.view<CTransform, CTriMesh>()
			.each([&](auto& transform, auto& mesh)
			{
				transform.SetScale(glm::vec3(0.05f) *
				(scene->camera.IsPerspective() ? 1.f : 1.f / glm::length(scene->camera.GetLookAtEye())));
				transform.SetEulerRotation(glm::vec3(glm::radians(-90.f), 0, 0));
			});
	}

	void PreUpdate()
	{
		int i = 0;
		scene->registry.view<CLight>()
			.each([&](auto& light)
			{
				std::string shaderName("light[" + std::to_string(i) + "].position");
				program->SetUniform(shaderName.c_str(), glm::vec3(scene->camera.GetViewMatrix() * glm::vec4(light.position, 1)));
				shaderName = std::string("light[" + std::to_string(i) + "].intensity");
				program->SetUniform(shaderName.c_str(), light.intensity);
				shaderName = std::string("light[" + std::to_string(i) + "].color");
				program->SetUniform(shaderName.c_str(), light.color);
				i++;
			});
		program->SetUniform("light_count", i);
	}

	void Update()
	{	
		//bind GLSL program
		program->Use();
		scene->registry.view<CTriMesh>()
			.each([&](const auto& entity, auto& mesh)
			{
				program->vaos[entity2VAOIndex[entity]].visible = mesh.visible;
				CPhongMaterial* material = scene->registry.try_get<CPhongMaterial>(entity);
				CTransform* transform = scene->registry.try_get<CTransform>(entity);
				
				const glm::mat4 mv =  scene->camera.GetViewMatrix() * (transform ? 
					transform->GetModelMatrix() : glm::mat4(1.0f));
				const glm::mat4 mvp = scene->camera.GetProjectionMatrix() * mv;
					
				program->SetUniform("material.ka", material ? material->ambient : glm::vec3(0.0f));
				program->SetUniform("material.kd", material ? material->diffuse : glm::vec3(0.0f));
				program->SetUniform("material.ks", material ? material->specular : glm::vec3(0.0f));
				program->SetUniform("material.shininess", material ? material->shininess : 0.0f);
				
				program->SetUniform("to_screen_space", mvp);
				program->SetUniform("to_view_space", mv);
				program->SetUniform("normals_to_view_space",
					glm::transpose(glm::inverse(glm::mat3(mv))));
				
				CImageMaps* imgMaps = scene->registry.try_get<CImageMaps>(entity);
				if (imgMaps)
				{
					//iterate over all imagemaps
					for (auto it = imgMaps->mapsBegin(); it != imgMaps->mapsEnd(); ++it)
					{
						int texIndex = entity2TextureIndices[entity].v[(int)it->first];
						//bind texture
						if (texIndex >= 0)
						{
							program->textures[texIndex].Bind();
							//set uniform
							const std::string uniformName = std::string("has_texture[") + std::to_string(((int)it->first)) + std::string("]");
							program->SetUniform(uniformName.c_str(), 1);
							const std::string uniformName2 = std::string("tex_list[") + std::to_string(((int)it->first)) + std::string("]");
							program->SetUniform(uniformName2.c_str(), ((int)it->first));
						}
						else
						{
							const std::string uniformName = std::string("has_texture[") + std::to_string(((int)it->first)) + std::string("]");
							program->SetUniform(uniformName.c_str(), 0);
						}
					}
				}
				program->SetUniform("shading_mode", ((int)mesh.GetShadingMode()));
				
				program->vaos[entity2VAOIndex[entity]].Draw();
			});
	}

	void End()
	{
		printf("Shutting down Renderer");
	}

	void UpdateGUI()
	{}
	
	/*
	* Recompile the shaders
	*/
	inline void RecompileShaders()
	{
		if (program->CompileShaders())
		{
			printf("Shaders compiled successfully\n");
			program->AttachVertexShader();
			program->AttachFragmentShader();
		}
		else
			printf("Shaders compilation failed\n");
	}
	/*
	* Reloads and recompiles shaders
	*/
	inline void ReloadShaders()
	{
		program->SetVertexShaderSourceFromFile("../assets/shaders/phong_textured/shader.vert");
		program->SetFragmentShaderSourceFromFile("../assets/shaders/phong_textured/shader.frag");
		RecompileShaders();
	}

	/*
	* Points camera to mesh center
	*/
	inline void ResetCamera()
	{
		scene->camera.SetOrbitDistance(25.f);
		scene->camera.SetCenter({ 0,0,0 });
		scene->camera.SetOrbitAngles({ 90,0,0 });
	}

	void OnWindowResize(int w, int h)
	{
		scene->camera.SetAspectRatio((float)w / (float)h);
	}

	void OnKeyboard(int key, int scancode, int action, int mods)
	{
		// if GLFW_ESC is pressed exit the application
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(GLFWHandler::GetInstance().GetWindowPointer(), true);
		if (key == GLFW_KEY_F6 && action == GLFW_PRESS)
			RecompileShaders();
		if (key == GLFW_KEY_F5 && action == GLFW_PRESS)
			ReloadShaders();
		if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
			ResetCamera();
		if (key == GLFW_KEY_P && action == GLFW_PRESS)
			scene->camera.SetPerspective(!scene->camera.IsPerspective());
		if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS)
			ctrlDown = true;
		else if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_RELEASE)
			ctrlDown = false;
	}

	//orbit camera
	void OnMouseButton(int button, int action, int mods)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT))
			m1Down = true;
		else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
			m1Down = false;
		if (button == GLFW_MOUSE_BUTTON_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT))
			m2Down = true;
		else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
			m2Down = false;
	}
	//orbit camera
	void OnMouseMove(double x, double y)
	{
		bool updateMousePos = true;
		glm::vec2 deltaPos(prevMousePos.x - x, prevMousePos.y - y);
		if (m1Down && ctrlDown)
		{
			//UGLY CODE HERE
			//fetch the first light
			auto view2 = scene->registry.view<CLight>();
			int i = 0;
			view2.each([&](auto& light)
				{
					if (i == 0)
					{
						//get the angles around 0,0,0 based on position
						glm::vec3 angles = glm::vec3(
							asin((light.position.y) / glm::length(light.position)),
							atan2f((-light.position.z), (light.position.x)) + 1.57f,
							0.f);
						//calculate new spherical coordinates using angles - mouse delta 
						angles = angles + glm::vec3(deltaPos.y * .005f, -deltaPos.x * .004f, 0.f);
						if (angles.x > 89.5f)
							angles.x = 89.5f;
						if (angles.x < -89.5f)
							angles.x = -89.5f;
						const float theta = angles.x;
						const float phi = angles.y;
						glm::vec3 unitSpherePos = {
							cos(theta) * sin(phi),
							sin(theta),
							cos(theta) * cos(phi)
						};
						//Preserve the distance to center and set the new position
						light.position = unitSpherePos * glm::length(light.position);
					}
					i++;
				});

		}
		else if (m1Down)
			scene->camera.SetOrbitAngles(scene->camera.GetOrbitAngles()
				- glm::vec3(deltaPos.y * 0.5f, -deltaPos.x * 0.4f, 0.f));
		
		if (m2Down)
			scene->camera.SetOrbitDistance(scene->camera.GetOrbitDistance() + deltaPos.y * 0.05f);
		
		if(updateMousePos)
			prevMousePos = { x,y };


	}

private:
	//--orbit controls--//
	bool m1Down = false;
	bool m2Down = false;
	glm::vec2 prevMousePos;

	bool ctrlDown = false;
};

class MultiTargetRenderer : public Renderer<MultiTargetRenderer>
{
public:
	MultiTargetRenderer(std::shared_ptr<Scene> scene) :Renderer(scene) {}
	~MultiTargetRenderer() {}

	//override ParseArguments
	void ParseArguments(int argc, char const* argv[])
	{}

	void Start()
	{
		printf("Initializing Renderer\n");
		program->SetGLClearFlags(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		program->CreatePipelineFromFiles("../assets/shaders/phong_textured/shader.vert",
			"../assets/shaders/phong_textured/shader.frag");
		program->SetClearColor({ 0.01f,0.f,0.05f,1.f });
		
		scene->registry.view<CTriMesh>()
			.each([&](const auto entity, auto& mesh)
				{
					OnGeometryChange(entity);
				});
		
		scene->registry.view<CImageMaps>()
			.each([&](const auto entity, auto& maps)
				{
					OnTextureChange(entity, false);
				});

		//Init camera
		int windowWidth, windowHeight;
		glfwGetWindowSize(GLFWHandler::GetInstance().GetWindowPointer(), &windowWidth, &windowHeight);
		scene->camera = Camera(glm::vec3(0.f, 0.f, 0.0f), glm::vec3(0.0f, 0, 0), 1.f,
			45.f, 0.01f, 100000.f, (float)windowWidth / (float)windowHeight, true);

		ResetCamera();
		
		scene->registry.view<CTransform, CTriMesh>()
			.each([&](auto& transform, auto& mesh)
			{
				/*transform.SetScale(glm::vec3(1.f) *
				(scene->camera.IsPerspective() ? 1.f : 1.f / glm::length(scene->camera.GetLookAtEye())));*/
				transform.SetEulerRotation(glm::vec3(glm::radians(-90.f), 0, 0));
			});
	}

	void PreUpdate()
	{	
		//render renderedTextures
		scene->registry.view<CImageMaps>()
			.each([&](const auto& entity, auto& maps)
			{
				//=======StackPush=======
				//Set the object's visibility to false
				auto* mesh = scene->registry.try_get<CTriMesh>(entity);
				Camera tmp = scene->camera;
				glm::vec4 clearColor = program->GetClearColor();
				if(mesh)
					mesh->visible = false;

				//iterate over each imagemap of maps
				if (!maps.dirty)
				{
					for (auto it = maps.mapsBegin(); it != maps.mapsEnd(); ++it)
					{
						if (it->second.IsRenderedImage())
						{
							auto* material = scene->registry.try_get<CPhongMaterial>(entity);
							if(material)
								program->SetClearColor(glm::vec4(material->diffuse,1));
							scene->camera = it->second.GetRenderedImageCamera();
							program->renderedTextures[it->second.GetProgramRenderedTexIndexIndex()].
								Render(std::bind(&MultiTargetRenderer::Update, this));
						}
					}
				}

				//=======StackPop=======
				program->SetClearColor(clearColor);
				scene->camera = tmp;
				if(mesh)
					mesh->visible = true;

			});
	}

	void Update()
	{	
		//bind GLSL program
		program->Use();

		int i = 0;
		scene->registry.view<CLight>()
			.each([&](auto& light)
			{
				std::string shaderName("light[" + std::to_string(i) + "].position");
				program->SetUniform(shaderName.c_str(), glm::vec3(scene->camera.GetViewMatrix() * glm::vec4(light.position, 1)));
				shaderName = std::string("light[" + std::to_string(i) + "].intensity");
				program->SetUniform(shaderName.c_str(), light.intensity);
				shaderName = std::string("light[" + std::to_string(i) + "].color");
				program->SetUniform(shaderName.c_str(), light.color);
				i++;
			});
		program->SetUniform("light_count", i);

		scene->registry.view<CTriMesh>()
			.each([&](const auto& entity, auto& mesh)
			{
				program->vaos[entity2VAOIndex[entity]].visible = mesh.visible;
				CPhongMaterial* material = scene->registry.try_get<CPhongMaterial>(entity);
				CTransform* transform = scene->registry.try_get<CTransform>(entity);
				
				const glm::mat4 mv =  scene->camera.GetViewMatrix() * (transform ? 
					transform->GetModelMatrix() : glm::mat4(1.0f));
				const glm::mat4 mvp = scene->camera.GetProjectionMatrix() * mv;
					
				program->SetUniform("material.ka", material ? material->ambient : glm::vec3(0.0f));
				program->SetUniform("material.kd", material ? material->diffuse : glm::vec3(0.0f));
				program->SetUniform("material.ks", material ? material->specular : glm::vec3(0.0f));
				program->SetUniform("material.shininess", material ? material->shininess : 0.0f);
				
				program->SetUniform("to_screen_space", mvp);
				program->SetUniform("to_view_space", mv);
				program->SetUniform("normals_to_view_space",
					glm::transpose(glm::inverse(glm::mat3(mv))));
				
				CImageMaps* imgMaps = scene->registry.try_get<CImageMaps>(entity);
				if (imgMaps && !imgMaps->dirty)
				{
					//iterate over all imagemaps
					for (auto it = imgMaps->mapsBegin(); it != imgMaps->mapsEnd(); ++it)
					{
						int texIndex = entity2TextureIndices[entity].v[(int)it->first];
						//bind texture
						if (texIndex >= 0)
						{
							program->textures[texIndex].Bind();
							//set uniform
							const std::string uniformName = std::string("has_texture[") + std::to_string(((int)it->first)) + std::string("]");
							program->SetUniform(uniformName.c_str(), 1);
							const std::string uniformName2 = std::string("tex_list[") + std::to_string(((int)it->first)) + std::string("]");
							program->SetUniform(uniformName2.c_str(), ((int)it->first));
						}
					}
				}
				program->SetUniform("shading_mode", ((int)mesh.GetShadingMode()));
				
				program->vaos[entity2VAOIndex[entity]].Draw();
				for (int i = 0; i < 5; i++)
				{
					const std::string uniformName = std::string("has_texture[") + std::to_string(i) + std::string("]");
					program->SetUniform(uniformName.c_str(), 0);
				}
				for (auto tex : program->textures)
				{
					tex.Unbind();
				}
			});
	}

	void End()
	{
		printf("Shutting down Renderer");
	}

	void UpdateGUI()
	{}
	
	/*
	* Recompile the shaders
	*/
	inline void RecompileShaders()
	{
		if (program->CompileShaders())
		{
			printf("Shaders compiled successfully\n");
			program->AttachVertexShader();
			program->AttachFragmentShader();
		}
		else
			printf("Shaders compilation failed\n");
	}
	/*
	* Reloads and recompiles shaders
	*/
	inline void ReloadShaders()
	{
		program->SetVertexShaderSourceFromFile("../assets/shaders/phong_textured/shader.vert");
		program->SetFragmentShaderSourceFromFile("../assets/shaders/phong_textured/shader.frag");
		RecompileShaders();
	}

	/*
	* Points camera to mesh center
	*/
	inline void ResetCamera()
	{
		scene->camera.SetOrbitDistance(25.f);
		scene->camera.SetCenter({ 0,0,0 });
		scene->camera.SetOrbitAngles({ 90,0,0 });
	}

	void OnWindowResize(int w, int h)
	{
		scene->camera.SetAspectRatio((float)w / (float)h);
	}

	void OnKeyboard(int key, int scancode, int action, int mods)
	{
		// if GLFW_ESC is pressed exit the application
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(GLFWHandler::GetInstance().GetWindowPointer(), true);
		if (key == GLFW_KEY_F6 && action == GLFW_PRESS)
			RecompileShaders();
		if (key == GLFW_KEY_F5 && action == GLFW_PRESS)
			ReloadShaders();
		if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
			ResetCamera();
		if (key == GLFW_KEY_P && action == GLFW_PRESS)
			scene->camera.SetPerspective(!scene->camera.IsPerspective());
		if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS)
			ctrlDown = true;
		else if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_RELEASE)
			ctrlDown = false;
		if (key == GLFW_KEY_LEFT_ALT && action == GLFW_PRESS)
			altDown = true;
		else if (key == GLFW_KEY_LEFT_ALT && action == GLFW_RELEASE)
			altDown = false;
	}

	//orbit camera
	void OnMouseButton(int button, int action, int mods)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT))
			m1Down = true;
		else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
			m1Down = false;
		if (button == GLFW_MOUSE_BUTTON_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT))
			m2Down = true;
		else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
			m2Down = false;
	}
	//orbit camera
	void OnMouseMove(double x, double y)
	{
		bool updateMousePos = true;
		glm::vec2 deltaPos(prevMousePos.x - x, prevMousePos.y - y);
		if(m1Down && altDown)//Apllies to all textures with cameras NO EXCEPTIONS... REMOVE IF YOU ARE DOING SOMETHING ELABORATE
		{
			scene->registry.view<CImageMaps>()
				.each([&](auto& maps)
					{
						if (!maps.dirty)
						{
							for (auto it = maps.mapsBegin(); it != maps.mapsEnd(); ++it)
							{
								if (it->second.IsRenderedImage())
								{
									it->second.GetRenderedImageCamera().SetOrbitAngles(
										it->second.GetRenderedImageCamera().GetOrbitAngles()
										- glm::vec3(deltaPos.y * 0.5f, -deltaPos.x * 0.4f, 0.f));
								}
							}
						}
					});
		}
		else if (m1Down && ctrlDown)
		{
			//UGLY CODE HERE
			//fetch the first light
			auto view2 = scene->registry.view<CLight>();
			int i = 0;
			view2.each([&](auto& light)
				{
					if (i == 0)
					{
						//get the angles around 0,0,0 based on position
						glm::vec3 angles = glm::vec3(
							asin((light.position.y) / glm::length(light.position)),
							atan2f((-light.position.z), (light.position.x)) + 1.57f,
							0.f);
						//calculate new spherical coordinates using angles - mouse delta 
						angles = angles + glm::vec3(deltaPos.y * .005f, -deltaPos.x * .004f, 0.f);
						if (angles.x > 89.5f)
							angles.x = 89.5f;
						if (angles.x < -89.5f)
							angles.x = -89.5f;
						const float theta = angles.x;
						const float phi = angles.y;
						glm::vec3 unitSpherePos = {
							cos(theta) * sin(phi),
							sin(theta),
							cos(theta) * cos(phi)
						};
						//Preserve the distance to center and set the new position
						light.position = unitSpherePos * glm::length(light.position);
					}
					i++;
				});

		}
		else if (m1Down)
			scene->camera.SetOrbitAngles(scene->camera.GetOrbitAngles()
				- glm::vec3(deltaPos.y * 0.5f, -deltaPos.x * 0.4f, 0.f));
		
		if (altDown && m2Down)
		{
			scene->registry.view<CImageMaps>()
				.each([&](auto& maps)
					{
						if (!maps.dirty)
						{
							for (auto it = maps.mapsBegin(); it != maps.mapsEnd(); ++it)
							{
								if (it->second.IsRenderedImage())
								{
									it->second.GetRenderedImageCamera().SetOrbitDistance(
										it->second.GetRenderedImageCamera().GetOrbitDistance() + deltaPos.y * 0.05f);
								}
							}
						}
					});
		}
		else if (m2Down)
			scene->camera.SetOrbitDistance(scene->camera.GetOrbitDistance() + deltaPos.y * 0.05f);
		
		if(updateMousePos)
			prevMousePos = { x,y };


	}

private:
	//--orbit controls--//
	bool m1Down = false;
	bool m2Down = false;
	glm::vec2 prevMousePos;

	bool ctrlDown = false;
	bool altDown = false;
};


class RBPlaygroundRenderer : public Renderer<RBPlaygroundRenderer>
{
public:
	RBPlaygroundRenderer(std::shared_ptr<Scene> scene) :Renderer(scene) {}
	~RBPlaygroundRenderer() {}

	//override ParseArguments
	void ParseArguments(int argc, char const* argv[])
	{}

	void Start()
	{
		printf("Initializing Renderer\n");
		program->SetGLClearFlags(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		program->CreatePipelineFromFiles("../assets/shaders/phong/shader.vert",
			"../assets/shaders/phong/shader.frag");
		program->SetClearColor({ 0.01f,0.f,0.05f,1.f });
		
		//OnGeometryChange();

		//Init camera
		int windowWidth, windowHeight;
		glfwGetWindowSize(GLFWHandler::GetInstance().GetWindowPointer(), &windowWidth, &windowHeight);
		scene->camera = Camera(glm::vec3(0.f, 0.f, 0.0f), glm::vec3(0.0f, 0, 0), 1.f,
			45.f, 0.01f, 100000.f, (float)windowWidth / (float)windowHeight, true);

		ResetCamera();

		auto arrow = scene->GetSceneObject("arrow");
		//scene->GetComponent<CVertexArrayObject>(arrow).visible = false;
		auto& material = scene->GetComponent<CPhongMaterial>(arrow);
		material.ambient = glm::vec3(0.0, 1.0f, 0.0f);
		material.diffuse = glm::vec3(0.0, 1.0f, 0.0f);
		material.specular = glm::vec3(0.0, 1.0f, 0.0f);
		material.shininess = 200.f;

		auto view3 = scene->registry.view<CTransform, CTriMesh>();
		view3.each([&](auto& transform, auto& mesh)
			{
				transform.SetScale(glm::vec3(1) *
				(scene->camera.IsPerspective() ? 1.f : 1.f / glm::length(scene->camera.GetLookAtEye())));
				transform.SetEulerRotation(glm::vec3(glm::radians(-90.f), 0, 0));
				transform.SetPivot(mesh.GetBoundingBoxCenter());
			});
		scene->GetComponent<CTransform>(arrow).SetPivot(glm::vec3(-7.0f,0,0));

	}

	void PreUpdate()
	{
		int i = 0;
		scene->registry.view<CLight>()
			.each([&](auto& light)
			{
				std::string shaderName("light[" + std::to_string(i) + "].position");
				program->SetUniform(shaderName.c_str(), glm::vec3(scene->camera.GetViewMatrix() * glm::vec4(light.position, 1)));
				shaderName = std::string("light[" + std::to_string(i) + "].intensity");
				program->SetUniform(shaderName.c_str(), light.intensity);
				shaderName = std::string("light[" + std::to_string(i) + "].color");
				program->SetUniform(shaderName.c_str(), light.color);
				i++;
			});
		program->SetUniform("light_count", i);
	}

	void Update()
	{
		//scene->registry.view <CBoundingBox, CVertexArrayObject>().each([&](const auto entity, 
		//	const auto bbox, auto& vao)
		//	{
		//			const auto mv = scene->camera.GetViewMatrix();
		//			const auto mvp = scene->camera.GetProjectionMatrix() * mv;
		//			program->SetUniform("to_screen_space", mvp);
		//			program->SetUniform("to_view_space", mv);
		//			program->SetUniform("normals_to_view_space",
		//				glm::transpose(glm::inverse(glm::mat3(mv))));
		//			program->SetUniform("material.ka", glm::vec3(1.0));
		//			program->SetUniform("material.kd", glm::vec3(1.0));
		//			program->SetUniform("material.ks", glm::vec3(1.0));
		//			program->SetUniform("material.shininess", 0);
		//			program->Use();
		//			vao.Draw();
		//	});

		//scene->registry.view<CTransform, CVertexArrayObject, CPhongMaterial>
		//	(entt::exclude<CBoundingBox>)
		//	.each([&](const auto entity, auto& transform, auto& vao, auto& material)
		//	{
		//		if (entity != scene->GetSceneObject("arrow"))
		//		{

		//			const auto mv = scene->camera.GetViewMatrix() * transform.GetModelMatrix();
		//			const auto mvp = scene->camera.GetProjectionMatrix() * mv;
		//			program->SetUniform("to_screen_space", mvp);
		//			program->SetUniform("to_view_space", mv);
		//			program->SetUniform("normals_to_view_space",
		//				glm::transpose(glm::inverse(glm::mat3(mv))));
		//			program->SetUniform("material.ka", material.ambient);
		//			program->SetUniform("material.kd", material.diffuse);
		//			program->SetUniform("material.ks", material.specular);
		//			program->SetUniform("material.shininess", material.shininess);

		//			//bind GLSL program
		//			program->Use();
		//			vao.Draw();


		//			auto arrow = scene->GetSceneObject("arrow");
		//			auto tra = scene->GetComponent<CTransform>(arrow);
		//			auto mat = scene->GetComponent<CPhongMaterial>(arrow);
		//			tra.SetPosition(transform.GetPosition());
		//			const auto mv2 = scene->camera.GetViewMatrix() * tra.GetModelMatrix();
		//			const auto mvp2 = scene->camera.GetProjectionMatrix() * mv2;
		//			program->SetUniform("to_screen_space", mvp2);
		//			program->SetUniform("to_view_space", mv2);
		//			program->SetUniform("normals_to_view_space",
		//				glm::transpose(glm::inverse(glm::mat3(mv2))));

		//			program->SetUniform("material.ka", mat.ambient);
		//			program->SetUniform("material.kd", mat.diffuse);
		//			program->SetUniform("material.ks", mat.specular);
		//			program->SetUniform("material.shininess", mat.shininess);

		//			scene->GetComponent<CVertexArrayObject>(arrow).Draw();
		//		}

		//	});
	}

	void End()
	{
		printf("Shutting down Renderer");
	}

	void UpdateGUI()
	{
		const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowSize(ImVec2(main_viewport->WorkSize.x / 5, main_viewport->WorkSize.y / 2));
		ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkSize.x - main_viewport->WorkSize.x / 5 - 5, main_viewport->WorkPos.y + 5));
		ImGui::Begin("Control panel", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
		if (ImGui::CollapsingHeader("SceneObjects", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (auto it = scene->sceneObjectsBegin(); it != scene->sceneObjectsEnd(); ++it)
				ImGui::Text(it->first.c_str());
			if (ImGui::Button("Reset Particle"))
				ResetParticle();
			
			ImGui::SameLine();
			static bool enableBB = false;
			if (ImGui::Checkbox("Bounding Box", &enableBB))
			{
				if (enableBB)
				{
					scene->CreateBoundingBox(glm::vec3(-25), glm::vec3(25), program->GetID());
				}
				else
					scene->RemoveSceneObject("boundingbox");
			}
			
			static bool enableVF = false;
			if (ImGui::Checkbox("VelocityField", &enableVF))
			{
				if (enableVF)
				{
					auto velocityF = scene->CreateSceneObject("velocityField");
					scene->registry.emplace<CVelocityField2D>(velocityF, [](glm::vec2 pos)
						{
							float x = pos.x;
							float y = pos.y;
							return glm::vec2(
								1.0f, -y*0.3f
								/*5.f * cos(0.5f * glm::pi<float>() * GLFWHandler::GetTime()),
								sin(2.f * glm::pi<float>() * GLFWHandler::GetTime())*/
							);
						}, FieldPlane::XZ).scaling = 0.5f;
				}
				else
					scene->RemoveSceneObject("velocityField");
			}
			ImGui::SameLine();
			static bool enableFF = false;
			if (ImGui::Checkbox("ForceField", &enableFF))
			{
				if (enableFF)
				{
					auto forceF = scene->CreateSceneObject("forceField");
					scene->registry.emplace<CForceField2D>(forceF, [](glm::vec2 pos)
						{
							float x = pos.x;
							float y = pos.y;
							float radius = sqrt(x * x + y * y);
							float angle = atan2(y, x) + 0.5f * glm::pi<float>();
							return glm::vec2(-radius * cos(angle), - radius * sin(angle));
								}, FieldPlane::XZ).scaling = 0.5f;
				}
				else
					scene->RemoveSceneObject("forceField");
			}
			std::string text(scene->explicit_euler ? "Expilicit Euler" : "Implicit Euler");
			ImGui::Text(text.c_str());
			ImGui::SameLine();
			ImGui::ToggleButton("Euler", &scene->explicit_euler);
			auto transform = scene->GetComponent<CTransform>(scene->GetSceneObject("sphere"));
			glm::vec3 pos = transform.GetPosition();
			if (ImGui::DragFloat3("Position##1", &pos[0], 0.01f))
			{
				transform.SetPosition(pos);
			}
		}
		if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::Button("Reset Camera(F1)"))
				ResetCamera();
			ImGui::SameLine();
			if (ImGui::Button("Look At Particle"))
				LookAndPosition(scene->GetComponent<CTransform>(scene->GetSceneObject("sphere")).GetPosition());
			glm::vec3 target = scene->camera.GetCenter();
			if (ImGui::DragFloat3("Target", &target[0], 0.01f))
			{
				scene->camera.SetCenter(target);
			}
			glm::vec3 eye = scene->camera.GetLookAtEye();
			if (ImGui::DragFloat3("Position", &eye[0], 0.01f))
			{
				scene->camera.SetLookAtEye(eye);
			}
		}
		if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen))
		{
			int i = 0;
			auto view = scene->registry.view<CPhongMaterial>();
			view.each([&](auto& material)
				{
					std::string field1(std::string("Specular##") + std::to_string(i));
					std::string field2(std::string("Diffuse##") + std::to_string(i));
					std::string field3(std::string("Ambient##") + std::to_string(i));
					std::string field4(std::string("Shininess##") + std::to_string(i));
					ImGui::ColorEdit3(field1.c_str(), &material.specular[0]);
					ImGui::ColorEdit3(field2.c_str(), &material.diffuse[0]);
					ImGui::ColorEdit3(field3.c_str(), &material.ambient[0]);
					ImGui::DragFloat(field4.c_str(), &material.shininess, 0.1f, 0.f, 500.f);
					ImGui::Separator();
			i++;
				});
		}
		if (ImGui::CollapsingHeader("Lights", ImGuiTreeNodeFlags_DefaultOpen))
		{
			int i = 0;
			scene->registry.view<CLight>()
				.each([&](auto& light)
				{
					std::string field1(std::string("Position##") + std::to_string(i));
					std::string field2(std::string("Intensity##") + std::to_string(i));
					std::string field3(std::string("Color##") + std::to_string(i));
					ImGui::DragFloat3(field1.c_str(), &light.position[0], 0.01f);
					ImGui::DragFloat(field2.c_str(), &light.intensity, 0.001f, 0.0f, 1.f);
					ImGui::ColorEdit3(field3.c_str(), &light.color[0], 0.01f);
					ImGui::Separator();
			i++;
				});
		}
		if (ImGui::CollapsingHeader("Shaders", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::Button("Read Shaders(F5)"))
			{
				ReloadShaders();
			}
			ImGui::SameLine();
			if (ImGui::Button("Compile Shaders(F6)"))
			{
				RecompileShaders();
			}
		}

		ImGui::End();
	}

	/*
	* Recompile the shaders
	*/
	inline void RecompileShaders()
	{
		if (program->CompileShaders())
		{
			printf("Shaders compiled successfully\n");
			program->AttachVertexShader();
			program->AttachFragmentShader();
		}
		else
			printf("Shaders compilation failed\n");
	}
	/*
	* Reloads and recompiles shaders
	*/
	inline void ReloadShaders()
	{
		program->SetVertexShaderSourceFromFile("../assets/shaders/phong/shader.vert");
		program->SetFragmentShaderSourceFromFile("../assets/shaders/phong/shader.frag");
		RecompileShaders();
	}

	/*
	* Points camera to 0,0,0 center
	*/
	inline void ResetCamera()
	{
		scene->camera.SetOrbitDistance(50.f);
		scene->camera.SetCenter({ 0,0,0 });
		scene->camera.SetOrbitAngles({ 90,0,0 });
	}

	inline void LookAndPosition(glm::vec3 target)
	{
		scene->camera.SetOrbitDistance(50.f);
		scene->camera.SetCenter(target);
		scene->camera.SetOrbitAngles({ 90,0,0 });
	}

	inline void ResetParticle()
	{
		auto& transform = scene->GetComponent<CTransform>(scene->GetSceneObject("sphere"));
		auto& rb = scene->GetComponent<CRigidBody>(scene->GetSceneObject("sphere"));
		transform.SetPosition(glm::vec3(0));
		transform.SetEulerRotation(glm::vec3(0));
		transform.SetScale(glm::vec3(1));
		rb.position = glm::vec3(0);
		rb.velocity = glm::vec3(0);
		rb.acceleration = glm::vec3(0);
	}

	void OnWindowResize(int w, int h)
	{
		scene->camera.SetAspectRatio((float)w / (float)h);
	}

	void OnKeyboard(int key, int scancode, int action, int mods)
	{
		// if GLFW_ESC is pressed exit the application
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(GLFWHandler::GetInstance().GetWindowPointer(), true);
		if (key == GLFW_KEY_F6 && action == GLFW_PRESS)
			RecompileShaders();
		if (key == GLFW_KEY_F5 && action == GLFW_PRESS)
			ReloadShaders();
		if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
			ResetCamera();
		if (key == GLFW_KEY_P && action == GLFW_PRESS)
			scene->camera.SetPerspective(!scene->camera.IsPerspective());
		if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS)
			ctrlDown = true;
		else if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_RELEASE)
			ctrlDown = false;
		if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS)
			shiftDown = true;
		else if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE)
			shiftDown = false;
	}

	//orbit camera
	void OnMouseButton(int button, int action, int mods)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			m1Down = true;
			/*if(shiftDown)
				scene->GetComponent<CVertexArrayObject>(scene->GetSceneObject("arrow")).visible = true;*/
		}
		else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
		{
			m1Down = false;

			auto view = scene->registry.view<CRigidBody>();
			view.each([&](auto& body)
				{
					body.ApplyForce(force);
					force = glm::vec3(0.0f);
				});
			/*scene->GetComponent<CVertexArrayObject>(scene->GetSceneObject("arrow")).visible = false;*/
		}

		if (button == GLFW_MOUSE_BUTTON_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT))
			m2Down = true;
		else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
			m2Down = false;
	}
	//orbit camera
	void OnMouseMove(double x, double y)
	{
		bool updateMousePos = true;
		glm::vec2 deltaPos(prevMousePos.x - x, prevMousePos.y - y);
		if (m1Down && shiftDown)
		{
			updateMousePos = false;
			const glm::vec3 right =
				glm::normalize(glm::cross(
					glm::normalize(scene->camera.GetCenter() - scene->camera.GetLookAtEye()),
					scene->camera.GetLookAtUp()));
			const glm::vec3 up = glm::cross(glm::normalize(scene->camera.GetCenter() - scene->camera.GetLookAtEye()), -right);
			force = right * deltaPos.x * 0.00008f - up * deltaPos.y * 0.00008f;
			force = glm::normalize(force) * glm::min(glm::length(force), 1.0f);
			
			auto &mat = scene->GetComponent<CPhongMaterial>(scene->GetSceneObject("arrow"));
			glm::vec3 color((1.f - glm::length(force) * COLOR_SHIFT_MULT), glm::length(force) * COLOR_SHIFT_MULT, 0.0f);
			mat.specular = mat.ambient = mat.diffuse = color;
			
			auto &tra = scene->GetComponent<CTransform>(scene->GetSceneObject("arrow"));
			tra.SetScale(glm::vec3(glm::length(force) * SCALE_MULT, 1.f, 1.f));
			tra.SetEulerRotation(glm::vec3(
				asin((-force.y) / glm::length(force)),
				atan2f(force.z, -force.x) - glm::pi<float>(),
				0.f));
		}
		else if (m1Down)
			scene->camera.SetOrbitAngles(scene->camera.GetOrbitAngles()
				- glm::vec3(deltaPos.y * 0.5f, -deltaPos.x * 0.4f, 0.f));

		if (m2Down)
			scene->camera.SetOrbitDistance(scene->camera.GetOrbitDistance() + deltaPos.y * 0.05f);

		if (updateMousePos)
			prevMousePos = { x,y };
	}

private:
	//--orbit controls--//
	bool m1Down = false;
	bool m2Down = false;
	glm::vec2 prevMousePos;

	bool ctrlDown = false;
	//---Force controls---//
	bool shiftDown = false;
	glm::vec3 force = glm::vec3(0.0f);

	const float COLOR_SHIFT_MULT = 4.5f;
	const float SCALE_MULT = 10.0f;
};