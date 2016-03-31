#include "GlutDisplay.h"

bool GlutDisplay::bMouseDown = false;
int GlutDisplay::mouseX=-1;
int GlutDisplay::mouseY=-1;
ScanLineZBuffer* GlutDisplay::engine= NULL;

void GlutDisplay::reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	engine->setSize(w,h);
}

void GlutDisplay::mouseCallBack(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON)
	{
		if(state == GLUT_DOWN)
		{
			bMouseDown = true;
			mouseX=x;
			mouseY=y;
			return;
		}
	}
	bMouseDown = false;
}

void GlutDisplay::motionCallBack(int x, int y )
{
	if (bMouseDown)
	{
		mouseX = x;
		mouseY = y;
		glutPostRedisplay();
	}
}

void GlutDisplay::render(ScanLineZBuffer* engine)
{
	int width=0,height=0;
	engine->getSize(width,height);
	GlutDisplay::engine = engine;

	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(width,height);
	glutCreateWindow("ScanLineZBuffer");
	glutDisplayFunc(GlutDisplay::loop);
	glutReshapeFunc(reshape);
	glutMouseFunc(NULL);
	glutMotionFunc(NULL);
	glutMainLoop();
}

void GlutDisplay::loop()
{
	int width=0,height=0;
	engine->getSize(width,height);

	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0,width,0,height);

	GLvoid * colorBuffer = engine->render();	
	glDrawPixels(width,height,GL_RGB,GL_UNSIGNED_BYTE,colorBuffer);

	glFinish();
}