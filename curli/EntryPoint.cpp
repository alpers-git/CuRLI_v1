#include <glm/glm.hpp>
#include <GLFWHandler.h>
#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>
#include <ImguiHelpers.h>
#include <stdio.h>

int main(int argc, char const *argv[])
{
	//Create a glfw window and render blue background
	auto& windowManager = GLFWHandler::GetInstance();
	windowManager.InitAndCreateWindow(800, 600, "CuRLI");
	
	windowManager.SetKeyboardCallback([](GLFWwindow* window, int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
		});
	
	//Init imgui
	gui::InitImgui(windowManager.GetWindowPointer());
	
	//Create a rendering loop with glfw
	while (!glfwWindowShouldClose(windowManager.GetWindowPointer()))
	{
		windowManager.Update();
		
		//Start the Dear ImGui frame
		gui::GetNewImguiFrame();
		ImGui::ShowDemoWindow();
		gui::RenderImgui();
		
		int display_w, display_h;
		const glm::vec3 clearColor1(0.09f, 0.30f, 0.55f);
		const glm::vec3 clearColor2(1.0f, 0.76f, 0.03f);
		float time = glfwGetTime() * 10.0f;
		//put time into a sin wave to get a value between 0 and 1
		float value = (sin(time) + 1.0f) / 2.0f;
		//lerp between the two colors
		auto color = glm::mix(
			clearColor1,
			clearColor2,
			value);
		printf("color %f %f %f\n", color.x, color.y, color.z);
		glClearColor(color.r, color.g, color.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
	
	gui::TerminateImgui();
	windowManager.Close();

    return 0;
}
