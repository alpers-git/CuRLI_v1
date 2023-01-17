#include <stdio.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>

int main(int argc, char const *argv[])
{
    glutInit(&argc, (char**)argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("CuRLI");
    glutDisplayFunc([]() {
        glClearColor(0.1f, 0.2f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glutSwapBuffers();
    });
    glutIdleFunc([]() {
        glutPostRedisplay();
    });

    glutMainLoop();
    return 0;
}
