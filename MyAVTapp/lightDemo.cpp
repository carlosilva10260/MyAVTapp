//
// AVT: Phong Shading and Text rendered with FreeType library
// The text rendering was based on https://learnopengl.com/In-Practice/Text-Rendering
// This demo was built for learning purposes only.
// Some code could be severely optimised, but I tried to
// keep as simple and clear as possible.
//
// The code comes with no warranties, use it at your own risk.
// You may use it, or parts of it, wherever you want.
// 
// Author: João Madeiras Pereira
//

#include <math.h>
#include <iostream>
#include <sstream>
#include <string>

// include GLEW to access OpenGL 3.3 functions
#include <GL/glew.h>


// GLUT is the toolkit to interface with the OS
#include <GL/freeglut.h>

#include <IL/il.h>


// Use Very Simple Libs
#include "VSShaderlib.h"
#include "VertexAttrDef.h"
#include "AVTmathLib.h"
#include "geometry.h"

#include "avtFreeType.h"

#include "camera.h"
#include "boat.h"
#include "waterCreatureManager.h"
#include "aabb.h"

using namespace std;

#define CAPTION "AVT Demo: Phong Shading and Text rendered with FreeType"
int WindowHandle = 0;
int WinX = 1024, WinY = 768;

unsigned int FrameCount = 0;

//shaders
VSShaderLib shader;  //geometry
VSShaderLib shaderText;  //render bitmap text

//File with the font
const string font_name = "fonts/arial.ttf";

// Lights

float directionalLightPos[4]{ 1.0f, 1000.0f, 1.0f, 0.0f };
int directionalON = 1;
float pointLightPos[2][4]{
{-50.0f, 20.0f, -50.0f, 0.0f},
{50.0f, 20.0f, 50.0f, 0.0f},
};

int pointON = 1;

//Vector with meshes
vector<struct MyMesh> myMeshes;
vector<struct MyMesh> boatMeshes;
vector<struct MyMesh> treeMeshes;
vector<struct MyMesh> floatMeshes;
vector<struct MyMesh> creatureMeshes;

// Boat
Boat boat;

// Creatures
WaterCreatureManager creatureManager;

//External array storage defined in AVTmathLib.cpp

/// The storage for matrices
extern float mMatrix[COUNT_MATRICES][16];
extern float mCompMatrix[COUNT_COMPUTED_MATRICES][16];

/// The normal matrix
extern float mNormal3x3[9];

GLint pvm_uniformId;
GLint vm_uniformId;
GLint normal_uniformId;
GLint lPos_uniformId;
GLint spot_loc0, spot_loc1;
GLint point_loc0, point_loc1;
GLint tex_loc0, tex_loc1, tex_loc2;
GLint dir_loc;

// Camera Position
int activeCamera = 0;
Camera cams[3];

// Mouse Tracking Variables
int startX, startY, tracking = 0;

// Camera Spherical Coordinates
float alpha = 39.0f, beta = 51.0f;
float r = 10.0f;

// Frame counting and FPS computation
long myTime, timebase = 0, frame = 0;
char s[32];
float lightPos[4] = { 4.0f, 6.0f, 2.0f, 1.0f };
int leftoar = 0;
int rightoar = 0;
float leftang = 0.0f;
float rightang = 0.0f;

void timer(int value)
{
	std::ostringstream oss;
	oss << CAPTION << ": " << FrameCount << " FPS @ (" << WinX << "x" << WinY << ")";
	std::string s = oss.str();
	glutSetWindow(WindowHandle);
	glutSetWindowTitle(s.c_str());
	FrameCount = 0;
	glutTimerFunc(1000, timer, 0);
}

void refresh(int value)
{
	boat.updateBoatMovement();
	boat.updateBoatRotationAngle();
	creatureManager.moveCreatures();
	glutPostRedisplay();
	glutTimerFunc(1000 / 60, refresh, 0);
}

// ------------------------------------------------------------
//
// Reshape Callback Function
//

void changeSize(int w, int h) {

	float ratio;
	// Prevent a divide by zero, when window is too short
	if (h == 0)
		h = 1;
	// set the viewport to be the entire window
	glViewport(0, 0, w, h);
	// set the projection matrix
	ratio = (1.0f * w) / h;
	loadIdentity(PROJECTION);
	const auto cameraType = cams[activeCamera].getType();
	if (cameraType == 1) {
		ortho(ratio * (-25), ratio * 25, -25, 25, 1.0f, 1000.0f);
	}
	else {
		perspective(53.13f, ratio, 1.0f, 1000.0f);
	}
}


// ------------------------------------------------------------
//
// Render stufff
//

static void setupRender() {
	FrameCount++;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// load identity matrices
	loadIdentity(VIEW);
	loadIdentity(MODEL);

	if (activeCamera == 2) {
		cams[activeCamera].setTarget({ boat.pos[0] + 0.5f, boat.pos[1], boat.pos[2] });
		cams[activeCamera].setPos({ boat.pos[0] + 0.5f, boat.pos[1] + 6, boat.pos[2] - 10 });
	}

	const auto cameraPos = cams[activeCamera].getPos();
	const auto cameraTarget = cams[activeCamera].getTarget();
	const auto cameraType = cams[activeCamera].getType();
	const auto cameraUp = cams[activeCamera].getUp();
	// set the camera using a function similar to gluLookAt
	lookAt(cameraPos[0], cameraPos[1], cameraPos[2], cameraTarget[0], cameraTarget[1], cameraTarget[2], cameraUp[0], cameraUp[1], cameraUp[2]);

	GLint m_view[4];
	glGetIntegerv(GL_VIEWPORT, m_view);
	float ratio = (m_view[2] - m_view[0]) / (m_view[3] - m_view[1]);
	loadIdentity(PROJECTION);
	if (cameraType == 1) {
		ortho(ratio * (-100), ratio * 100, -100, 100, 1.0f, 1000.0f);
	}
	else {
		if (activeCamera == 2) {
			perspective(75.0f, ratio, 1.0f, 1000.0f);
		}
		else {
			perspective(53.13f, ratio, 1.0f, 1000.0f);
		}
	}

	// use our shader

	glUseProgram(shader.getProgramIndex());

	//send the light position in eye coordinates
	//glUniform4fv(lPos_uniformId, 1, lightPos); //efeito capacete do mineiro, ou seja lighPos foi definido em eye coord 
	
	float res[4];
	multMatrixPoint(VIEW, lightPos, res);   //lightPos definido em World Coord so is converted to eye space
	glUniform4fv(lPos_uniformId, 1, res);
	

	if (pointON == 1) {
		multMatrixPoint(VIEW, pointLightPos[0], res);
		glUniform4fv(point_loc0, 1, res);
		multMatrixPoint(VIEW, pointLightPos[1], res);
		glUniform4fv(point_loc1, 1, res);
	}
	if (directionalON == 1) {
		multMatrixPoint(VIEW, directionalLightPos, res);
		glUniform4fv(dir_loc, 1, res);
	};
}

static void sendMaterial(const Material& mat) {
	GLint loc;
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
	glUniform4fv(loc, 1, mat.ambient);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
	glUniform4fv(loc, 1, mat.diffuse);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
	glUniform4fv(loc, 1, mat.specular);
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
	glUniform1f(loc, mat.shininess);
}

static void renderFloats() {
	pushMatrix(MODEL);

	for (int i = 0; i < 12; ++i) {
		// send the material
		sendMaterial(floatMeshes[i].mat);
		pushMatrix(MODEL);

		if (i == 0 || i == 1) {
			translate(MODEL, 19.0f, 0.2f, 75.0f);
		}
		if (i == 2 || i == 3) {
			translate(MODEL, -49.0f, 0.2f, -49.0f);
		}
		if (i == 4 || i == 5) {
			translate(MODEL, 50.0f, 0.2f, 0.0f);
		}
		if (i == 6 || i == 7) {
			translate(MODEL, -20.0f, 0.2f, 18.0f);
		}
		if (i == 8 || i == 9) {
			translate(MODEL, 10.0f, 0.2f, -55.0f);
		}
		if (i == 10 || i == 11) {
			translate(MODEL, -72.0f, 0.2f, 72.0f);
		}

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
		glBindVertexArray(floatMeshes[i].vao);

		glDrawElements(floatMeshes[i].type, floatMeshes[i].numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);
	}
	popMatrix(MODEL);
}

static void renderCreatures() {
	for (int i = 0; i < 6; i++) {
		sendMaterial(creatureMeshes[i].mat);
		pushMatrix(MODEL);

		auto creature = creatureManager.creatures[i];

		std::pair<float, float>* rotateDirs = creature.getRotateAxis();

		translate(MODEL, creature.pos[0], creature.pos[1], creature.pos[2]);
		rotate(MODEL, 90, rotateDirs->second, 0.0f, rotateDirs->first);
		rotate(MODEL, creature.spinAngle, 0.0f, 0.0, 1.0f);

		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
		glBindVertexArray(creatureMeshes[i].vao);

		glDrawElements(creatureMeshes[i].type, creatureMeshes[i].numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);
	}
}

void renderTree(){
	pushMatrix(MODEL);

	for (int i = 0; i < 16; ++i) {
		// send the material
		sendMaterial(treeMeshes[i].mat);
		pushMatrix(MODEL);


		if (i == 0) { //tree base big island 
			translate(MODEL, 45.0f, 9.5f, 53.0f);
			scale(MODEL, 0.5, 0.5, 0.5);
		}
		if (i == 1) { //tree top big island
			translate(MODEL,45.0f, 10.5f, 53.0f);
			scale(MODEL, 0.5, 0.5, 0.5);
		}
		if (i == 2) { //tree base big island
			translate(MODEL, 58.0f, 8.5f, 49.0f);
			scale(MODEL, 0.5, 0.5, 0.5);
		}
		if (i == 3) { //tree top big island
			translate(MODEL, 58.0f, 9.5f, 49.0f);
			scale(MODEL, 0.5, 0.5, 0.5);
		}
		if (i == 4) { //tree base big island
			translate(MODEL, 48.0f, 8.0f, 44.0f);
			scale(MODEL, 0.5, 0.5, 0.5);
		}
		if (i == 5) { //tree top big island
			translate(MODEL, 48.0f, 9.0f, 44.0f);
			scale(MODEL, 0.5, 0.5, 0.5);
		}
		if (i == 6) { //tree base big island
			translate(MODEL, 50.0f, 10.0f, 55.0f);
			scale(MODEL, 0.5, 0.5, 0.5);
		}
		if (i == 7) { //tree top big island
			translate(MODEL, 50.0f, 10.0f, 55.0f);
			scale(MODEL, 0.5, 0.5, 0.5);
		}
		if (i == 8) { //tree base medium island #1
			translate(MODEL, 63.0f, 7.0f, -47.0f);
			scale(MODEL, 0.5, 0.5, 0.5);
		}
		if (i == 9) { //tree top medium island #1
			translate(MODEL, 63.0f, 8.0f, -47.0f);
			scale(MODEL, 0.5, 0.5, 0.5);
		}
		if (i == 10) { //tree base medium island #1
			translate(MODEL, 60.0f, 9.0f, -42.0f);
			scale(MODEL, 0.5, 0.5, 0.5);
		}
		if (i == 11) { //tree top medium island #1
			translate(MODEL, 60.0f, 10.0f, -42.0f);
			scale(MODEL, 0.5, 0.5, 0.5);
		}
		if (i == 12) { //tree base medium island #2
			translate(MODEL, -48.0f, 8.0f, 1.0f);
			scale(MODEL, 0.5, 0.5, 0.5);
		}
		if (i == 13) { //tree top medium island #2
			translate(MODEL, -48.0f, 9.0f, 1.0f);
			scale(MODEL, 0.5, 0.5, 0.5);
		}
		if (i == 14) { //tree base medium island #2
			translate(MODEL, -53.0f, 7.0f, -3.0f);
			scale(MODEL, 0.5, 0.5, 0.5);
		}
		if (i == 15) { //tree top medium island #2
			translate(MODEL, -53.0f, 8.0f, -3.0f);
			scale(MODEL, 0.5, 0.5, 0.5);
		}


		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
		glBindVertexArray(treeMeshes[i].vao);

		glDrawElements(treeMeshes[i].type, treeMeshes[i].numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);
	}
	popMatrix(MODEL);
}

static void renderBoat() {
	loadIdentity(MODEL);

	pushMatrix(MODEL);

	translate(MODEL, boat.pos[0], boat.pos[1], boat.pos[2]);
	rotate(MODEL, boat.angle, 0, 1.0f, 0);
	
	for (int i = 0; i < 5; ++i) {
		// send the material
		sendMaterial(boatMeshes[i].mat);
		pushMatrix(MODEL);

		if (i == 0) { // pawn
			translate(MODEL, 0.5f, 0.5f, 1.0f);
			scale(MODEL, 0.25f, 0.25, 0.25);
		}
		if (i == 1) { //base
			translate(MODEL, 0.0f, 0.0f, -0.5f);
			scale(MODEL, 1.0, 0.5, 3.0);
		}
		if (i == 2) { // front
			translate(MODEL, 0.5f, 0.0f, 2.5f);
			scale(MODEL, 0.25f, 0.25f, 0.25f);
			rotate(MODEL, 90, 1, 0,0);
			rotate(MODEL, 45, 0, 1, 0);
		}
		if (i == 3) { //oars
			translate(MODEL, 1.5f, 0.0f, 1.5f);
			rotate(MODEL, 45, 0, 0, 1);
			
			if (leftoar == 1) {
				leftang += 5.0f;
				rotate(MODEL, leftang, -0.1 , -0.5, 0);
				if (leftang > 360.0f) {
					leftang = 0.0f;
					leftoar = 0;
				}
			}
			scale(MODEL, 0.1f, 1.0f, 0.1f);

		}
		if (i == 4) { //oars
			
			translate(MODEL, -0.5f, 0.0f, 1.5f);
			rotate(MODEL, -45, 0, 0, 1);
			if (rightoar == 1) {
				rightang += 5.0f;
				rotate(MODEL, rightang, 0.1, 0.5, 0);
				if (rightang > 360.0f) {
					rightang = 0.0f;
					rightoar = 0;
				}
			}
			scale(MODEL, 0.1f, 1.0f, 0.1f);
			

		}

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
		glBindVertexArray(boatMeshes[i].vao);
		glDrawElements(boatMeshes[i].type, boatMeshes[i].numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);
	}

	popMatrix(MODEL);
}

static void renderScene(void) {	
	setupRender();

	renderTree();
	renderFloats();
	renderCreatures();

	for (int j = 0; j < 2; ++j) {
		// send the material
		sendMaterial(myMeshes[j].mat);
		pushMatrix(MODEL);

		if (j == 0) { //water
			rotate(MODEL, -90, 1, 0, 0);
		}
		else if (j == 1) { // big island
			translate(MODEL, 50.0f, 0.0f, 50.0f);
			scale(MODEL, 1.5, 1, 1);
		}
		else if (j == 2) { //medium island #1
			translate(MODEL, 60.0f, 0.0f, -45.0f);
		}
		else if (j == 3) { //medium island #2
			translate(MODEL, -50.0f, 0.0f, 0.0f);
		}

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh
		glBindVertexArray(myMeshes[j].vao);

		glDrawElements(myMeshes[j].type, myMeshes[j].numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);
	}

	renderBoat();


	//Render text (bitmap fonts) in screen coordinates. So use ortoghonal projection with viewport coordinates.
	glDisable(GL_DEPTH_TEST);
	//the glyph contains transparent background colors and non-transparent for the actual character pixels. So we use the blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	int m_viewport[4];
	glGetIntegerv(GL_VIEWPORT, m_viewport);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	glutSwapBuffers();
}

// ------------------------------------------------------------
//
// Events from the Keyboard
//

void processKeys(unsigned char key, int xx, int yy)
{
	switch (key) {

	case 27:
		glutLeaveMainLoop();
		break;

	case 'm': glEnable(GL_MULTISAMPLE); break;

	case '1':
		activeCamera = 0;
		break;
	case '2':
		activeCamera = 1;
		break;
	case '3':
		activeCamera = 2;
		break;

	case 'A': case 'a':
		// right paddle stroke rotates the boat slightly to the left
		boat.rotate(-2.0f); // Rotate by -5 degrees
		boat.accelerate();
		rightoar = 1;

		break;
	case 'D': case 'd':
		// left paddle stroke rotates the boat slightly to the right
		boat.rotate(2.0f); // Rotate by 5 degrees
		boat.accelerate();
		leftoar = 1;

		break;
	case 'S': case 's':
		// Invert the paddle direction
		boat.directionModifier *= -1;
		break;
	case 'O': case 'o':
		// Increase paddle strength
		boat.addPaddleStrength(0.4f); // Increase by 0.1
		break;

	case 'N': case 'n':
		//Toggle directional light
		if (directionalON == 1) {
			directionalON = 0;
		}
		else {
			directionalON = 1;
		}
		break;
	case 'C': case 'c':
		//Toggle point lights
		if (pointON == 1) {
			pointON = 0;
		}
		else {
			pointON = 1;
		}
		break;
	}

}


// ------------------------------------------------------------
//
// Mouse Events
//

void processMouseButtons(int button, int state, int xx, int yy)
{
	// start tracking the mouse
	if (state == GLUT_DOWN) {
		startX = xx;
		startY = yy;
		if (button == GLUT_LEFT_BUTTON)
			tracking = 1;
		else if (button == GLUT_RIGHT_BUTTON)
			tracking = 2;
	}

	//stop tracking the mouse
	else if (state == GLUT_UP) {
		if (tracking == 1) {
			alpha -= (xx - startX);
			beta += (yy - startY);
		}
		else if (tracking == 2) {
			r += (yy - startY) * 0.01f;
			if (r < 0.1f)
				r = 0.1f;
		}
		tracking = 0;
	}
}

// Track mouse motion while buttons are pressed

void processMouseMotion(int xx, int yy)
{

	int deltaX, deltaY;
	float alphaAux, betaAux;
	float rAux;

	deltaX = -xx + startX;
	deltaY = yy - startY;

	// left mouse button: move camera
	if (tracking == 1) {


		alphaAux = alpha + deltaX;
		betaAux = beta + deltaY;

		if (betaAux > 85.0f)
			betaAux = 85.0f;
		else if (betaAux < -85.0f)
			betaAux = -85.0f;
		rAux = r;
	}
	// right mouse button: zoom
	else if (tracking == 2) {

		alphaAux = alpha;
		betaAux = beta;
		rAux = r + (deltaY * 0.01f);
		if (rAux < 0.1f)
			rAux = 0.1f;
	}

	float camX = rAux * sin(alphaAux * 3.14f / 180.0f) * cos(betaAux * 3.14f / 180.0f);
	float camZ = rAux * cos(alphaAux * 3.14f / 180.0f) * cos(betaAux * 3.14f / 180.0f);
	float camY = rAux * sin(betaAux * 3.14f / 180.0f);

	if (activeCamera == 2) {
		cams[activeCamera].setPos({ camX, camY, camZ });
	}

	//  uncomment this if not using an idle or refresh func
	//	glutPostRedisplay();
}


void mouseWheel(int wheel, int direction, int x, int y) {

	r += direction * 0.1f;
	if (r < 0.1f)
		r = 0.1f;

	float camX = r * sin(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	float camZ = r * cos(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	float camY = r * sin(beta * 3.14f / 180.0f);

	cams[activeCamera].setPos({ camX, camY, camZ });

	//  uncomment this if not using an idle or refresh func
	//	glutPostRedisplay();
}

// --------------------------------------------------------
//
// Shader Stuff
//


GLuint setupShaders() {

	// Shader for models
	shader.init();
	shader.loadShader(VSShaderLib::VERTEX_SHADER, "shaders/pointlight_phong.vert");
	shader.loadShader(VSShaderLib::FRAGMENT_SHADER, "shaders/pointlight_phong.frag");

	// set semantics for the shader variables
	glBindFragDataLocation(shader.getProgramIndex(), 0, "colorOut");
	glBindAttribLocation(shader.getProgramIndex(), VERTEX_COORD_ATTRIB, "position");
	glBindAttribLocation(shader.getProgramIndex(), NORMAL_ATTRIB, "normal");
	//glBindAttribLocation(shader.getProgramIndex(), TEXTURE_COORD_ATTRIB, "texCoord");

	glLinkProgram(shader.getProgramIndex());
	printf("InfoLog for Model Rendering Shader\n%s\n\n", shaderText.getAllInfoLogs().c_str());

	if (!shader.isProgramValid()) {
		printf("InfoLog for Model Rendering Shader\n%s\n\n", shaderText.getAllInfoLogs().c_str());
		printf("GLSL Model Program Not Valid!\n");
		exit(1);
	}

	pvm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_pvm");
	vm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_viewModel");
	normal_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_normal");
	//lPos_uniformId = glGetUniformLocation(shader.getProgramIndex(), "l_pos");
	point_loc0 = glGetUniformLocation(shader.getProgramIndex(),"pointLights[0].position");
	point_loc1 = glGetUniformLocation(shader.getProgramIndex(),"pointLights[1].position");
	dir_loc = glGetUniformLocation(shader.getProgramIndex(), "dirLight.direction");
	spot_loc0 = glGetUniformLocation(shader.getProgramIndex(),"spotLights[0].position");
	spot_loc1 = glGetUniformLocation(shader.getProgramIndex(),"spotLights[1].position");
	tex_loc0 = glGetUniformLocation(shader.getProgramIndex(), "texmap0");
	tex_loc1 = glGetUniformLocation(shader.getProgramIndex(), "texmap1");
	tex_loc2 = glGetUniformLocation(shader.getProgramIndex(), "texmap2");

	printf("InfoLog for Per Fragment Phong Lightning Shader\n%s\n\n", shader.getAllInfoLogs().c_str());

	// Shader for bitmap Text
	shaderText.init();
	shaderText.loadShader(VSShaderLib::VERTEX_SHADER, "shaders/text.vert");
	shaderText.loadShader(VSShaderLib::FRAGMENT_SHADER, "shaders/text.frag");

	glLinkProgram(shaderText.getProgramIndex());
	printf("InfoLog for Text Rendering Shader\n%s\n\n", shaderText.getAllInfoLogs().c_str());

	if (!shaderText.isProgramValid()) {
		printf("GLSL Text Program Not Valid!\n");
		exit(1);
	}

	return(shader.isProgramLinked() && shaderText.isProgramLinked());
}

// ------------------------------------------------------------
//
// Model loading and OpenGL setup
//

void init()
{
	MyMesh amesh;

	/* Initialization of DevIL */
	if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION)
	{
		printf("wrong DevIL version \n");
		exit(0);
	}
	ilInit();

	/// Initialization of freetype library with font_name file
	freeType_init(font_name);

	// set the camera position based on its spherical coordinates
	/*camX = r * sin(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camZ = r * cos(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camY = r * sin(beta * 3.14f / 180.0f);*/

	cams[0].setPos({ 0, 200, 0 });
	cams[0].setUp({ 0, 0, 1 });

	cams[1].setPos({ 0, 150, 0 });	
	cams[1].setUp({ 0, 0, 1 });
	cams[1].setType(1);

	cams[2].setPos({ 4, 16, 4 });

	float h20_amb[] = { 0.0f, 0.0f, 0.25f, 1.0f };
	float h20_diff[] = { 0.1f, 0.1f, 0.8f, 1.0f };
	float h20_spec[] = { 0.1f, 0.1f, 0.3f, 1.0f };
	float emissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininess = 300.0f;
	int texcount = 0;

	//Create Plane/Water
	amesh = createQuad(200, 200);
	memcpy(amesh.mat.ambient, h20_amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, h20_diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, h20_spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh);

	//Create Pawn Boat
	float pawn_amb[] = { 0.2f, 0.15f, 0.1f, 1.0f };
	float pawn_diff[] = { 0.8f, 0.6f, 0.4f, 1.0f };
	float pawn_spec[] = { 0.8f, 0.8f, 0.8f, 1.0f };

	amesh = createPawn();
	memcpy(amesh.mat.ambient, pawn_amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, pawn_diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, pawn_spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	boatMeshes.push_back(amesh);

	//Create Big Island
	float grass_amb[] = { 0.0f, 0.3f, 0.0f, 1.0f };
	float grass_diff[] = { 0.1f, 0.8f, 0.1f, 1.0f };
	float grass_spec[] = { 0.9f, 0.9f, 0.9f, 1.0f };

	amesh = createSphere(10.0f, 50);
	memcpy(amesh.mat.ambient, grass_amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, grass_diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, grass_spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh);

	//Create Middle islands
	for (int i = 0; i < 2; i++) {

		amesh = createSphere(8.0f, 50);
		memcpy(amesh.mat.ambient, grass_amb, 4 * sizeof(float));
		memcpy(amesh.mat.diffuse, grass_diff, 4 * sizeof(float));
		memcpy(amesh.mat.specular, grass_spec, 4 * sizeof(float));
		memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
		amesh.mat.shininess = shininess;
		amesh.mat.texCount = texcount;
		myMeshes.push_back(amesh);

	}

	//Create Trees
	int numOfTrees = 8;

	for (int i = 0; i < numOfTrees; i++) {
		amesh = createCylinder(4.0f, 1.0f, 50);
		memcpy(amesh.mat.ambient, pawn_amb, 4 * sizeof(float));
		memcpy(amesh.mat.diffuse, pawn_diff, 4 * sizeof(float));
		memcpy(amesh.mat.specular, pawn_spec, 4 * sizeof(float));
		memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
		amesh.mat.shininess = shininess;
		amesh.mat.texCount = texcount;
		treeMeshes.push_back(amesh);


		amesh = createCone(4.0f, 2.0f, 50);
		memcpy(amesh.mat.ambient, grass_amb, 4 * sizeof(float));
		memcpy(amesh.mat.diffuse, grass_diff, 4 * sizeof(float));
		memcpy(amesh.mat.specular, grass_spec, 4 * sizeof(float));
		memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
		amesh.mat.shininess = shininess;
		amesh.mat.texCount = texcount;
		treeMeshes.push_back(amesh);


	}

	//Create Boat
	amesh = createCube();
	memcpy(amesh.mat.ambient, pawn_amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, pawn_diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, pawn_spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	boatMeshes.push_back(amesh);

	amesh = createCone(1.0f, 2.0f, 4);
	memcpy(amesh.mat.ambient, pawn_amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, pawn_diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, pawn_spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	boatMeshes.push_back(amesh);

	amesh = createCylinder(1.0f, 1.0f, 50);
	memcpy(amesh.mat.ambient, pawn_amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, pawn_diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, pawn_spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	boatMeshes.push_back(amesh);
	boatMeshes.push_back(amesh);

	//Create Floats
	int numOfFloats = 6;

	float float_amb[] = { 0.3f, 0.0f, 0.0f, 1.0f };
	float float_diff[] = { 0.8f, 0.1f, 0.1f, 1.0f };
	float float_spec[] = { 0.9f, 0.9f, 0.9f, 1.0f };

	for (int i = 0; i < numOfFloats; i++) {

		amesh = createCylinder(6.0f, 0.5f, 50);
		memcpy(amesh.mat.ambient, float_amb, 4 * sizeof(float));
		memcpy(amesh.mat.diffuse, float_diff, 4 * sizeof(float));
		memcpy(amesh.mat.specular, float_spec, 4 * sizeof(float));
		memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
		amesh.mat.shininess = shininess;
		amesh.mat.texCount = texcount;
		floatMeshes.push_back(amesh);


		amesh = createSphere(1.0f, 10);
		memcpy(amesh.mat.ambient, float_amb, 4 * sizeof(float));
		memcpy(amesh.mat.diffuse, float_diff, 4 * sizeof(float));
		memcpy(amesh.mat.specular, float_spec, 4 * sizeof(float));
		memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
		amesh.mat.shininess = shininess;
		amesh.mat.texCount = texcount;
		floatMeshes.push_back(amesh);

	}

	float creature_amb[] = { 0.0f, 0.2f, 0.1f, 1.0f };    // A dark green ambient color
	float creature_diff[] = { 0.1f, 0.6f, 0.3f, 1.0f };   // A medium green diffuse color
	float creature_spec[] = { 0.4f, 0.8f, 0.4f, 1.0f };   // A bright green specular highlight
	float creature_emissive[] = { 0.0f, 0.1f, 0.0f, 1.0f };   // A very faint green emissive glow

	for (auto &creature : creatureManager.creatures) {
		amesh = createCylinder(2.0f, 0.5f, 50);
		memcpy(amesh.mat.ambient, creature_amb, 4 * sizeof(float));
		memcpy(amesh.mat.diffuse, creature_diff, 4 * sizeof(float));
		memcpy(amesh.mat.specular, creature_spec, 4 * sizeof(float));
		memcpy(amesh.mat.emissive, creature_emissive, 4 * sizeof(float));
		amesh.mat.shininess = shininess;
		amesh.mat.texCount = texcount;
		creatureMeshes.push_back(amesh);
	}

	// some GL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

}

// ------------------------------------------------------------
//
// Main function
//


int main(int argc, char** argv) {

	//  GLUT initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);

	glutInitContextVersion(4, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);

	glutInitWindowPosition(100, 100);
	glutInitWindowSize(WinX, WinY);
	WindowHandle = glutCreateWindow(CAPTION);


	//  Callback Registration
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);

	glutTimerFunc(0, timer, 0);
	//glutIdleFunc(renderScene);  // Use it for maximum performance
	glutTimerFunc(0, refresh, 0);    //use it to to get 60 FPS whatever

	//	Mouse and Keyboard Callbacks
	glutKeyboardFunc(processKeys);
	glutMouseFunc(processMouseButtons);
	glutMotionFunc(processMouseMotion);
	glutMouseWheelFunc(mouseWheel);


	//	return from main loop
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	//	Init GLEW
	glewExperimental = GL_TRUE;
	glewInit();

	printf("Vendor: %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("Version: %s\n", glGetString(GL_VERSION));
	printf("GLSL: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	if (!setupShaders())
		return(1);

	init();

	//  GLUT main loop
	glutMainLoop();

	return(0);
}