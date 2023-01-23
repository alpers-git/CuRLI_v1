#pragma once
#include <glad/glad.h>
#include <GLFWHandler.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cyTriMesh.h>
#include <fstream>
#include <string>

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
	* Called before render loop
	*/
	virtual void Start() = 0;
	/*
	* Called During Render
	*/
	virtual void Update() = 0;
	/*
	* Called During Render after clear buffers calls
	*/
	virtual void PreUpdate() = 0;
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

	void Start()
	{
		printf("Initializing TeapotRenderer\n");
		clearFlags = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT; glGenVertexArrays(1, &VAO);
		vertexShader.glID = glCreateShader(GL_VERTEX_SHADER);
		fragmentShader.glID = glCreateShader(GL_FRAGMENT_SHADER);
		
		//Set shader sources& compile
		vertexShader.SetSourceFromFile("../assets/shaders/simple/shader.vert", true);//todo
		fragmentShader.SetSourceFromFile("../assets/shaders/simple/shader.frag", true);//todo:fix the path

		//Attach shaders
		vertexShader.AttachShader(this->program);
		fragmentShader.AttachShader(this->program);
		printf("shaders %d %d\n", vertexShader.glID, fragmentShader.glID);


		//create&bind vertex array object
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		//create&bind vertex buffer object
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		//create a glm::vec3 array of vertices from the teapot
		float* vertices = new float[teapot.NV()*3];
		for (int i = 0; i < teapot.NV(); i+=3)
		{
			vertices[i] = teapot.V(i).x;
			vertices[i+1] = teapot.V(i).y;
			vertices[i+2] = teapot.V(i).z;
		}
		
		//set buffer data
		glBufferData(GL_ARRAY_BUFFER, teapot.NV() * sizeof(float)*3, vertices, GL_STATIC_DRAW);

		//bind to GLSL attribute
		GLuint pos = glGetAttribLocation(this->program, "pos");
		glEnableVertexAttribArray(pos);
		glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, 0);
		printf("pos: %d\n", pos);
		
		delete[] vertices;

	}

	void Update()
	{
		//bind GLSL program
		glUseProgram(this->program);
		glDrawArrays(GL_POINTS, 0, teapot.NV());
	}

	void PreUpdate()
	{
		GLuint mvpID = glGetUniformLocation(this->program, "mvp");
		printf("mvpID: %d\n", mvpID);

		//center teapot to 0,0 using its bounding box
		teapot.ComputeBoundingBox();
		auto tmp = (teapot.GetBoundMax() + teapot.GetBoundMin()) / 2.0f;
		glm::vec3 centr(tmp.x, tmp.y, tmp.z);
		glm::mat4 model = glm::translate(glm::mat4(1.0f), -centr);
		//scale teapot to fit in a 1x1x1 box
		float scale = 1.0f / glm::max(glm::max(teapot.GetBoundMax().x - teapot.GetBoundMin().x, teapot.GetBoundMax().y - teapot.GetBoundMin().y), teapot.GetBoundMax().z - teapot.GetBoundMin().z);
		model = glm::scale(model, glm::vec3(scale, scale, scale));
		
		//create view matrix
		glm::mat4 view = glm::lookAt(
			glm::vec3(0.0f, 0.0f, 3.0f), //camera position
			glm::vec3(0.0f, 0.0f, 0.0f), //look at
			glm::vec3(0.0f, 1.0f, 0.0f)  //up
		);
		GLFWHandler& glfw = GLFWHandler::GetInstance();
		int windowWidth, windowHeight;
		glfwGetWindowSize(glfw.GetWindowPointer(), &windowWidth, &windowHeight);
		
		//create projection matrix
		glm::mat4 projection = glm::perspective(
			glm::radians(45.0f), //fov
			(float)windowWidth / (float)windowHeight, //aspect ratio
			0.1f, //near plane
			100.0f //far plane
		);
		
		//create mvp matrix
		mvp = projection * view * model;
		
		

		//upload mvp to GLSL uniform
		glUniformMatrix4fv(mvpID, 1, GL_FALSE, &mvp[0][0]);
	}

	void End()
	{
		printf("Shutting down TeapotRenderer");
	}

	void UpdateGUI()
	{
		ImGui::Begin("Control panel");
		if (ImGui::Button("Recompile Shaders(F6)"))
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
		if (ImGui::Button("Center Teapot"))
		{
			//Center Teapot
		}
		ImGui::End();
	}

	inline void SetMesh(cyTriMesh& mesh)
	{
		teapot = mesh;
		printf("Mesh with %d vertices has been set\n", teapot.NV());
	}
	inline void ChangeVertexShaderSrc(char* src)
	{
		vertexShader.SetSource(src);
	}
	inline void ChangeFragmentShaderSrc(char* src)
	{
		fragmentShader.SetSource(src);
	}

private:
	cyTriMesh teapot;
	glm::mat4 mvp = glm::mat4(1.0f);
	GLuint VAO;
	GLuint VBO;
};