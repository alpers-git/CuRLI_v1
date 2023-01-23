#pragma once
#include <glad/glad.h>
#include <GLFWHandler.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cyTriMesh.h>

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
		if(!source)
			delete[] source;
		glDeleteShader(glID);
	}

	bool Compile()
	{
		glShaderSource(glID, 1, &source, NULL);
		glCompileShader(glID);
		int success;
		char infoLog[512];
		glGetShaderiv(glID, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(glID, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
			return false;
		}
		return true;
	}
	
	void AttachShader(GLuint program)
	{
		glAttachShader(program, glID);
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
		static_cast<T*>(this)->Update();
		glClear(clearFlags);
		//Post processing
		static_cast<T*>(this)->PostUpdate();
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
	glm::vec4 clearColor = glm::vec4(0.02f,0.5f,0.5f,1.f);
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
	virtual void PostUpdate() = 0;
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

	void PostUpdate()
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
		vertexShader.glID = glCreateShader(this->program);
		fragmentShader.glID = glCreateShader(this->program);
		
		//create&bind vertex array object
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		
		//
		
	}

	void Update()
	{
	}

	void PostUpdate()
	{
	}

	void End()
	{
		printf("Shutting down TeapotRenderer");
	}

	void UpdateGUI()
	{
		ImGui::Begin("Test Window");
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
	}

private:
	cyTriMesh teapot;
	GLuint VAO;
	GLuint VBO;
};