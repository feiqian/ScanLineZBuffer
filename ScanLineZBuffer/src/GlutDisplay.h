#ifndef __GLUT_DISPLAY_H__
#define __GLUT_DISPLAY_H__

#include "Glut.h"
#include "ScanLineZBuffer.h"

class ScanLineZBuffer;

class GlutDisplay
{
public:
	static void render(ScanLineZBuffer* engine);
private:
	static void loop();
	static void reshape(int w, int h);

	static void motionCallBack(int x, int y );
	static void mouseCallBack(int button, int state, int x, int y);

	static ScanLineZBuffer* engine;
	static bool bMouseDown;
	static int mouseX;
	static int mouseY;
};

#endif

