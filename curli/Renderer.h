#pragma once
#include <Scene.h>
#include <glad/glad.h>
#include <GLFWHandler.h>
#include <iostream>
#include <cyTriMesh.h>
#include <fstream>
#include <string>
#include <imgui.h>

//define a macro function

struct Shader
{
	GLuint glID;
	char* source;

	Shader(char* source = nullptr)
		:source(source)
	{
		glID = 0;
	}
	
	~Shader()
	{
		if(source)
			delete[] source;
		glDeleteShader(glID);
	}

	bool Compile()
	{
		glShaderSource(glID, 1, &source, NULL);
		glCompileShader(glID);
		int success;
		glGetShaderiv(glID, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			GLchar infoLog[512];
			glGetShaderInfoLog(glID, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
			return false;
		}
		return true;
	}
	
	bool AttachShader(GLuint program)
	{
		glAttachShader(program, glID);
		glLinkProgram(program);
		int status;
		glGetProgramiv(glID, GL_LINK_STATUS, &status);
		if (!status)
		{
			char infoLog[512];
			glGetProgramInfoLog(glID, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::ATTACH_FAILED\n" << infoLog << std::endl;
			return false;
		}
		return true;
	}

	void SetSource(const char* src, bool compile = false)
	{
		if (src && source)
			delete[] source;

		if (src)
		{
			//set source to src
			source = new char[strlen(src) + 1];
			strcpy(source, src);
		}
		
		//print source
		printf("------Shader source:------\n");
		std::cout << source << std::endl;
		
		if (compile)
			Compile();
	}

	void SetSourceFromFile(const char* filePath, bool compile = false)
	{
		std::string content;
		std::ifstream fileStream(filePath, std::ios::in);

		if (!fileStream.is_open()) {
			std::cerr << "Could not read file " << filePath << ". File does not exist." << std::endl;
			return;
		}

		std::string line = "";
		while (!fileStream.eof()) {
			std::getline(fileStream, line);
			content.append(line + "\n");
		}

		fileStream.close();
		SetSource(content.c_str(), compile);
	}
		
	
};

template <class T>
class Renderer
{
	friend class GLFWHandler;
public:
	Renderer(Scene& scene)
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
		program = glCreateProgram();
		glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
		static_cast<T*>(this)->Start();
	}
	
	//Renders the Scene and clears the Frame
	void Render()
	{
		GLFWHandler::GetInstance().SwapBuffers();

		//Scene changes
		static_cast<T*>(this)->PreUpdate();

		glClear(clearFlags);
		//Rendering
		static_cast<T*>(this)->Update();
	}
	//Cleans up after render loop exits
	void Terminate()
	{
		static_cast<T*>(this)->End();
		GLFWHandler::GetInstance().Close();
	}
	void SetGLClearFlags(GLbitfield)
	{
		clearFlags = flags;
	}
	void SetClearColor(glm::vec4 color)
	{
		clearColor = color;
		glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
	}
	
	void DrawGUI()
	{
		gui::GetNewImguiFrame();
		static_cast<T*>(this)->UpdateGUI();
		gui::RenderImgui();
	}
	
protected:
	GLbitfield clearFlags = GL_COLOR_BUFFER_BIT;
	glm::vec4 clearColor = glm::vec4(0.02f,0.02f,0.02f,1.f);
	//unsigned int VBO, VAO, EBO;
	GLuint program;
	Shader vertexShader;
	Shader fragmentShader;
	Scene& scene;

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
	AnimatedBGRenderer(Scene& scene) :Renderer(scene) {}
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
		SetClearColor(glm::vec4(glm::mix(
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

class TeapotRenderer : public Renderer<TeapotRenderer>
{
public:
	TeapotRenderer(Scene& scene) :Renderer(scene) {}
	~TeapotRenderer() {}

	//override ParseArguments
	void ParseArguments(int argc, char const* argv[])
	{
		//get first argument and use that to load the mesh using cyTrimesh
		std::string meshPath = argv[1];
		cyTriMesh mesh;
		mesh.LoadFromFileObj(meshPath.c_str());
		SetMesh(mesh);
	}

	void Start()
	{
		printf("Initializing TeapotRenderer\n");
		clearFlags = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;

		glEnable(GL_DEPTH_TEST);//!!!

		vertexShader.glID = glCreateShader(GL_VERTEX_SHADER);
		fragmentShader.glID = glCreateShader(GL_FRAGMENT_SHADER);
		
		//Set shader sources& compile
		vertexShader.SetSourceFromFile("../assets/shaders/simple/shader.vert", true);//todo
		fragmentShader.SetSourceFromFile("../assets/shaders/simple/shader.frag", true);//todo:fix the path

		//Attach shaders
		vertexShader.AttachShader(this->program);
		fragmentShader.AttachShader(this->program);


		//create&bind vertex array object
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		//create&bind vertex buffer object
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		//create a glm::vec3 array of vertices from the teapot
		float* vertices = new float[teapot.NV()*3];
		for (int i = 0; i < teapot.NV(); i++)
		{
			vertices[i*3] = teapot.V(i).x;
			vertices[i*3+1] = teapot.V(i).y;
			vertices[i*3+2] = teapot.V(i).z;
		}
		
		//set buffer data
		glBufferData(GL_ARRAY_BUFFER, teapot.NV() * sizeof(float)*3, vertices, GL_STATIC_DRAW);

		//bind to GLSL attribute
		GLuint pos = glGetAttribLocation(this->program, "pos");
		glEnableVertexAttribArray(pos);
		glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, 0);
		
		delete[] vertices;

		int windowWidth, windowHeight;
		glfwGetWindowSize(GLFWHandler::GetInstance().GetWindowPointer(), &windowWidth, &windowHeight);
		teapot.ComputeBoundingBox();
		//Init camera
		scene.camera = Camera(glm::vec3(0.f, 0.f, 0.0f), glm::vec3(0.0f,0, 0), 1.f,
		45.f, 0.01f, 100000.f, (float)windowWidth / (float)windowHeight, true);

		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.05f));
		//LookAtMesh();

		mvp = scene.camera.GetProjectionMatrix() * scene.camera.GetViewMatrix() * modelMatrix;
		GLuint mvpID = glGetUniformLocation(this->program, "mvp");
		glUniformMatrix4fv(mvpID, 1, GL_FALSE, &mvp[0][0]);

	}

	void PreUpdate()
	{
		modelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.05f) /*
			(camera.IsPerspective() ? 1.f : 1.f / glm::length(camera.GetEye()))*/);
		modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
		//find the center point of the teapot
		glm::vec3 center = glm::cy2GLM(teapot.GetBoundMax() + teapot.GetBoundMin()) * 0.5f;
		//translate the teapot to the center
		modelMatrix = glm::translate(modelMatrix, -center);
		//rotate around y with time
		/*modelMatrix = glm::rotate(glm::mat4(1.0), -(float)glfwGetTime() * 0.5f, glm::vec3(0.f, 0.f, 1.f));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.05f) * 
			(camera.IsPerspective() ? 1.f : 1.f/glm::length(camera.GetEye())) );*/
		printf("preUpdate\n");
		mvp = scene.camera.GetProjectionMatrix() * scene.camera.GetViewMatrix() * modelMatrix;
		//upload mvp to GLSL uniform
		GLuint mvpID = glGetUniformLocation(this->program, "mvp");
		glUniformMatrix4fv(mvpID, 1, GL_FALSE, &mvp[0][0]);
	}

	void Update()
	{
		//bind GLSL program
		glUseProgram(this->program);
		glDrawArrays(GL_POINTS, 0, teapot.NV());
	}

	void End()
	{
		printf("Shutting down TeapotRenderer");
	}

	void UpdateGUI()
	{
		const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 10, main_viewport->WorkPos.y + 10));
		ImGui::Begin("Control panel", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
		if (ImGui::Button("Read Shader Files(F5)"))
		{
			vertexShader.SetSourceFromFile("../assets/shaders/simple/shader.vert");
			fragmentShader.SetSourceFromFile("../assets/shaders/simple/shader.frag");
		}
		if (ImGui::Button("Recompile Shaders(F6)"))
		{
			
		}
		if (ImGui::Button("Look at Teapot(F1)"))
		{
			LookAtMesh();
		}
		ImGui::End();
	}
	
	/*
	* Sets the mesh
	*/
	inline void SetMesh(cyTriMesh& mesh)
	{
		teapot = mesh;
		printf("Mesh with %d vertices has been set\n", teapot.NV());
	}
	/*
	* Recompile the shaders
	*/
	inline void RecompileShaders()
	{
		if (vertexShader.Compile())
		{
			printf("Vertex Shader Recompiled Succesfully\n");
			vertexShader.AttachShader(this->program);
		}
		if (fragmentShader.Compile())
		{
			printf("Fragment Shader Recompiled Succesfully\n");
			fragmentShader.AttachShader(this->program);
		}
	}
	/*
	* Reloads and recompiles shaders
	*/
	inline void ReloadShaders()
	{
		vertexShader.SetSourceFromFile("../assets/shaders/simple/shader.vert");
		fragmentShader.SetSourceFromFile("../assets/shaders/simple/shader.frag");
		RecompileShaders();
	}
	
	/*
	* Points camera to mesh center
	*/
	inline void LookAtMesh()
	{
		//Find the center point of the teapot and apply the modelMatrix transform
		auto tmp = (teapot.GetBoundMax() + teapot.GetBoundMin()) * 0.5f;
		glm::vec3 center = glm::vec4(tmp.x, tmp.y, tmp.z, 1.0f) * modelMatrix;
		
		/*scene.camera.SetLookAtEye({0,5,0.f});
		scene.camera.SetLookAtUp({ 0,0,1.f });*/
		scene.camera.SetOrbitDistance(1.f);
		scene.camera.SetCenter({0,0,0});
	}

	void OnWindowResize(int w, int h)
	{
		scene.camera.SetAspectRatio((float)w / (float)h);
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
			scene.camera.SetPerspective(!scene.camera.IsPerspective());
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
		glm::vec3 right;
		if (m1Down)
		{
			scene.camera.SetOrbitAngles(scene.camera.GetOrbitAngles() - glm::vec3(deltaPos.y * 0.5f, -deltaPos.x * 0.4f, 0.f));
			/*right = glm::cross(scene.camera.GetLookAtEye() - scene.camera.GetCenter(), scene.camera.GetLookAtUp());
			scene.camera.SetLookAtEye(scene.camera.GetLookAtEye() - right * deltaPos.x * 0.01f -
				scene.camera.GetLookAtUp() * deltaPos.y * 0.05f);*/
			//scene.camera.SetCenter({ 0, 0, 0 }, true);
		}
		if (m2Down)
		{
			scene.camera.SetOrbitDistance(scene.camera.GetOrbitDistance() + deltaPos.y * 0.05f);
		}
		prevMousePos = { x,y };
	}

private:
	//--orbit controls--//
	bool m1Down = false;
	bool m2Down = false;
	glm::vec2 prevMousePos;
	//--------------------//
	
	cyTriMesh teapot;
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	glm::mat4 mvp = glm::mat4(1.0f);
	GLuint VAO;
	GLuint VBO;
};