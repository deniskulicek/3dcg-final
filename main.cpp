#if defined(_WIN32)
	#include <windows.h>
#endif
#include <GL/freeglut.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "lib/keys.h"
#include "lib/DeltaTimer.h"
#include "lib/generate_grid.h"
#include "lib/generate_mountain.h"
#include <iostream>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <vector>

using std::cout;
using std::endl;
using std::vector;
using glm::vec3;
using glm::ivec2;
using glm::ivec3;
using glm::mat4;

vector<vec3> lights;
DeltaTimer deltaTimer;

void initializeLights() {
	lights.resize(2);
	lights[0] = vec3(1, 2, 3);
	lights[1] = vec3(3, 4, 5);
}

void drawLight(const mat4 & transform, const vec3 & light) {
	mat4 local = glm::translate(transform, light);
	glLoadMatrixf(glm::value_ptr(local));
	glPushAttrib(GL_LIGHTING);
	glDisable(GL_LIGHTING);
	glColor3f(1,1,0);
	glutSolidSphere(0.2, 8, 8);
	glPopAttrib();
	glLoadMatrixf(glm::value_ptr(transform));
}

void drawAxes(float length = 1) {
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDisable(GL_LIGHTING);
	glutSolidSphere(length/10.0f, 8, 8);
	glBegin(GL_LINES);
		glColor3f(1, 0, 0);
		glVertex3f(0, 0, 0);
		glVertex3f(length, 0, 0);

		glColor3f(0, 1, 0);
		glVertex3f(0, 0, 0);
		glVertex3f(0, length, 0);

		glColor3f(0, 0, 1);
		glVertex3f(0, 0, 0);
		glVertex3f(0, 0, length);
	glEnd();
	glPopAttrib();
}

ivec2 surfaceSize(40, 40);
vec3 surfaceScale(8.0f, 8.0f, 1.0f);
vec3 surfacePosition(0.0f);
vector<float> surfaceHeights;
vector<vec3> surfaceVertices;
vector<ivec3> surfaceTriangles;

void initializeSurface() {
	surfaceVertices.resize((surfaceSize.x + 1)*(surfaceSize.y + 1));
	surfaceTriangles = generate_grid(surfaceSize.x, surfaceSize.y);

	vector<MountainGenerationLevel> levels(3);
	levels[0].granularity = 0.1f; levels[0].min =  0.00f; levels[0].max = 1.00f;
	levels[1].granularity = 0.5f; levels[1].min = -0.10f; levels[1].max = 0.10f;
	levels[2].granularity = 1.0f; levels[2].min = -0.04f; levels[2].max = 0.04f;
	surfaceHeights = generate_mountain(surfaceSize.x + 1, surfaceSize.y + 1, levels);
	cout << surfaceHeights.size() << endl;

	for (int ix = 0; ix <= surfaceSize.x; ix++) {
		for (int iy = 0; iy <= surfaceSize.y; iy++) {
			int iv = iy + ix*(surfaceSize.y + 1);
			float rx = (float) ix/surfaceSize.x - 0.5f;
			float ry = (float) iy/surfaceSize.y - 0.5f;
			float z = surfaceHeights[iv];
			surfaceVertices[iv] = vec3(rx, ry, z);
		}
	}

}

void drawSurface(const mat4 & transform) {
	mat4 local = glm::translate(transform, surfacePosition);
	local = glm::scale(local, surfaceScale);
	glLoadMatrixf(glm::value_ptr(local));

	glBegin(GL_TRIANGLES);
	glColor3f(1, 0, 0);
	for (vector<float>::size_type it = 0; it < surfaceTriangles.size(); it++) {
		ivec3 vis = surfaceTriangles[it];
		glVertex3fv(glm::value_ptr(surfaceVertices[vis[0]]));
		glVertex3fv(glm::value_ptr(surfaceVertices[vis[1]]));
		glVertex3fv(glm::value_ptr(surfaceVertices[vis[2]]));
	}
	glEnd();
	glLoadMatrixf(glm::value_ptr(transform));
}

vec3 camera_position_velocity(0.0f, 0.0f, 0.0f);
vec3 camera_rotation_velocity(0.0f, 0.0f, 0.0f);
mat4 camera(1.0f);

void initializeCamera() {
	camera = glm::rotate(camera, glm::two_thirds<float>()*glm::pi<float>(), vec3(1.0f, 1.0f, 1.0f));
	camera = glm::inverse(camera);
	camera = glm::translate(camera, vec3(-4.0f, 0.0f, 0.0f));
	camera = glm::inverse(camera);
}

void render()
{
	glClearColor (0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT  | GL_DEPTH_BUFFER_BIT);

	mat4 transform = glm::inverse(camera);
	glLoadMatrixf(glm::value_ptr(transform));

	drawAxes();

	drawSurface(transform);

	// glutSolidTeapot(3);



	// glLoadMatrixf(transform);

	// for (vector<vec3>::size_type il = 0; il < lights.size(); il++) {
	// 	drawLight(transform, lights[il]);
	// }
	// glLightfv(GL_LIGHT0, GL_SPECULAR, whiteSpecularLight);
 //    glLightfv(GL_LIGHT0, GL_AMBIENT, blackAmbientLight);
 //    glLightfv(GL_LIGHT0, GL_DIFFUSE, whiteDiffuseLight);
 //    glLightfv(GL_LIGHT0, GL_POSITION, glm::value_ptr(LightPos));

	// glPushMatrix();
	// 	glTranslatef(0, 0, -4);
	// 	glRotatef(cubeRotation, 1, .8, .6);
	// 	glutSolidTeapot(1);
	// glPopMatrix();

	// drawCoordSystem();

	glutSwapBuffers();
	glutPostRedisplay();
}

void animate() {
	float delta = deltaTimer.update();

	vec3 lmov;
	if (keys::held((unsigned char) 'd')) lmov.x = 1.0f;
	if (keys::held((unsigned char) 'a')) lmov.x = -1.0f;
	if (keys::held((unsigned char) 'w')) lmov.y = 1.0f;
	if (keys::held((unsigned char) 's')) lmov.y = -1.0f;
	if (keys::held((unsigned char) 'q')) lmov.z = 1.0f;
	if (keys::held((unsigned char) 'z')) lmov.z = -1.0f;
	lights[0] += 2*delta*lmov;
	lights[1] += 3*delta*lmov;

	vec3 crot(0.0f, 0.0f, 0.0f);
	if (keys::held((unsigned char) 'l')) crot.z = -delta;
	if (keys::held((unsigned char) 'j')) crot.z = delta;
	if (keys::held((unsigned char) 'i')) crot.x = delta;
	if (keys::held((unsigned char) 'k')) crot.x = -delta;
	if (keys::held((unsigned char) 'u')) crot.y = delta;
	if (keys::held((unsigned char) 'o')) crot.y = -delta;
	camera_rotation_velocity += crot;
	camera_rotation_velocity *= 0.9f;
	crot += 0.2f*camera_rotation_velocity;

	vec3 cmov(0.0f, 0.0f, 0.0f);
	if (keys::held((unsigned char) 'd')) cmov.x = delta;
	if (keys::held((unsigned char) 'a')) cmov.x = -delta;
	if (keys::held((unsigned char) 'w')) cmov.z = -delta;
	if (keys::held((unsigned char) 's')) cmov.z = delta;
	if (keys::held((unsigned char) 'q')) cmov.y = delta;
	if (keys::held((unsigned char) 'z')) cmov.y = -delta;
	camera_position_velocity += cmov;
	camera_position_velocity *= 0.9f;
	cmov += 5.0f*camera_position_velocity;

	mat4 mcam(1.0f);
	mcam = glm::translate(mcam, cmov);
	mcam = glm::rotate(mcam, crot.x, vec3(1.0f, 0.0f, 0.0f));
	mcam = glm::rotate(mcam, crot.y, vec3(0.0f, 1.0f, 0.0f));
	mcam = glm::rotate(mcam, crot.z, vec3(0.0f, 0.0f, 1.0f));

	camera = camera*mcam;
}

//take keyboard input into account
void onKeyDown(unsigned char key, int x, int y) {
	keys::onDown(key);

	const char ESC = 27;

	if (key == ESC) exit(0);
}

void onKeyUp(unsigned char key, int x, int y) {
	keys::onUp(key);
}

void onSpecialKeyDown(int key, int x, int y) {
	keys::onSpecialDown(key);

}

void onSpecialKeyUp(int key, int x, int y) {
	keys::onSpecialUp(key);
}

int last_mouse_x;
int last_mouse_y;
float last_mouse_dx;
float last_mouse_dy;

void onMouse(int button, int state, int x, int y) {
	// cout << "Mouse press " << button << " state " << state << " x: " << x << " y: " << y << endl;

	last_mouse_x = x;
	last_mouse_y = y;

	if (state == 1) {
		vec3 cmov(-0.007f*last_mouse_dx, 0.007f*last_mouse_dy, 0.0f);
		camera_position_velocity = cmov;
	}
}


void onMotion(int x, int y) {
	// cout << "Mouse motion x: " << x << " y: " << y << endl;

	last_mouse_dx = x - last_mouse_x;
	last_mouse_dy = y - last_mouse_y;
	last_mouse_x = x;
	last_mouse_y = y;

	vec3 cmov(-0.007f*last_mouse_dx, 0.007f*last_mouse_dy, 0.0f);

	mat4 mcam(1.0f);
	mcam = glm::translate(mcam, cmov);

	camera = camera*mcam;
}

void reshape(int w, int h) {
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//glOrtho (-1.1, 1.1, -1.1,1.1, -1000.0, 1000.0);
	gluPerspective(50, (float)w/h, 1, 100);
	glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
	// Initialize glut.
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(200, 100);
	glutInitWindowSize(800,600);
	glutCreateWindow(argv[0]);

	// Initialize opengl.
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_NORMALIZE);
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	glPolygonMode(GL_FRONT,GL_FILL);
	glPolygonMode(GL_BACK,GL_LINE);

	glShadeModel(GL_SMOOTH);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Initialize resources.
	initializeLights();
	initializeSurface();
	initializeCamera();

	// Configure glut.
	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(onKeyDown);
	glutKeyboardUpFunc(onKeyUp);
	glutSpecialFunc(onSpecialKeyDown);
	glutSpecialUpFunc(onSpecialKeyUp);
	glutMouseFunc(onMouse);
	glutMotionFunc(onMotion);
	glutDisplayFunc(render);
	glutIdleFunc(animate);

	// Kickstart the main loop.
	glutMainLoop();

	return 0;
}