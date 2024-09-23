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

//Vector with meshes
vector<struct MyMesh> myMeshes;
vector<struct MyMesh> boatMeshes;
vector<struct MyMesh> treeMeshes;
vector<struct MyMesh> floatMeshes;

// Boat

Boat boat;

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
GLint tex_loc, tex_loc1, tex_loc2;

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
	//PUT YOUR CODE HERE
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
		ortho(ratio * (-25), ratio * 25, -25, 25, 1.0f, 100.0f);
	}
	else {
		perspective(53.13f, ratio, 1.0f, 100.0f);
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
		cams[activeCamera].setTarget({ boat.pos[0], boat.pos[1], boat.pos[2] });
	}
	const auto cameraPos = cams[activeCamera].getPos();
	const auto cameraTarget = cams[activeCamera].getTarget();
	const auto cameraType = cams[activeCamera].getType();
	// set the camera using a function similar to gluLookAt
	lookAt(cameraPos[0], cameraPos[1], cameraPos[2], cameraTarget[0], cameraTarget[1], cameraTarget[2], 0, 1, 0);

	GLint m_view[4];
	glGetIntegerv(GL_VIEWPORT, m_view);
	float ratio = (m_view[2] - m_view[0]) / (m_view[3] - m_view[1]);
	loadIdentity(PROJECTION);
	if (cameraType == 1) {
		ortho(ratio * (-25), ratio * 25, -25, 25, 1.0f, 100.0f);
	}
	else {
		perspective(53.13f, ratio, 1.0f, 100.0f);
	}

	// use our shader

	glUseProgram(shader.getProgramIndex());

	//send the light position in eye coordinates
	//glUniform4fv(lPos_uniformId, 1, lightPos); //efeito capacete do mineiro, ou seja lighPos foi definido em eye coord 

	float res[4];
	multMatrixPoint(VIEW, lightPos, res);   //lightPos definido em World Coord so is converted to eye space
	glUniform4fv(lPos_uniformId, 1, res);
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

	for (int i = 0; i < 4; ++i) {
		// send the material
		sendMaterial(floatMeshes[i].mat);
		pushMatrix(MODEL);

		if (i == 0 || i == 1) {
			translate(MODEL, 10.0f, 0.2f, 10.0f);
		}
		if (i == 2 || i == 3) {
			translate(MODEL, 10.0f, 0.2f, 0.0f);
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

void renderTree(){
	pushMatrix(MODEL);

	for (int i = 0; i < 2; ++i) {
		// send the material
		sendMaterial(treeMeshes[i].mat);
		pushMatrix(MODEL);


		if (i == 0) { //tree base
			translate(MODEL, 50.0f, 10.0f, 50.0f);
			scale(MODEL, 0.5, 0.5, 0.5);
			}
		else if (i == 1) { //tree top
			translate(MODEL, 50.0f, 11.0f, 50.0f);
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
			translate(MODEL, 1.0f, 0.5f, 0.0f);
			scale(MODEL, 0.25f, 0.25, 0.25);
		}
		if (i == 1) { //base
			translate(MODEL, 0.0f, 0.0f, -0.5f);
			scale(MODEL, 3.0, 0.5, 1.0);
		}
		if (i == 2) { // front
			translate(MODEL, 3.0f, 0.0f, 0.0f);
			scale(MODEL, 0.25f, 0.25f, 0.25f);
			rotate(MODEL, -90, 0, 0, 1);
			rotate(MODEL, -45, 0, 1, 0);
		}
		if (i == 3) { //oars
			translate(MODEL, 1.5f, 0.0f, 1.0f);
			scale(MODEL, 0.1f, 1.0f, 0.1f);

		}
		if (i == 4) { //oars
			translate(MODEL, 1.5f, 0.0f, -1.0f);
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

void renderScene(void) {
	boat.updateBoatMovement();
	boat.updateBoatRotationAngle();

	setupRender();

	renderTree();
	renderFloats();
	

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

	//viewer at origin looking down at  negative z direction
	//pushMatrix(MODEL);
	//loadIdentity(MODEL);
	//pushMatrix(PROJECTION);
	//loadIdentity(PROJECTION);
	//pushMatrix(VIEW);
	//loadIdentity(VIEW);
	//ortho(m_viewport[0], m_viewport[0] + m_viewport[2] - 1, m_viewport[1], m_viewport[1] + m_viewport[3] - 1, -1, 1);
	//RenderText(shaderText, "This is a sample text", 25.0f, 25.0f, 1.0f, 0.5f, 0.8f, 0.2f);
	//RenderText(shaderText, "AVT Light and Text Rendering Demo", 440.0f, 570.0f, 0.5f, 0.3f, 0.7f, 0.9f);
	//popMatrix(PROJECTION);
	//popMatrix(VIEW);
	//popMatrix(MODEL);
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

	case 'c':
		printf("Camera Spherical Coordinates (%f, %f, %f)\n", alpha, beta, r);
		break;
	case 'm': glEnable(GL_MULTISAMPLE); break;
	case 'n': glDisable(GL_MULTISAMPLE); break;

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
		// Left paddle stroke rotates the boat slightly to the left
		boat.rotate(-5.0f); // Rotate by -5 degrees
		boat.accelerate();
		break;
	case 'D': case 'd':
		// Right paddle stroke rotates the boat slightly to the right
		boat.rotate(5.0f); // Rotate by 5 degrees
		boat.accelerate();
		break;
	case 'S': case 's':
		// Invert the paddle direction
		boat.directionModifier *= -1;
		break;
	//case 'O': case 'o':
	//	// Increase paddle strength
	//	boat.addPaddleStrength(0.1f); // Increase by 0.1
	//	break;
	//case 'P': case 'p':
	//	// Decrease paddle strength
	//	boat.addPaddleStrength(-0.1f); // Decrease by 0.1
	//	break;
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
	lPos_uniformId = glGetUniformLocation(shader.getProgramIndex(), "l_pos");
	tex_loc = glGetUniformLocation(shader.getProgramIndex(), "texmap");
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

	cams[0].setPos({ 4, 16, 4 });
	cams[1].setPos({ 4, 8, 4 });
	cams[1].setType(1);

	cams[2].setPos({ 4, 16, 4 });
	cams[2].setTarget({});

	float h20_amb[] = { 0.0f, 0.0f, 0.25f, 1.0f };
	float h20_diff[] = { 0.1f, 0.1f, 0.8f, 1.0f };
	float h20_spec[] = { 0.9f, 0.9f, 0.9f, 1.0f };
	float emissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininess = 100.0f;
	int texcount = 0;


	//Create Plane/Water
	amesh = createQuad(500, 500);
	memcpy(amesh.mat.ambient, h20_amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, h20_diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, h20_spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	myMeshes.push_back(amesh);

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

	int numOfTrees = 1;

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

	int numOfFloats = 2;

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