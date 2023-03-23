#pragma once
#include <stdio.h>
#include <GLFWHandler.h>


//singleton class of ApplicationState
class ApplicationState
{
public:
	static ApplicationState& GetInstance()
	{
		static ApplicationState instance;
		return instance;
	}
	entt::entity selectedObject;
	bool physicsInteraction = false;
	bool renderingWireframe = false;

	ApplicationState(const ApplicationState&) = delete;
	void operator=(GLFWHandler const&) = delete;
private:
	ApplicationState() {}
	~ApplicationState() {}
};