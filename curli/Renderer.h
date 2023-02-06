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

		
		const auto view = scene->registry.view<CTriMesh>();
		// use an extended callback
		view.each([&](const auto entity, const auto& mesh)
			{
				scene->registry.emplace<CVertexArrayObject>(entity);
			});
		const auto viewVAO = scene->registry.view<CVertexArrayObject, CTriMesh>();
		viewVAO.each([&](const auto entity, auto& vao, auto& mesh)
			{
				vao.CreateVAO();
				VertexBufferObject vbo(
					&mesh.GetMesh().V(0),
					mesh.GetNumVertices(),
					GL_FLOAT,
					"pos",
					3,
					program->GetID());
				vao.AddVBO(vbo);
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
		auto view = scene->registry.view<CTransform, CVertexArrayObject>();

		view.each([&](auto& transform, auto& vao)
			{

				const auto mvp = scene->camera.GetProjectionMatrix() * scene->camera.GetViewMatrix() * transform.GetModelMatrix();
				program->SetUniform("mvp", mvp);
				//bind GLSL program
				program->Use();
				vao.Draw(GL_POINTS);
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

		const auto view = scene->registry.view<CTriMesh>();
		// use an extended callback
		view.each([&](const auto entity, const auto& mesh)
			{
				scene->registry.emplace<CVertexArrayObject>(entity);
			});
		const auto viewVAO = scene->registry.view<CVertexArrayObject, CTriMesh>();
		viewVAO.each([&](const auto entity, auto& vao, auto& mesh)
			{
				vao.CreateVAO();
				VertexBufferObject vertexVBO(
				mesh.GetVertexDataPtr(),
				mesh.GetNumVertices(),
				GL_FLOAT,
				"pos",
				3,
				program->GetID());
				vao.AddVBO(vertexVBO);
				
				VertexBufferObject normalsVBO(
					mesh.GetNormalDataPtr(),
					mesh.GetNumNormals(),
					GL_FLOAT,
					"norm",
					3,
					program->GetID());
				vao.AddVBO(normalsVBO);

				vao.CreateEBO((unsigned int *)mesh.GetFaceDataPtr(), mesh.GetNumFaces()*3);
			});

		material.diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
		material.specular = glm::vec3(0.9f, 0.9f, 0.9f);
		material.ambient = glm::vec3(0.2f, 0.2f, 0.3f);
		material.shininess = 450.f;

		//Init camera
		int windowWidth, windowHeight;
		glfwGetWindowSize(GLFWHandler::GetInstance().GetWindowPointer(), &windowWidth, &windowHeight);
		scene->camera = Camera(glm::vec3(0.f, 0.f, 0.0f), glm::vec3(0.0f, 0, 0), 1.f,
			45.f, 0.01f, 100000.f, (float)windowWidth / (float)windowHeight, true);

		LookAtMesh();
	}

	void PreUpdate()
	{
		//for each element with CTransform in the scene registry loop
		auto view = scene->registry.view<CTransform, CTriMesh>();
		int i = 0;
		view.each([&](auto& transform, auto& mesh)
			{
				transform.SetScale(glm::vec3(0.05f) *
				(scene->camera.IsPerspective() ? 1.f : 1.f / glm::length(scene->camera.GetLookAtEye())));
				transform.SetEulerRotation(glm::vec3(glm::radians(-90.f ), i * 60.f, (float)glfwGetTime() * 0.5f));
				transform.SetPosition(-mesh.GetBoundingBoxCenter() + glm::vec3(i*15));
				i++;
			});

		i = 0;
		auto view2 = scene->registry.view<CLight>();
		view2.each([&](auto& light)
			{
				std::string shaderName("light[" + std::to_string(i) + "].position");
				program->SetUniform(shaderName.c_str(), glm::vec3(scene->camera.GetViewMatrix() * glm::vec4(light.position, 1)));
				shaderName = std::string("light[" + std::to_string(i) + "].intensity");
				program->SetUniform(shaderName.c_str(), light.intensity);
				i++;
			});
		program->SetUniform("light_count", i);
	}

	void Update()
	{	
		auto view2 = scene->registry.view<CTransform, CVertexArrayObject>();
		view2.each([&](auto& transform, auto& vao)
			{

				const auto mv =  scene->camera.GetViewMatrix() * transform.GetModelMatrix();
				const auto mvp = scene->camera.GetProjectionMatrix() * mv;
				program->SetUniform("to_screen_space", mvp);
				program->SetUniform("to_view_space", mv);
				program->SetUniform("normals_to_view_space",
					glm::transpose(glm::inverse(glm::mat3(mv))));
				program->SetUniform("material_ka", material.ambient);
				program->SetUniform("material_kd", material.diffuse);
				program->SetUniform("material_ks", material.specular);
				program->SetUniform("material_shininess", material.shininess);
				//bind GLSL program
				program->Use();
				vao.Draw(GL_TRIANGLES);
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
				LookAtMesh();
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
			ImGui::ColorEdit3("Specular", &material.specular[0]);
			ImGui::ColorEdit3("Diffuse", &material.diffuse[0]);
			ImGui::ColorEdit3("Ambient", &material.ambient[0]);
			ImGui::DragFloat("Shininess", &material.shininess, 0.1f, 0.f, 500.f);
		}
		if (ImGui::CollapsingHeader("Lights", ImGuiTreeNodeFlags_DefaultOpen))
		{
			int i  = 0;
			auto view = scene->registry.view<CLight>();
			view.each([&](auto& light)
				{
					std::string field1(std::string("Intensity##") + std::to_string(i));
					std::string field2(std::string("Light Pos##") + std::to_string(i));
					ImGui::DragFloat(field1.c_str(), &light.intensity, 0.001f, 0.0f, 1.f);
					ImGui::DragFloat3( field2.c_str(), &light.position[0], 0.01f);
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
	inline void LookAtMesh()
	{
		scene->camera.SetOrbitDistance(3.f);
		scene->camera.SetCenter({ 0,0,0 });
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
		prevMousePos = { x,y };


	}

private:
	//--orbit controls--//
	bool m1Down = false;
	bool m2Down = false;
	glm::vec2 prevMousePos;
	//---phong-material---//
	struct {
		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;
		float shininess;
	}material;

	bool ctrlDown = false;
};