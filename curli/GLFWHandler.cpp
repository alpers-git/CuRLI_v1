#include <iostream>
#include <GLFWHandler.h>

GLFWHandler::GLFWHandler()
{
}

GLFWHandler::~GLFWHandler()
{
}

void GLFWHandler::InitAndCreateWindow(int width, int height, const char* title)
{
	if (!glfwInit())
	{
		std::cout << "Failed to initialize GLFW" << std::endl;
		return;
	}
	
	windowHandle = glfwCreateWindow(width, height, title, NULL, NULL);
	glfwMakeContextCurrent(windowHandle);
	glfwSwapInterval(0); // Enable vsync
}

void GLFWHandler::SwapBuffers()
{
	glfwSwapBuffers(windowHandle);
}

void GLFWHandler::Close()
{
	glfwTerminate();
}


bool GLFWHandler::IsRunning()
{
	return !glfwWindowShouldClose(windowHandle);
}

void GLFWHandler::SetWindowSize(int width, int height)
{
	glfwSetWindowSize(windowHandle, width, height);
}

void GLFWHandler::SetWindowPosition(int x, int y)
{
	glfwSetWindowPos(windowHandle, x, y);
}

void GLFWHandler::SetWindowName(const char* name)
{
	glfwSetWindowTitle(windowHandle, name);
}

void GLFWHandler::SetWindowIcon(const char* path)
{
	/*GLFWimage images[1];
	images[0].pixels = stbi_load(path, &images[0].width, &images[0].height, 0, 4);
	glfwSetWindowIcon(images[0]);*/
}

void GLFWHandler::SetWindowIcon(const char* path, int width, int height)
{
	/*GLFWimage images[1];
	images[0].pixels = stbi_load(path, &images[0].width, &images[0].height, 0, 4);
	glfwSetWindowIcon(images[0]);*/
}

void GLFWHandler::SetKeyboardCallback(GLFWkeyfun callback)
{
	glfwSetKeyCallback(windowHandle, callback);
}