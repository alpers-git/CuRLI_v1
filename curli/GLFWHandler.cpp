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
	glfwSwapInterval(1); // Enable vsync

	setCallbacks();
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

void GLFWHandler::setCallbacks()
{
	/*glfwSetWindowCloseCallback(windowHandle,
		[](GLFWwindow* window)
		{
			Event event;
			event.type = Event::Type::WindowClose;
			GLFWHandler::GetInstance().QueueEvent(event);
		});*/
	
	glfwSetWindowSizeCallback(windowHandle,
		[](GLFWwindow* window, int width, int height)
		{
			Event event;
			event.type = Event::Type::WindowResize;
			event.windowResize.width = width;
			event.windowResize.height = height;
			GLFWHandler::GetInstance().QueueEvent(event);
		});

	glfwSetWindowPosCallback(windowHandle,
		[](GLFWwindow* window, int xpos, int ypos)
		{
			Event event;
			event.type = Event::Type::WindowMove;
			event.windowMove.x = xpos;
			event.windowMove.y = ypos;
			GLFWHandler::GetInstance().QueueEvent(event);
		});
	
	glfwSetWindowFocusCallback(windowHandle,
		[](GLFWwindow* window, int focused)
		{
			Event event;
			event.type = Event::Type::WindowFocus;
			event.windowFocus.focused = focused;
			GLFWHandler::GetInstance().QueueEvent(event);
		});

	glfwSetWindowIconifyCallback(windowHandle,
		[](GLFWwindow* window, int iconified)
		{
			Event event;
			event.type = Event::Type::WindowIconify;
			event.windowIconify.iconified = iconified;
			GLFWHandler::GetInstance().QueueEvent(event);
		});
	
	glfwSetWindowMaximizeCallback(windowHandle,
		[](GLFWwindow* window, int maximized)
		{
			Event event;
			event.type = Event::Type::WindowMaximize;
			event.windowMaximize.maximized = maximized;
			GLFWHandler::GetInstance().QueueEvent(event);
		});

	glfwSetKeyCallback(windowHandle,
		[](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			Event event;
			event.type = Event::Type::Keyboard;
			event.keyboard.key = key;
			event.keyboard.scancode = scancode;
			event.keyboard.action = action;
			event.keyboard.mods = mods;
			GLFWHandler::GetInstance().QueueEvent(event);
		});

	glfwSetCharCallback(windowHandle,
		[](GLFWwindow* window, unsigned int codepoint)
		{
			Event event;
			event.type = Event::Type::Char;
			event.character.codepoint = codepoint;
			GLFWHandler::GetInstance().QueueEvent(event);
		});
	
	glfwSetMouseButtonCallback(windowHandle,
		[](GLFWwindow* window, int button, int action, int mods)
		{
			Event event;
			event.type = Event::Type::MouseButton;
			event.mouseButton.button = button;
			event.mouseButton.action = action;
			event.mouseButton.mods = mods;
			GLFWHandler::GetInstance().QueueEvent(event);
		});
	
	glfwSetCursorPosCallback(windowHandle,
		[](GLFWwindow* window, double xpos, double ypos)
		{
			Event event;
			event.type = Event::Type::MouseMove;
			event.mouseMove.x = xpos;
			event.mouseMove.y = ypos;
			GLFWHandler::GetInstance().QueueEvent(event);
		});
	
	glfwSetScrollCallback(windowHandle,
		[](GLFWwindow* window, double xoffset, double yoffset)
		{
			Event event;
			event.type = Event::Type::MouseScroll;
			event.mouseScroll.x = xoffset;
			event.mouseScroll.y = yoffset;
			GLFWHandler::GetInstance().QueueEvent(event);
		});
}