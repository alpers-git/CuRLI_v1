#include <stdio.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>

#define GLUT_KEY_ESCAPE 27

namespace glutenFree
{
	void DisplayFunc()
	{
		glClear(GL_COLOR_BUFFER_BIT);
		glutSwapBuffers();
	};
	void IdleFunc()
	{
		glutPostRedisplay();
	};
	//void glutReshapeFunc(void (*func)(int width, int height));
	//void glutKeyboardFunc(void (*func)(unsigned char key, int x, int y));
	void KeyboardFunc(unsigned char key, int x, int y)
	{
		//get key turn it into ascii char and print it
		printf("Key %c was pressed at (%d, %d)\n", key, x, y);
		switch (key)
		{
		case GLUT_KEY_UP:
			break;
		case GLUT_KEY_DOWN:
			break;
		case GLUT_KEY_LEFT:
			break;
		case GLUT_KEY_RIGHT:
			break;
		case GLUT_KEY_ESCAPE:
			glutLeaveMainLoop();
			break;
		}
	}
	void SpecialFunc(int key, int x, int y)
	{
		switch (key)
		{
		case GLUT_KEY_UP:
			break;
		case GLUT_KEY_DOWN:
			break;
		case GLUT_KEY_LEFT:
			break;
		case GLUT_KEY_RIGHT:
			break;
		case GLUT_KEY_ESCAPE:
			printf("Escape key was pressed at (%d, %d)\n", x, y);
			glutLeaveMainLoop();
			break;
		}
	};

	class GlutenFree
	{
	public:
		static GlutenFree& GetInstance()
		{
			static GlutenFree instance;
			return instance;
		}
		void InitAndCreateWindow(int argc, char** argv, bool easySetup=false)
		{
			glutInit(&argc, argv);
			glClearColor(0.09f, 0.30f, 0.55f, 1.0f);
			
			glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
			glutInitWindowSize(800, 600);
			glutInitWindowPosition(100, 100);
			glutCreateWindow("CuRLI");

			if (easySetup)
			{
				SetDisplayFunc(DisplayFunc);
				SetIdleFunc(IdleFunc);
				SetKeyboardUpFunc(KeyboardFunc);
				//SetSpecialFunc(SpecialFunc);
			}
		}
		void Run()
		{
			glutMainLoop();
		}
		void SetDisplayFunc(void (*func)(void))
		{
			glutDisplayFunc(func);
		}
		void SetIdleFunc(void (*func)(void))
		{
			glutIdleFunc(func);
		}
		void SetReshapeFunc(void (*func)(int width, int height))
		{
			glutReshapeFunc(func);
		}
		void SetKeyboardFunc(void (*func)(unsigned char key, int x, int y))
		{
			glutKeyboardFunc(func);
		}
		void SetKeyboardUpFunc(void (*func)(unsigned char key, int x, int y))
		{
			glutKeyboardUpFunc(func);
		}
		void SetSpecialFunc(void (*func)(int key, int x, int y))
		{
			glutSpecialFunc(func);
		}
	private:
		GlutenFree() {}
		GlutenFree(const GlutenFree&) = delete;
		GlutenFree& operator=(const GlutenFree&) = delete;
	};
}

