#pragma once
#include <Scene.h>
#include <OpenGLProgram.h>
#include <glad/glad.h>
#include <GLFWHandler.h>
#include <iostream>
#include <cyTriMesh.h>
#include <fstream>
#include <string>
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

		//glEnable(GL_DEPTH_TEST);//!!!

		//vertexShader.glID = glCreateShader(GL_VERTEX_SHADER);
		//fragmentShader.glID = glCreateShader(GL_FRAGMENT_SHADER);
		//
		////Set shader sources& compile
		//vertexShader.SetSourceFromFile("../assets/shaders/simple/shader.vert", true);//todo
		//fragmentShader.SetSourceFromFile("../assets/shaders/simple/shader.frag", true);//todo:fix the path

		////Attach shaders
		//vertexShader.AttachShader(this->program);
		//fragmentShader.AttachShader(this->program);
		
		program->CreatePipelineFromFiles("../assets/shaders/simple/shader.vert",
			"../assets/shaders/simple/shader.frag");


		//create&bind vertex array object
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		//create&bind vertex buffer object
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		//set buffer data
		entt::entity entity{};
		auto mesh = scene->GetComponent<CTriMesh>(entity);
		
		glBufferData(GL_ARRAY_BUFFER, mesh.GetNumVertices() * sizeof(float)*3, 
			&mesh.GetMesh().V(0), GL_STATIC_DRAW);

		//bind to GLSL attribute
		GLuint pos = glGetAttribLocation(program->glID, "pos");
		glEnableVertexAttribArray(pos);
		glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, 0);

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
		auto view = scene->registry.view<CTransform, CTriMesh>();
		//upload mvp to GLSL uniform
		GLuint mvpID = glGetUniformLocation(program->glID, "mvp");

		view.each([&](auto& transform, auto& mesh)
			{

				mvp = scene->camera.GetProjectionMatrix() * scene->camera.GetViewMatrix() * transform.GetModelMatrix();
				glUniformMatrix4fv(mvpID, 1, GL_FALSE, &mvp[0][0]);
				//bind GLSL program
				glUseProgram(program->glID);
				glDrawArrays(GL_POINTS, 0, mesh.GetNumVertices());
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
	
	glm::mat4 mvp = glm::mat4(1.0f);
	GLuint VAO;
	GLuint VBO;
};

class PhongRenderer : Renderer<PhongRenderer>
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
		//clearFlags = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;

		//glEnable(GL_DEPTH_TEST);//!!!

		/*vertexShader.glID = glCreateShader(GL_VERTEX_SHADER);
		fragmentShader.glID = glCreateShader(GL_FRAGMENT_SHADER);*/

		//Set shader sources& compile
		//vertexShader.SetSourceFromFile("../assets/shaders/phong/shader.vert", true);//todo
		//fragmentShader.SetSourceFromFile("../assets/shaders/phong/shader.frag", true);//todo:fix the path

		////Attach shaders
		//vertexShader.AttachShader(this->program);
		//fragmentShader.AttachShader(this->program);


		//create&bind vertex array object
		//glGenVertexArrays(1, &VAO);
		//glBindVertexArray(VAO);

		////create&bind vertex buffer object
		//glGenBuffers(1, &VBO);
		//glBindBuffer(GL_ARRAY_BUFFER, VBO);

		////set buffer data
		//entt::entity entity{};
		//auto mesh = scene->GetComponent<CTriMesh>(entity);

		//glBufferData(GL_ARRAY_BUFFER, mesh.GetNumVertices() * sizeof(float) * 3,
		//	&mesh.GetMesh().V(0), GL_STATIC_DRAW);

		////bind to GLSL attribute
		//GLuint pos = glGetAttribLocation(this->program, "pos");
		//glEnableVertexAttribArray(pos);
		//glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, 0);

		////Init camera
		//int windowWidth, windowHeight;
		//glfwGetWindowSize(GLFWHandler::GetInstance().GetWindowPointer(), &windowWidth, &windowHeight);
		//scene->camera = Camera(glm::vec3(0.f, 0.f, 0.0f), glm::vec3(0.0f, 0, 0), 1.f,
		//	45.f, 0.01f, 100000.f, (float)windowWidth / (float)windowHeight, true);

		//LookAtMesh();
	}

	void PreUpdate()
	{
		//for each element with CTransform in the scene registry loop
		auto view = scene->registry.view<CTransform, CTriMesh>();

		view.each([&](auto& transform, auto& mesh)
			{
				transform.SetScale(glm::vec3(0.05f));
				transform.SetEulerRotation(glm::vec3(glm::radians(-90.f), 0, (float)glfwGetTime() * 0.5f));
				transform.SetPosition(-mesh.GetBoundingBoxCenter());
			});
	}

	void Update()
	{
		auto view = scene->registry.view<CTransform, CTriMesh>();
		//upload mvp to GLSL uniform
		GLuint mvpID = glGetUniformLocation(program->glID, "mvp");

		view.each([&](auto& transform, auto& mesh)
			{

				//mvp = scene->camera.GetProjectionMatrix() * scene->camera.GetViewMatrix() * transform.GetModelMatrix();
				//glUniformMatrix4fv(mvpID, 1, GL_FALSE, &mvp[0][0]);
				////bind GLSL program
				//glUseProgram(this->program);
				//glDrawArrays(GL_POINTS, 0, mesh.GetNumVertices());
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