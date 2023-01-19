#include <GL/freeglut.h>
#include <imgui.h>
#include <backends/imgui_impl_glut.h>

namespace gui
{
	void InitImgui()
	{
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		ImGui_ImplGLUT_Init();
		ImGui_ImplGLUT_InstallFuncs();
		ImGui_ImplOpenGL3_Init();
	}

	void GetNewImguiFrame()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGLUT_NewFrame();
	}

	void RenderImgui()
	{
		ImGui::Render();
		ImGuiIO& io = ImGui::GetIO();
		glViewport(0, 0, (GLsizei)io.DisplaySize.x, (GLsizei)io.DisplaySize.y);
	}

	void TerminateImgui()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGLUT_Shutdown();
		ImGui::DestroyContext();
	}
}