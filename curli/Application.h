#pragma once
#include "Renderer.h"
#include <queue>


struct Event
{
	//todo
};

//Implement a singleton EventHandler class
class EventHandler
{
public:
	static EventHandler& GetInstance()
	{
		static EventHandler instance;
		return instance;
	}
	EventHandler(EventHandler const&) = delete;
	void operator=(EventHandler const&) = delete;
private:
	std::queue<Event> eventQueue;
	EventHandler();
	~EventHandler();
};


class Application
{
public:
	Application(int argc, char const* argv[]);
	~Application();

	void Run();
private:
	TeapotRenderer renderer;
};