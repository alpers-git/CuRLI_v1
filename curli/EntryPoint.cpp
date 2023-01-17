#include <stdio.h>
#include <glm/glm.hpp>
#include <GlutHandlers.h>

int main(int argc, char const *argv[])
{
    glutenFree::GlutenFree& glutHdlrIns = glutenFree::GlutenFree::GetInstance();
	glutHdlrIns.InitAndCreateWindow(argc, (char**)argv, true);
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
    return 0;
}
