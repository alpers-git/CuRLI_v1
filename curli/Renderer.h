#pragma once
#include <GLFWHandler.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

//Implement a renderer class with Init, Render, Shutdown functions
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
		static_cast<T*>(this)->Start();
		glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
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
	
private:
	GLbitfield clearFlags = GL_COLOR_BUFFER_BIT;
	glm::vec4 clearColor = glm::vec4(0.f,0.f,0.f,1.f);
	//unsigned int VBO, VAO, EBO;
	//unsigned int shaderProgram;

	//Called before render loop
	virtual void Start() = 0;
	//Called During Render
	virtual void Update() = 0;
	//Called During Render after clear buffers calls
	virtual void PostUpdate() = 0;
	//Called after render loop
	virtual void End() = 0;
	//For Imgui components
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