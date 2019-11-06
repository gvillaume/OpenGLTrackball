#include <iostream>
#include <fstream>
using namespace std;

#include <stdlib.h>
#include <math.h>

#include <sys/types.h>
#include <GL/glut.h>
#include <GL/freeglut.h>


#include "objLoader.h"


#define KEY_LEFT 100
#define KEY_UP 101
#define KEY_RIGHT 102
#define KEY_DOWN 103

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

int winWidth = 1024;
int winHeight = 1024;
bool firstTime = true;

WavefrontObj* obj_data;

// Trackball parameters initialization 
float angle = 0.0, axis[3], trans[3];

bool trackingMouse = false;
bool redrawContinue = false;
bool trackballMove = false;

float lastPos[3] = { 0.0, 0.0, 0.0 };
int curx, cury;
int startX, startY;


// Translation & Rotation
float x_trans = 0.0f; // translate object in x direction
float y_trans = 0.0f; // translate object in y direction
float x_fullTrans = 0.0f;	// total translation in x direction
float y_fullTrans = 0.0f;	// total translation in y direction
float zoom = 1.0f; // zoom for scaling



void Init(int w, int h)
{
	glViewport(0, 0, w, h);
	glShadeModel(GL_SMOOTH);								// Set Smooth Shading 
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);			    	// Background Color 
	glClearDepth(1.0f);										// Depth buffer setup 
	glEnable(GL_DEPTH_TEST);								// Enables Depth Testing 
	glDepthFunc(GL_LEQUAL);									// The Type Of Depth Test To Do 
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);		// Use perspective correct interpolation if available

	glMatrixMode(GL_PROJECTION);							// Select The Projection Matrix
	glLoadIdentity();										// Reset The Projection Matrix
	double aspect = (double)h / w;
	glFrustum(-5, 5, -5 * aspect, 5 * aspect, 10, 500);          // Define perspective projection frustum
	//gluPerspective(30, w/h, 10, 74);
	glTranslated(0.0, 0.0, -24);                          // Viewing transformation

	glMatrixMode(GL_MODELVIEW);								// Select The Modelview Matrix
	glLoadIdentity();										// Reset The Modelview Matrix

	if (firstTime)
	{
		glEnable(GL_LIGHTING);
		float frontColor[] = { 0.2f, 0.7f, 0.7f, 1.0f };

		glMaterialfv(GL_FRONT, GL_AMBIENT, frontColor);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, frontColor);
		glMaterialfv(GL_FRONT, GL_SPECULAR, frontColor);
		glMaterialf(GL_FRONT, GL_SHININESS, 100);

		float lightDirection[] = { 2.0f, 0.0f, 1.0f, 0 };
		float ambientIntensity[] = { 0.1f, 0.1f, 0.1f, 1.0f };
		float lightIntensity[] = { 0.9f, 0.9f, 0.9f, 1.0f };
		float lightSpec[] = { 1.0f, 1.0f, 1.0f, 1 };
		glLightfv(GL_LIGHT0, GL_AMBIENT, ambientIntensity);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, lightIntensity);
		glLightfv(GL_LIGHT0, GL_POSITION, lightDirection);
		glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpec);
		glEnable(GL_LIGHT0);
		firstTime = false;
	}
}


void Draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear The Screen And The Depth Buffer

	GLfloat objectform[16];							//Array to save matrix
	glGetFloatv(GL_MODELVIEW_MATRIX, objectform);	//Save matrix for future use
	glLoadIdentity();								//Load identity matrix to apply current transformations
	glTranslatef(x_fullTrans + x_trans, y_fullTrans + y_trans, 0.0f);	//Move object back to original position, plus current translation
	glScalef(zoom, zoom, zoom);						//Scale object
	glRotatef(angle, axis[0], axis[1], axis[2]);	//Rotate object by rotation angle around rotation axis
	glTranslatef(-x_fullTrans, -y_fullTrans, 0.0f);	//Translate object back to origin
	glMultMatrixf(objectform);						//Load matrix to preserve previous transformations


	if (obj_data != NULL)
		obj_data->Draw();
	else
		glutSolidTeapot(1.0);	//draw a teapot when no argument is provided

	glFlush();
	glutSwapBuffers();

	x_fullTrans += x_trans;		//Add current translation to total translation
	y_fullTrans += y_trans;		
	x_trans = 0.0f;				//Reset current translation
	y_trans = 0.0f;
	zoom = 1.0f;				//Reset scaling
	angle = 0.0f;				//Reset rotation angle
}


//
// GLUT keypress function
// 
void Specialkey(int key, int x, int y)
{
	// Set translation using the arrow keys
	if (key == KEY_UP) {
		y_trans += 0.5f;
	}
	else if (key == KEY_DOWN) {
		y_trans -= 0.5f;
	}
	else if (key == KEY_RIGHT) {
		x_trans += 0.5f;
	}
	else if (key == KEY_LEFT) {
		x_trans -= 0.5f;
	}

	glutPostRedisplay();
}


void onKeyPress(unsigned char key, int x, int y)
{
	if (key == 'p')
	{
		obj_data->mode = GL_LINE_LOOP;
	}
	else if (key == 's')
	{
		glShadeModel(GL_SMOOTH);								// Set Smooth Shading 
		obj_data->mode = GL_POLYGON;
	}
	else if (key == 'f')
	{
		glShadeModel(GL_FLAT);								// Set Smooth Shading 
		obj_data->mode = GL_POLYGON;
	}
	else if (key == 'q')
	{
		delete obj_data;
		exit(0);
	}

	glutPostRedisplay();
}

void MouseWheel(int wheel, int direction, int x, int y)
{
	// Set zoom for the redraw based on mouse wheel direction
	if (direction > 0)
		zoom += 0.1f;
	else
		zoom -= 0.1f;

	glutPostRedisplay();

}



void mouseMotion(int x, int y)
{

	if (trackingMouse) //If the left button has been clicked
	{
		float curPos[3], dx, dy, dz;
		float d, norm;
		curPos[0] = (2.0f * x - winWidth) / winWidth; //Calculate x component for vector at mouse's current position
		curPos[1] = (winHeight - 2.0f * y) / winHeight; //Calculate y component for vector at mouse's current position
		d = sqrtf(curPos[0] * curPos[0] + curPos[1] * curPos[1]); //Calculate z component
		d = (d < 1.0f) ? d : 1.0f; //Project vector onto surface of trackball
		curPos[2] = sqrtf(1.001f - d * d); //Calculate z component
		norm = 1.0 / sqrt(curPos[0] * curPos[0] + curPos[1] * curPos[1] + curPos[2] * curPos[2]);
		curPos[0] *= norm; //Normalize vecor
		curPos[1] *= norm;
		curPos[2] *= norm;
		dx = curPos[0] - lastPos[0]; //Check if mouse has moved from last position
		dy = curPos[1] - lastPos[1];
		dz = curPos[2] - lastPos[2];
		if (dx || dy || dz) //If mouse has moved
		{
			angle = 90.0 * sqrt(dx * dx + dy * dy + dz * dz); //Calculate rotation angle
			axis[0] = lastPos[1] * curPos[2] - lastPos[2] * curPos[1]; //Calculate rotation axis
			axis[1] = lastPos[2] * curPos[0] - lastPos[0] * curPos[2];
			axis[2] = lastPos[0] * curPos[1] - lastPos[1] * curPos[0];
			lastPos[0] = curPos[0]; //Set the last position to the current
			lastPos[1] = curPos[1];
			lastPos[2] = curPos[2];
		}
	}
	
	glutPostRedisplay();
}




void mouseButton(int button, int state, int x, int y)
{
	//Detect mouse click
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		trackingMouse = true; //Set boolean so that future mouse movements are tracked

		float d, norm;
		lastPos[0] = (2.0f * x - winWidth) / winWidth; //Calculate the x and y components of the initial movement vector
		lastPos[1] = (winHeight - 2.0f * y) / winHeight;
		d = sqrtf(lastPos[0] * lastPos[0] + lastPos[1] * lastPos[1]); //Calculate the z component of the vector using pythagorean theorem
		d = (d < 1.0f) ? d : 1.0f; //If the z component is not on the surface of the trackball, set it to 1: the radius of the trackball
		lastPos[2] = sqrtf(1.001f - d * d);
		norm = 1.0 / sqrt(lastPos[0] * lastPos[0] + lastPos[1] * lastPos[1] + lastPos[2] * lastPos[2]); //Calculate vector length
		lastPos[0] *= norm; //Normalize vector
		lastPos[1] *= norm;
		lastPos[2] *= norm;
	}
	else
		trackingMouse = false; //If button not clicked, set boolean so that no mouse movement is tracked
}





int main(int argc, char* argv[])
{
	// glut initialization functions:
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(winWidth, winHeight);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("ImageViewer");

	Init(winWidth, winHeight);

	// display, onMouseButton, mouse_motion, onKeyPress, and resize are functions defined above
	glutDisplayFunc(Draw);
	glutKeyboardFunc(onKeyPress);
	glutSpecialFunc(Specialkey);
	glutMouseFunc(mouseButton);
	glutMotionFunc(mouseMotion);
	glutMouseWheelFunc(MouseWheel);
	glutReshapeFunc(Init);

	if (argc >= 2)
		obj_data = new WavefrontObj(argv[1]);
	else
		obj_data = NULL;

	// start glutMainLoop -- infinite loop
	glutMainLoop();

	return 0;
}
