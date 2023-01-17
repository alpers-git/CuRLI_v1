#include <stdio.h>
#include <glm/glm.hpp>
#include <GL/freeglut.h>

int main(int argc, char const *argv[])
{
    glutInit(&argc, (char**)argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("CuRLI");
    glutDisplayFunc([]() {
        glClearColor(37.f/255.f, 77.f/255.f, 141.f/255.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glutSwapBuffers();
    });
    glutIdleFunc([]() {
        glutPostRedisplay();
    });
    glutMainLoop();
    return 0;
}
