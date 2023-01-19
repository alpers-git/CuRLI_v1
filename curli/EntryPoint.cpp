#include <glm/glm.hpp>
#include <GlutenFree.h>
#include <imgui.h>
#include <backends/imgui_impl_glut.h>
#include <backends/imgui_impl_opengl3.h>
#include <ImguiHelpers.h>
#include <stdio.h>

int main(int argc, char const *argv[])
{
	//Get Glut handler
    glutenFree::GlutenFree& glutHdlrIns = glutenFree::GlutenFree::GetInstance();
	//Create Window
	glutHdlrIns.InitAndCreateWindow(argc, (char**)argv, true);
	
	//===Init&Create Imgui context===
	gui::InitImgui();
	
	
	glutHdlrIns.SetDisplayFunc([] {
		//===Get new frame for Imgui===
		gui::GetNewImguiFrame();
		
		//===Render Imgui components===
		//ImGui::ShowDemoWindow();
		gui::RenderImgui();
		glClear(GL_COLOR_BUFFER_BIT);

		//glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound, but prefer using the GL3+ code.
		//===OpenGl call to draw===
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glutSwapBuffers();
	});

	
	glutHdlrIns.SetIdleFunc([] {
		const glm::vec3 clearColor1(0.09f, 0.30f, 0.55f);
		const glm::vec3 clearColor2(1.0f, 0.76f, 0.03f);
		float time = glutGet(GLUT_ELAPSED_TIME) * 0.001f;
		//put time into a sin wave to get a value between 0 and 1
		float value = (sin(time) + 1.0f) / 2.0f;
		//lerp between the two colors
		auto color = glm::mix(
			clearColor1,
			clearColor2,
			value);
		printf("color %f %f %f\n", color.x, color.y, color.z);
		glClearColor(color.r, color.g, color.b, 1.0f);
		glutPostRedisplay();
		});
	glutHdlrIns.Run();
	
	//===Cleanup Imgui===
	gui::TerminateImgui();
	
    return 0;
}
