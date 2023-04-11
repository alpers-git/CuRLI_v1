#pragma once
#include <stdio.h>
#include <GLFW/glfw3.h>
#include <queue>
#include <entt/entt.hpp>

struct Event
{
	enum class Type
	{
		WindowClose,
		WindowResize,
		WindowMove,
		WindowFocus,
		WindowIconify,
		WindowMaximize,
		Keyboard,
		Char,
		MouseButton,
		MouseMove,
		MouseScroll,
		Drop,
		GeometryChange,
		TextureChange,
		SoftbodySim
	};
	Type type;
	union
	{
		struct
		{
			int width, height;
		} windowResize;
		struct
		{
			int x, y;
		} windowMove;
		struct
		{
			int focused;
		} windowFocus;
		struct
		{
			int iconified;
		} windowIconify;
		struct
		{
			int maximized;
		} windowMaximize;
		struct
		{
			int key, scancode, action, mods;
		} keyboard;
		struct
		{
			unsigned int codepoint;
		} character;
		struct
		{
			int button, action, mods;
		} mouseButton;
		struct
		{
			double x, y;
		} mouseMove;
		struct
		{
			double x, y;
		} mouseScroll;
		struct
		{
			int count;
			const char** paths;
		} drop;
		
		struct
		{
			entt::entity e;//todo
			bool toBeRemoved;
		} geometryChange;
		struct
		{
			entt::entity e;//todo
			bool toBeRemoved;
			//ImageMap::BindingSlot slot;
		} textureChange;

		struct
		{
			entt::entity e;
		}softbodySim;
	};
};

class GLFWHandler
{
public:
	static GLFWHandler& GetInstance()
	{
		static GLFWHandler instance;
		return instance;
	}
	GLFWHandler(GLFWHandler const&) = delete;
	void operator=(GLFWHandler const&) = delete;
	void InitAndCreateWindow(int width=800, int height=600, const char* title="CuRLI");
	void SwapBuffers();
	void Close();
	bool IsRunning();
	void SetWindowSize(int width, int height);
	void SetWindowPosition(int x, int y);
	void SetWindowName(const char* name);
	void SetWindowIcon(const char* path);
	void SetWindowIcon(const char* path, int width, int height);
	
	template<typename R, typename P>
	void DispatchEvents(R& renderer, P& pIntegrator)
	{
		glfwPollEvents();
		
		auto& io = ImGui::GetIO();
		if (io.WantCaptureMouse ) {
			//go through the queue and remove all mouse events
			std::queue<Event> tempQueue;
			while (!eventQueue.empty())
			{
				if (eventQueue.front().type != Event::Type::MouseButton && eventQueue.front().type != Event::Type::MouseMove && eventQueue.front().type != Event::Type::MouseScroll)
				{
					tempQueue.push(eventQueue.front());
				}
				eventQueue.pop();
			}
			eventQueue = tempQueue;
			//return;
		}
		if (io.WantCaptureKeyboard) {
			//go through the queue and remove all keyboard events
			std::queue<Event> tempQueue;
			while (!eventQueue.empty())
			{
				if (eventQueue.front().type != Event::Type::Keyboard && eventQueue.front().type != Event::Type::Char)
				{
					tempQueue.push(eventQueue.front());
				}
				eventQueue.pop();
			}
			eventQueue = tempQueue;
			//return;
		}

		//while loop over events
		while (!eventQueue.empty())
		{
			switch (eventQueue.front().type)
			{
			case Event::Type::WindowClose:
				Close();
				break;
			case Event::Type::WindowResize:
				renderer.OnWindowResize(eventQueue.front().windowResize.width, eventQueue.front().windowResize.height);
				break;
			case Event::Type::WindowMove:
				renderer.OnWindowMove(eventQueue.front().windowMove.x, eventQueue.front().windowMove.y);
				break;
			case Event::Type::WindowFocus:
				renderer.OnWindowFocus(eventQueue.front().windowFocus.focused);
				break;
			case Event::Type::WindowIconify:
				renderer.OnWindowIconify(eventQueue.front().windowIconify.iconified);
				break;
			case Event::Type::WindowMaximize:
				renderer.OnWindowMaximize(eventQueue.front().windowMaximize.maximized);
				break;
			case Event::Type::Keyboard:
				renderer.OnKeyboard(eventQueue.front().keyboard.key, eventQueue.front().keyboard.scancode, eventQueue.front().keyboard.action, eventQueue.front().keyboard.mods);
				pIntegrator.OnKeyboard(eventQueue.front().keyboard.key, eventQueue.front().keyboard.scancode, eventQueue.front().keyboard.action, eventQueue.front().keyboard.mods);
				break;
			/*case Event::Type::Char:
				renderer.OnChar(eventQueue.front().character.codepoint);
				break;*/
			case Event::Type::MouseButton:
				renderer.OnMouseButton(eventQueue.front().mouseButton.button, eventQueue.front().mouseButton.action, eventQueue.front().mouseButton.mods);
				pIntegrator.OnMouseButton(eventQueue.front().mouseButton.button, eventQueue.front().mouseButton.action, eventQueue.front().mouseButton.mods);
				break;
			case Event::Type::MouseMove:
				renderer.OnMouseMove(eventQueue.front().mouseMove.x, eventQueue.front().mouseMove.y);
				pIntegrator.OnMouseMove(eventQueue.front().mouseMove.x, eventQueue.front().mouseMove.y);
				break;
			case Event::Type::MouseScroll:
				renderer.OnMouseScroll(eventQueue.front().mouseScroll.x, eventQueue.front().mouseScroll.y);
				break;
			case Event::Type::GeometryChange:
				renderer.OnGeometryChange(eventQueue.front().geometryChange.e, eventQueue.front().geometryChange.toBeRemoved);
				break;
			case Event::Type::TextureChange:
				renderer.OnTextureChange(eventQueue.front().textureChange.e, eventQueue.front().textureChange.toBeRemoved);
				break;
			case Event::Type::SoftbodySim:
				renderer.OnSoftbodyChange(eventQueue.front().softbodySim.e);
				break;
			/*case Event::Type::Drop:
				renderer.OnDrop(eventQueue.front().drop.count, eventQueue.front().drop.paths);
				break;*/
			}
			eventQueue.pop();
		}
	}

	
	inline GLFWwindow* GetWindowPointer() { return windowHandle; }
	inline void QueueEvent(Event event) 
	{
		eventQueue.push(event); 
	}
	static inline float GetTime() { return glfwGetTime(); }
	
private:
	GLFWHandler();
	~GLFWHandler();
	GLFWwindow* windowHandle=NULL;
	std::queue<Event> eventQueue;
	
	void setCallbacks();
};


