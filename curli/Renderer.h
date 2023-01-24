#pragma once
#include <glad/glad.h>
#include <GLFWHandler.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cyTriMesh.h>
#include <fstream>
#include <string>
#include <imgui.h>

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

struct Camera
{
private:
	
	glm::vec3 eye = glm::vec3(1.f);
	glm::vec3 center = glm::vec3(0.f);
	glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);
	float fov = 45.f;
	float aspectRatio = 1.0f;
	float nearPlane = 0.1f;
	float farPlane = 100.0f;
	bool perspective = true;
	
	void inline SetViewMatrix()
	{
		viewMatrix = glm::lookAt(eye, center, up);
	}
	void inline SetProjectionMatrix()
	{
		if (perspective)
			projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
		else
			projectionMatrix = glm::ortho(-aspectRatio, aspectRatio, -1.f, 1.f, nearPlane, farPlane);
	}
	
public:
	glm::mat4 viewMatrix = glm::lookAt(eye, center, up);
	glm::mat4 projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);

	Camera()
	{}
	Camera(glm::vec3 eye, glm::vec3 center, glm::vec3 up,
		float fov = 45.f, float near = 0.1f, float far = 100.f, float aspectRatio = 1.f, bool perspective = true)
		:eye(eye), center(center), up(up), fov(fov), nearPlane(near), farPlane(far), aspectRatio(aspectRatio), perspective(perspective)
	{
		viewMatrix = glm::lookAt(eye, center, up);
		if(perspective)
			projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, near, far);
		else
			projectionMatrix = glm::ortho(-aspectRatio, aspectRatio, -1.f, 1.f, near, far);
	}
	void SetPerspective(bool perspective, bool recalculate=false)
	{
		this->perspective = perspective;
		if(recalculate)
			SetProjectionMatrix();
	}
	void SetFOV(float fov, bool recalculate=false)
	{
		this->fov = fov;
		if(recalculate)
			SetProjectionMatrix();
	}
	void SetAspectRatio(float aspectRatio, bool recalculate=false)
	{
		this->aspectRatio = aspectRatio;
		if(recalculate)
			SetProjectionMatrix();
	}
	void SetNearPlane(float nearPlane, bool recalculate=false)
	{
		this->nearPlane = nearPlane;
		if(recalculate)
			SetProjectionMatrix();
	}
	void SetFarPlane(float farPlane, bool recalculate=false)
	{
		this->farPlane = farPlane;
		if(recalculate)
			SetProjectionMatrix();
	}
	void SetEye(glm::vec3 eye, bool recalculate=false)
	{
		this->eye = eye;
		if(recalculate)
			SetViewMatrix();
	}
	void SetCenter(glm::vec3 center, bool recalculate=false)
	{
		this->center = center;
		if(recalculate)
			SetViewMatrix();
	}
	void SetUp(glm::vec3 up, bool recalculate=false)
	{
		this->up = up;
		if(recalculate)
			SetViewMatrix();
	}
};

template <class T>
class Renderer
{
public:
	Renderer()
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
		GLFWHandler::GetInstance().Update();
		//Scene changes
		static_cast<T*>(this)->PreUpdate();
		glClear(clearFlags);
		//Post processing
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

	/*
	* Parses arguments called when application starts
	*/
	void ParseArguments(int argc, char const* argv[]){}
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
	/*
	* For Imgui components
	*/
	void UpdateGUI() {}
};

class AnimatedBGRenderer : public Renderer<AnimatedBGRenderer>
{
public:
	AnimatedBGRenderer() {}
	~AnimatedBGRenderer() {}

	void Start()
	{
		printf("Initializing AnimatedBGRenderer");
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

	void PreUpdate()
	{
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
};

class TeapotRenderer : public Renderer<TeapotRenderer>
{
public:
	TeapotRenderer() {}
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
		camera = Camera(glm::vec3(0.f, 0.f, 10.0f), glm::vec3(0.0f),
			glm::vec3(0.f, 1.f, 0.f), 45.f, 0.01f, 100.f,
			(float)windowWidth / (float)windowHeight, false);
		//create mvp matrix
		//auto tmp = (teapot.GetBoundMax() + teapot.GetBoundMin()) *.5f;
		//glm::vec3 center(tmp.x, tmp.y, tmp.z);
		//modelMatrix = glm::translate(glm::mat4(1.0), -center);

		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.05f));
		LookAtMesh();

		mvp = camera.projectionMatrix * camera.viewMatrix * modelMatrix;
		GLuint mvpID = glGetUniformLocation(this->program, "mvp");
		glUniformMatrix4fv(mvpID, 1, GL_FALSE, &mvp[0][0]);

	}

	void PreUpdate()
	{
		int windowWidth, windowHeight;
		glfwGetWindowSize(GLFWHandler::GetInstance().GetWindowPointer(), &windowWidth, &windowHeight);
		camera.SetAspectRatio((float)windowWidth/(float)windowHeight, true);

		//rotate around y with time
		modelMatrix = glm::rotate(glm::mat4(1.0), -(float)glfwGetTime() * 0.5f, glm::vec3(0.f, 0.f, 1.f));
		//modelMatrix = glm::translate(modelMatrix, center);
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.05f));
		mvp = camera.projectionMatrix * camera.viewMatrix * modelMatrix;
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
		ImGui::Begin("Control panel");
		if (ImGui::Button("Read Shader Files"))
		{
			vertexShader.SetSourceFromFile("../assets/shaders/simple/shader.vert");
			fragmentShader.SetSourceFromFile("../assets/shaders/simple/shader.frag");
		}
		if (ImGui::Button("Recompile Shaders(F6)"))
		{
			
		}
		if (ImGui::Button("Look at Teapot"))
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
		camera.SetCenter(center);
		camera.SetEye({ 0.0, 5.f, 0.0f});
		camera.SetUp({ 0.f, 0.f, 1.f }, true);
	}

private:
	Camera camera;
	cyTriMesh teapot;
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	glm::mat4 mvp = glm::mat4(1.0f);
	GLuint VAO;
	GLuint VBO;
};