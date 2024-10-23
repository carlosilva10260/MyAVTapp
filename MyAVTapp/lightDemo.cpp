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
// Author: Jo„o Madeiras Pereira
//

#include <cstdio>
#include <math.h>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

// include GLEW to access OpenGL 3.3 functions
#include <GL/glew.h>


// GLUT is the toolkit to interface with the OS
#include <GL/freeglut.h>

#include <IL/il.h>

// assimp include files. These three are usually needed.
#include "assimp/Importer.hpp"	//OO version Header!
#include "assimp/scene.h"

// Use Very Simple Libs
#include "VSShaderlib.h"
#include "VertexAttrDef.h"
#include "AVTmathLib.h"
#include "geometry.h"
#include "Texture_Loader.h"
#include "l3dBillboard.h"
#include "meshFromAssimp.h"
#include "avtFreeType.h"

#include "camera.h"
#include "boat.h"
#include "waterCreatureManager.h"
#include "aabb.h"
#include "redfloat.h"
#include "GameTime.h"
#include <BoundingSphere.h>
#include <flare.h>

// using namespace std;
using std::string;
using std::array;
using std::vector;
using std::pair;

#define CAPTION "AVT Demo: Phong Shading and Text rendered with FreeType"
#define frand()			((float)rand()/RAND_MAX)
#define M_PI			3.14159265
#define MAX_PARTICULAS  1500


int WindowHandle = 0;
int WinX = 1024, WinY = 768;

unsigned int FrameCount = 0;

Assimp::Importer importer1;
const aiScene* scene1;
float scaleFactor1;
GLuint* textureIds1;  //for the backpack
char model_dir[50];

//shaders
VSShaderLib shader;  //geometry
VSShaderLib shaderText;  //render bitmap text

//File with the font
const string font_name = "fonts/arial.ttf";

bool normalMapKey = TRUE;

// Boat
Boat boat;
int aKey = 0;
int dKey = 0;
int sKey = 0;


//Firework

typedef struct {
	float	life;		// vida
	float	fade;		// fade
	float	r, g, b;    // color
	GLfloat x, y, z;    // posi¬ç‚Äπo
	GLfloat vx, vy, vz; // velocidade 
	GLfloat ax, ay, az; // acelera¬ç‚Äπo
} Particle;

Particle particula[MAX_PARTICULAS];
int dead_num_particles = 0;
int fireworks = 0;



// Lights

float directionalLightPos[4]{ 200.0f, 200.0f, 1.0f, 0.0f };
int directionalON = 1;
float pointLightPos[6][4]{
{ 19.0f, 3.0f, 75.0f, 1.0f},
{-49.0f, 3.0f, -49.0f, 1.0f},
{ 50.0f, 3.0f, 0.0f, 1.0f},
{-20.0f, 3.0f, 18.0f, 1.0f},
{10.0f, 3.0f, -55.0f, 1.0f},
{-72.0f, 3.0f, 72.0f, 1.0f},
};

int pointON = 1;


//Spotlights

float spotLightPos[2][4]{
	{boat.pos[0], boat.pos[1]+1.5, boat.pos[2], 1.0},
	{boat.pos[0], boat.pos[1]+1.0, boat.pos[2], 1.0},
};

float spotLightDir[2][4]{
	{-boat.dir[0], -boat.dir[1], -boat.dir[2], 0.0f},
	{-boat.dir[0], -boat.dir[1], -boat.dir[2], 0.0f},
};
float spotLightAngle[2]{ 10.0f, 10.0f };

int spotON = 1;

//Vector with meshes
vector<struct MyMesh> myMeshes;
vector<struct MyMesh> skyMesh;
vector<struct MyMesh> flareMeshes;
vector<struct MyMesh> particleMeshes;
vector<struct MyMesh> myMeshes1; //backpack array
array<BoundingSphere, 3> islandBoundingSpheres;
vector<struct MyMesh> boatMeshes;
vector<struct MyMesh> treeMeshes;
vector<struct MyMesh> floatMeshes;
vector<struct MyMesh> mesh;
array<AABB, 6> floatAABBs;
vector<struct MyMesh> creatureMeshes;

int fogON = 0;

// Creatures
WaterCreatureManager creatureManager;

// Floats
array<Redfloat, 7> floats;

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
GLint spot_pos_loc0, spot_pos_loc1, spot_dir_loc0, spot_dir_loc1, spot_angle_loc0, spot_angle_loc1;
GLint point_loc0, point_loc1, point_loc2, point_loc3, point_loc4, point_loc5;
GLint tex_loc0, tex_loc1, tex_loc2, tex_loc3, tex_loc4, tex_loc5, tex_normalMap_loc, tex_cube_loc;
GLint dir_loc;
GLint texMode_uniformId;
GLint normalMap_loc;
GLint specularMap_loc;
GLint diffMapCount_loc;
GLint view_uniformId;
GLint model_uniformId;

GLint dir_toggle, point_toggle, spot_toggle, fog_toggle;

GLuint TextureArray[7];
GLuint FlareTextureArray[5];

FLARE_DEF AVTflare;
float lightScreenPos[3];  //Position of the light in Window Coordinates
bool flareEffect = true;

int deltaMove = 0, deltaUp = 0, type = 0;



//bump mapping 
int bumpmapping = 0;

// Camera Position
int activeCamera = 0;
Camera cams[3];

// Text coords
float livesX = 10.0f;
float livesY = 728.0f;
float timeX = 700.0f;
float timeY = 728.0f;

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

// Lives and Timer
int lives = 5;
GameTime elapsedTime;
int lastTime = 0;

// Game state
bool paused = false;
bool restart = false;
bool goal_flag = false;

enum CollisionType { NONE, CREATURE, RED_FLOAT, ISLAND, OUT_OF_BOUNDS };

inline double clamp(const double x, const double min, const double max) {
	return (x < min ? min : (x > max ? max : x));
}

inline int clampi(const int x, const int min, const int max) {
	return (x < min ? min : (x > max ? max : x));
}

static void goal_position() {
	int goal_pos = rand() % 5;

	if (goal_pos == 0) {
		floats[6].pos = { -69.0f, 0.2f, -69.0f };
	}
	else if (goal_pos == 1) {
		floats[6].pos = { 69.0f, 0.2f, 69.0f };
	}
	else if (goal_pos == 2) {
		floats[6].pos = { -75.0f, 0.2f, 75.0f };
	}
	else if (goal_pos == 3) {
		floats[6].pos = { 69.0f, 0.2f, -69.0f };
	}
	else if (goal_pos == 4) {
		floats[6].pos = { -39.0f, 0.2f, -69.0f };
	}
}

static void restartGame() {
	boat.resetBoatPosition();
	elapsedTime.reset();
	goal_position();
	lives = 5;
}

void updateParticles()
{
	int i;
	float h;

	/* M√©todo de Euler de integra√ß√£o de eq. diferenciais ordin√°rias
	h representa o step de tempo; dv/dt = a; dx/dt = v; e conhecem-se os valores iniciais de x e v */

	//h = 0.125f;
	h = 0.033;
	if (fireworks) {

		for (i = 0; i < MAX_PARTICULAS; i++)
		{
			particula[i].x += (h * particula[i].vx);
			particula[i].y += (h * particula[i].vy);
			particula[i].z += (h * particula[i].vz);
			particula[i].vx += (h * particula[i].ax);
			particula[i].vy += (h * particula[i].ay);
			particula[i].vz += (h * particula[i].az);
			particula[i].life -= particula[i].fade;
		}
	}
}

void iniParticles(void)
{
	GLfloat v, theta, phi;
	int i;

	for (i = 0; i < MAX_PARTICULAS; i++)
	{
		v = 0.8 * frand() + 0.2;
		phi = frand() * M_PI;
		theta = 2.0 * frand() * M_PI;

		particula[i].x = floats[6].pos[0];
		particula[i].y = floats[6].pos[1] + 2.0f;
		particula[i].z = floats[6].pos[2];
		particula[i].vx = v * cos(theta) * sin(phi);
		particula[i].vy = v * cos(phi);
		particula[i].vz = v * sin(theta) * sin(phi);
		particula[i].ax = 0.1f; /* simular um pouco de vento */
		particula[i].ay = -0.15f; /* simular a acelera√ß√£o da gravidade */
		particula[i].az = 0.0f;

		/* tom amarelado que vai ser multiplicado pela textura que varia entre branco e preto */
		particula[i].r = 0.882f;
		particula[i].g = 0.552f;
		particula[i].b = 0.211f;

		particula[i].life = 1.0f;		/* vida inicial */
		particula[i].fade = 0.0025f;	    /* step de decr√©scimo da vida para cada itera√ß√£o */
	}
}


void timer(int value)
{
	if (!paused && !restart) {
		elapsedTime.addSecond();
	}
	std::ostringstream oss;
	oss << CAPTION << ": " << FrameCount << " FPS @ (" << WinX << "x" << WinY << ")";
	std::string s = oss.str();
	glutSetWindow(WindowHandle);
	glutSetWindowTitle(s.c_str());
	FrameCount = 0;
	glutTimerFunc(1000, timer, 0);
}

pair<CollisionType, int> isBoatColliding() {
	for (auto& creature : creatureManager.creatures) {
		if (AABB::isColliding(creature.getCreatureAABB(), boat.getBoatAABB())) {
			return { CREATURE, 0 };
		}
	}

	for (int i = 0; i < 3; i++) {
		if (BoundingSphere::isColliding(islandBoundingSpheres[i], boat.getBoatAABB())) {
			return { ISLAND, i };
		}
	}

	for (int i = 0; i < 7; i++) {
		if (AABB::isColliding(floats[i].getFloatAABB(), boat.getBoatAABB())) {
			if (floats[i].yellow) {
				fireworks = 1;
				iniParticles();
			}
			printf("before %f %f %f\n", floats[i].pos[0], floats[i].pos[1], floats[i].pos[2]);
			return { RED_FLOAT, i };
		}
	}

	if (boat.pos[0] > 98 || boat.pos[0] < -98) {
		return { OUT_OF_BOUNDS, 0 };
	}

	if (boat.pos[2] > 98 || boat.pos[0] < -98) {
		return { OUT_OF_BOUNDS, 0 };
	}

	return { NONE, 0 };
}





void refresh(int value)
{
	int currentTime = glutGet(GLUT_ELAPSED_TIME);
	float deltaTime = (currentTime - lastTime) / 1000.0f;
	lastTime = currentTime;
	if (paused || restart) {
		glutPostRedisplay();
		glutTimerFunc(1000 / 60, refresh, 0);
		return;
	}
	boat.updateBoatMovement(deltaTime);
	boat.updateBoatRotationAngle();
	creatureManager.moveCreatures();
	pair<CollisionType, int> collisionPair = isBoatColliding();
	CollisionType collisionType = collisionPair.first;
	if (collisionType == CREATURE) {
		printf("COLLISION WITH CREATURE DETECTED!!!\n");
		boat.resetBoatPosition();
		lives--;
	}
	else if (collisionType == ISLAND) {
		printf("COLLISION WITH ISLAND DETECTED!!!\n");
		boat.stop();
	}
	else if (collisionType == RED_FLOAT) {
		printf("COLLISION WITH FLOAT DETECTED!!!\n");
		floats[collisionPair.second].onCollide(boat.dir, boat.speed);
		boat.stop();
	}
	else if (collisionType == OUT_OF_BOUNDS) {
		printf("OUT OF BOUNDS DETECTED!!!\n");
		boat.stop();
	}

	for (auto& redfloat : floats) {
		redfloat.updateFloatPos(deltaTime);
	}

	glutPostRedisplay();
	glutTimerFunc(1000 / 60, refresh, 0);
}

// ------------------------------------------------------------
//
// Reshape Callback Function
//

void changeSize(int w, int h) {
	const int initialW = 1024;
	const int initialH = 768;

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

	int wDiff = w - initialW;
	int hDiff = h - initialH;

	livesX = std::max(10.0f + wDiff, 0.0f);
	livesY = 728.0f + hDiff;
	timeX = std::max(700.0f + wDiff, 0.0f);
	timeY = 728.0f + hDiff;
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
		// Convert alpha and beta to radians for trigonometric functions
		float alphaRad = cams[activeCamera].getAlpha() * 3.14159f / 180.0f;
		float betaRad = cams[activeCamera].getBeta() * 3.14159f / 180.0f;
		float r = cams[activeCamera].getR();  // Distance from boat

		// Use the boat's direction vector to offset the camera position
		std::array<float, 3> boatDir = boat.dir;  // Direction vector (normalized)
		float boatForwardX = boatDir[0];
		float boatForwardZ = boatDir[2];

		// Apply alpha (horizontal rotation) to rotate around the boat's Y axis
		float rotatedX = boatForwardX * cos(alphaRad) - boatForwardZ * sin(alphaRad);
		float rotatedZ = boatForwardX * sin(alphaRad) + boatForwardZ * cos(alphaRad);

		// Position the camera behind the boat, adjusted by the rotated direction
		float camX = boat.pos[0] - rotatedX * r * cos(betaRad);
		float camY = boat.pos[1] + r * sin(betaRad);  // Adjust vertical position
		float camZ = boat.pos[2] - rotatedZ * r * cos(betaRad);

		cams[activeCamera].setPos({ camX, camY, camZ });

		// Target the boat's position, using the boat's forward direction
		float targetX = boat.pos[0] + rotatedX;
		float targetY = boat.pos[1];
		float targetZ = boat.pos[2] + rotatedZ;

		cams[activeCamera].setTarget({ targetX, targetY, targetZ });
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

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TextureArray[0]);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, TextureArray[1]);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, TextureArray[2]);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, TextureArray[3]);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, TextureArray[4]);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, TextureArray[5]);

	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_CUBE_MAP, TextureArray[6]);


	//Indicar aos tres samplers do GLSL quais os Texture Units a serem usados
	glUniform1i(tex_loc0, 0);
	glUniform1i(tex_loc1, 1);
	glUniform1i(tex_loc2, 2);
	glUniform1i(tex_loc3, 3);
	glUniform1i(tex_loc4, 4);
	glUniform1i(tex_normalMap_loc, 5);
	glUniform1i(tex_cube_loc, 6);


	//send the light position in eye coordinates
	//glUniform4fv(lPos_uniformId, 1, lightPos); //efeito capacete do mineiro, ou seja lighPos foi definido em eye coord 
	
	float res[4];
	multMatrixPoint(VIEW, lightPos, res);   //lightPos definido em World Coord so is converted to eye space
	glUniform4fv(lPos_uniformId, 1, res);
	
	//point light
	multMatrixPoint(VIEW, pointLightPos[0], res);
	glUniform4fv(point_loc0, 1, res);
	multMatrixPoint(VIEW, pointLightPos[1], res);
	glUniform4fv(point_loc1, 1, res);
	multMatrixPoint(VIEW, pointLightPos[2], res);
	glUniform4fv(point_loc2, 1, res);
	multMatrixPoint(VIEW, pointLightPos[3], res);
	glUniform4fv(point_loc3, 1, res);
	multMatrixPoint(VIEW, pointLightPos[4], res);
	glUniform4fv(point_loc4, 1, res);
	multMatrixPoint(VIEW, pointLightPos[5], res);
	glUniform4fv(point_loc5, 1, res);

	glUniform1i(point_toggle, pointON);


	//directional light
	multMatrixPoint(VIEW, directionalLightPos, res);
	glUniform4fv(dir_loc, 1, res);

	glUniform1i(dir_toggle, directionalON);




	//spotlights
	multMatrixPoint(VIEW, spotLightPos[0], res);
	glUniform4fv(spot_pos_loc0, 1, res);
	multMatrixPoint(VIEW, spotLightPos[1], res);
	glUniform4fv(spot_pos_loc1, 1, res);

	multMatrixPoint(VIEW, spotLightDir[0], res);
	glUniform4fv(spot_dir_loc0, 1, res);
	multMatrixPoint(VIEW, spotLightDir[1], res);
	glUniform4fv(spot_dir_loc1, 1, res);

	glUniform1f(spot_angle_loc0, spotLightAngle[0]);
	glUniform1f(spot_angle_loc1, spotLightAngle[1]);

	glUniform1i(spot_toggle, spotON);

	//pass the toggle for fog
	glUniform1i(fog_toggle, fogON);
}

void aiRecursive_render(const aiNode* nd, vector<struct MyMesh>& myMeshes, GLuint*& textureIds)
{
	GLint loc;

	// Get node transformation matrix
	aiMatrix4x4 m = nd->mTransformation;
	// OpenGL matrices are column major
	m.Transpose();

	// save model matrix and apply node transformation
	pushMatrix(MODEL);

	float aux[16];
	memcpy(aux, &m, sizeof(float) * 16);
	multMatrix(MODEL, aux);


	// draw all meshes assigned to this node
	for (unsigned int n = 0; n < nd->mNumMeshes; ++n) {

		// send the material
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, myMeshes[nd->mMeshes[n]].mat.ambient);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, myMeshes[nd->mMeshes[n]].mat.diffuse);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, myMeshes[nd->mMeshes[n]].mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.emissive");
		glUniform4fv(loc, 1, myMeshes[nd->mMeshes[n]].mat.emissive);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, myMeshes[nd->mMeshes[n]].mat.shininess);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.texCount");
		glUniform1i(loc, myMeshes[nd->mMeshes[n]].mat.texCount);

		unsigned int  diffMapCount = 0;  //read 2 diffuse textures

		//devido ao fragment shader suporta 2 texturas difusas simultaneas, 1 especular e 1 normal map

		glUniform1i(normalMap_loc, false);   //GLSL normalMap variable initialized to 0
		glUniform1i(specularMap_loc, false);
		glUniform1ui(diffMapCount_loc, 0);

		if (myMeshes[nd->mMeshes[n]].mat.texCount != 0)
			for (unsigned int i = 0; i < myMeshes[nd->mMeshes[n]].mat.texCount; ++i) {

				//Activate a TU with a Texture Object
				GLuint TU = myMeshes[nd->mMeshes[n]].texUnits[i];
				glActiveTexture(GL_TEXTURE1 + TU);
				glBindTexture(GL_TEXTURE_2D, textureIds[TU]);

				if (myMeshes[nd->mMeshes[n]].texTypes[i] == DIFFUSE) {
					if (diffMapCount == 0) {
						diffMapCount++;
						loc = glGetUniformLocation(shader.getProgramIndex(), "texUnitDiff");
						glUniform1i(loc, TU);
						glUniform1ui(diffMapCount_loc, diffMapCount);
					}
					else if (diffMapCount == 1) {
						diffMapCount++;
						loc = glGetUniformLocation(shader.getProgramIndex(), "texUnitDiff1");
						glUniform1i(loc, TU);
						glUniform1ui(diffMapCount_loc, diffMapCount);
					}
					else printf("Only supports a Material with a maximum of 2 diffuse textures\n");
				}
				else if (myMeshes[nd->mMeshes[n]].texTypes[i] == SPECULAR) {
					loc = glGetUniformLocation(shader.getProgramIndex(), "texUnitSpec");
					glUniform1i(loc, TU);
					glUniform1i(specularMap_loc, true);
				}
				else if (myMeshes[nd->mMeshes[n]].texTypes[i] == NORMALS) { //Normal map
					loc = glGetUniformLocation(shader.getProgramIndex(), "texUnitNormalMap");
					if (normalMapKey)
						glUniform1i(normalMap_loc, normalMapKey);
					glUniform1i(loc, TU);

				}
				else printf("Texture Map not supported\n");
			}

		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// bind VAO
		glBindVertexArray(myMeshes[nd->mMeshes[n]].vao);

		if (!shader.isProgramValid()) {
			printf("Program Not Valid!\n");
			exit(1);
		}
		// draw
		glDrawElements(myMeshes[nd->mMeshes[n]].type, myMeshes[nd->mMeshes[n]].numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	// draw all children
	for (unsigned int n = 0; n < nd->mNumChildren; ++n) {
		aiRecursive_render(nd->mChildren[n], myMeshes, textureIds);

	}

	popMatrix(MODEL);
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
	loc = glGetUniformLocation(shader.getProgramIndex(), "mat.texCount");
	glUniform1f(loc, mat.texCount);
}

static void renderFloats() {
	pushMatrix(MODEL);

	for (int i = 0; i < 14; ++i) {
		// send the material
		sendMaterial(floatMeshes[i].mat);
		pushMatrix(MODEL);

		// 1.0f radius
		if (i == 0 || i == 1) {
			translate(MODEL, floats[0].pos[0], floats[0].pos[1], floats[0].pos[2]);
			pointLightPos[0][0] = floats[0].pos[0];
			pointLightPos[0][1] = floats[0].pos[1] + 6;
			pointLightPos[0][2] = floats[0].pos[2];
		}
		if (i == 2 || i == 3) {
			translate(MODEL, floats[1].pos[0], floats[1].pos[1], floats[1].pos[2]);
			pointLightPos[1][0] = floats[1].pos[0];
			pointLightPos[1][1] = floats[1].pos[1] + 6;
			pointLightPos[1][2] = floats[1].pos[2];
		}
		if (i == 4 || i == 5) {
			translate(MODEL, floats[2].pos[0], floats[2].pos[1], floats[2].pos[2]);
			pointLightPos[2][0] = floats[2].pos[0];
			pointLightPos[2][1] = floats[2].pos[1] + 6;
			pointLightPos[2][2] = floats[2].pos[2];
		}
		if (i == 6 || i == 7) {
			translate(MODEL, floats[3].pos[0], floats[3].pos[1], floats[3].pos[2]);
			pointLightPos[3][0] = floats[3].pos[0];
			pointLightPos[3][1] = floats[3].pos[1] + 6;
			pointLightPos[3][2] = floats[3].pos[2];
		}
		if (i == 8 || i == 9) {
			translate(MODEL, floats[4].pos[0], floats[4].pos[1], floats[4].pos[2]);
			pointLightPos[4][0] = floats[4].pos[0];
			pointLightPos[4][1] = floats[4].pos[1] + 6;
			pointLightPos[4][2] = floats[4].pos[2];
		}
		if (i == 10 || i == 11) {
			translate(MODEL, floats[5].pos[0], floats[5].pos[1], floats[5].pos[2]);
			pointLightPos[5][0] = floats[5].pos[0];
			pointLightPos[5][1] = floats[5].pos[1] + 6;
			pointLightPos[5][2] = floats[5].pos[2];
		}
		if (i == 12 || i == 13) {
			translate(MODEL, floats[6].pos[0], floats[6].pos[1], floats[6].pos[2]);
			floats[6].yellow = true;
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

void renderTree() {
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
			translate(MODEL, 45.0f, 10.5f, 53.0f);
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
			translate(MODEL, 48.0f, 9.0f, 44.0f);
			scale(MODEL, 0.5, 0.5, 0.5);
		}
		if (i == 5) { //tree top big island
			translate(MODEL, 48.0f, 10.0f, 44.0f);
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

	float pos[3], right[3], up[3];
	GLint loc;
	float x = 0.0f, y = 1.75f, z = 10.0f;
	float cam[3] = { cams[activeCamera].getPos()[0], cams[activeCamera].getPos()[1], cams[activeCamera].getPos()[2]};

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if (bumpmapping) {
		glUniform1i(texMode_uniformId, 5);
	}
	else {
		glUniform1i(texMode_uniformId, 2); // draw textured quads
	}

	for (int i = -4; i < 4; i++) {
		for (int j = -4; j < 4; j++) {
			pushMatrix(MODEL);
			translate(MODEL, 5 + i * 20.0, -3, 5 + j * 20.0);

			pos[0] = 5 + i * 10.0; pos[1] = 0; pos[2] = 5 + j * 10.0;

			if (type == 2)
				l3dBillboardSphericalBegin(cam, pos);
			else if (type == 3)
				l3dBillboardCylindricalBegin(cam, pos);

			int objId = 0;  //quad for tree

			//diffuse and ambient color are not used in the tree quads
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
			glUniform4fv(loc, 1, mesh[objId].mat.specular);
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
			glUniform1f(loc, mesh[objId].mat.shininess);

			pushMatrix(MODEL);
			translate(MODEL, 0.0, 3.0, 0.0f);

			// send matrices to OGL
			if (type == 0 || type == 1) {     //Cheating matrix reset billboard techniques
				computeDerivedMatrix(VIEW_MODEL);

				//reset VIEW_MODEL
				if (type == 0) BillboardCheatSphericalBegin();
				else BillboardCheatCylindricalBegin();

				computeDerivedMatrix_PVM(); // calculate PROJ_VIEW_MODEL
			}
			else computeDerivedMatrix(PROJ_VIEW_MODEL);

			glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
			glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
			computeNormalMatrix3x3();
			glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);
			glBindVertexArray(mesh[objId].vao);
			glDrawElements(mesh[objId].type, mesh[objId].numIndexes, GL_UNSIGNED_INT, 0);
			popMatrix(MODEL);

			popMatrix(MODEL);
		}
	}
	glUniform1i(texMode_uniformId, 0); // draw textured quads
}

static void renderBoat() {
	loadIdentity(MODEL);

	pushMatrix(MODEL);

	translate(MODEL, boat.pos[0], boat.pos[1], boat.pos[2]);
	rotate(MODEL, boat.angle, 0, 1.0f, 0);


	for (int i = 0; i < 2; i++) {
		spotLightPos[i][0] = boat.pos[0] + 0.5;
		if (i == 1) {
			spotLightPos[i][1] = boat.pos[1] + 1.0;
		}
		else {
			spotLightPos[i][1] = boat.pos[1] + 1.5;
		}
		spotLightPos[i][2] = boat.pos[2];
		spotLightDir[i][0] = -boat.dir[0];
		spotLightDir[i][1] = -boat.dir[1];
		spotLightDir[i][2] = -boat.dir[2];
	}


	
	
	for (int i = 0; i < 5; ++i) {
		// send the material
		sendMaterial(boatMeshes[i].mat);
		pushMatrix(MODEL);

		if (i == 0) { // pawn
			translate(MODEL, 0.0f, 0.5f, 1.0f);
			scale(MODEL, 0.25f, 0.25, 0.25);
		}
		if (i == 1) { //base
			translate(MODEL, -0.5f, 0.0f, -0.5f);
			scale(MODEL, 1.0, 0.5, 3.0);
		}
		if (i == 2) { // front
			translate(MODEL, 0.0f, 0.0f, 2.5f);
			scale(MODEL, 0.25f, 0.25f, 0.25f);
			rotate(MODEL, 90, 1, 0,0);
			rotate(MODEL, 45, 0, 1, 0);
		}
		if (i == 3) { //oars
			translate(MODEL, 1.0f, 0.0f, 1.5f);
			rotate(MODEL, 45, 0, 0, 1);
			
			if (leftoar == 1) {
				leftang += 5.0f;
				if (sKey == 1) {
					rotate(MODEL, leftang, 0.1, 0.5, 0);
				}
				else {
					rotate(MODEL, leftang, -0.1, -0.5, 0);
				}
				if (leftang > 360.0f) {
					leftang = 0.0f;
				}
			}
			scale(MODEL, 0.1f, 1.0f, 0.1f);

		}
		if (i == 4) { //oars
			
			translate(MODEL, -1.0f, 0.0f, 1.5f);
			rotate(MODEL, -45, 0, 0, 1);
			if (rightoar == 1) {
				rightang += 5.0f;
				if (sKey == 1) {
					rotate(MODEL, rightang, -0.1, -0.5, 0);
				}
				else {
					rotate(MODEL, rightang, 0.1, 0.5, 0);
				}
				if (rightang > 360.0f) {
					rightang = 0.0f;
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

	pushMatrix(MODEL);
	translate(MODEL, -0.125, 0.6, 0.3);
	scale(MODEL, scaleFactor1, scaleFactor1, scaleFactor1);
	translate(MODEL, 0.5, 1.0, 0.0);
	rotate(MODEL, 180, 0, 1, 0);
	aiRecursive_render(scene1->mRootNode, myMeshes1, textureIds1);
	glUniform1i(diffMapCount_loc, 0);
	popMatrix(MODEL);

	popMatrix(MODEL);
}

static void renderWater(void) {
	glDepthMask(GL_FALSE);
	//myMeshes[0].mat.texCount = 0;

	sendMaterial(myMeshes[0].mat);
	pushMatrix(MODEL);
	rotate(MODEL, -90, 1, 0, 0);
	glUniform1i(texMode_uniformId, 1);

	// send matrices to OGL
	computeDerivedMatrix(PROJ_VIEW_MODEL);
	glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
	glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
	computeNormalMatrix3x3();
	glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

	// Render mesh
	glBindVertexArray(myMeshes[0].vao);

	glDrawElements(myMeshes[0].type, myMeshes[0].numIndexes, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glDepthMask(GL_TRUE);

	glUniform1i(texMode_uniformId, 0);
	popMatrix(MODEL);
}

void render_flare(FLARE_DEF* flare, int lx, int ly, int* m_viewport) {  //lx, ly represent the projected position of light on viewport

	int     dx, dy;          // Screen coordinates of "destination"
	int     px, py;          // Screen coordinates of flare element
	int		cx, cy;
	float    maxflaredist, flaredist, flaremaxsize, flarescale, scaleDistance;
	int     width, height, alpha;    // Piece parameters;
	int     i;
	float	diffuse[4];

	GLint loc;

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	int screenMaxCoordX = m_viewport[0] + m_viewport[2] - 1;
	int screenMaxCoordY = m_viewport[1] + m_viewport[3] - 1;

	//viewport center
	cx = m_viewport[0] + (int)(0.5f * (float)m_viewport[2]) - 1;
	cy = m_viewport[1] + (int)(0.5f * (float)m_viewport[3]) - 1;

	// Compute how far off-center the flare source is.
	maxflaredist = sqrt(cx * cx + cy * cy);
	flaredist = sqrt((lx - cx) * (lx - cx) + (ly - cy) * (ly - cy));
	scaleDistance = (maxflaredist - flaredist) / maxflaredist;
	flaremaxsize = (int)(m_viewport[2] * flare->fMaxSize);
	flarescale = (int)(m_viewport[2] * flare->fScale);

	// Destination is opposite side of centre from source
	dx = clampi(cx + (cx - lx), m_viewport[0], screenMaxCoordX);
	dy = clampi(cy + (cy - ly), m_viewport[1], screenMaxCoordY);

	// Render each element. To be used Texture Unit 0

	glUniform1i(texMode_uniformId, 3); // draw modulated textured particles 
	glUniform1i(tex_loc5, 0);  //use TU 0

	for (i = 0; i < flare->nPieces; ++i)
	{
		// Position is interpolated along line between start and destination.
		px = (int)((1.0f - flare->element[i].fDistance) * lx + flare->element[i].fDistance * dx);
		py = (int)((1.0f - flare->element[i].fDistance) * ly + flare->element[i].fDistance * dy);
		px = clampi(px, m_viewport[0], screenMaxCoordX);
		py = clampi(py, m_viewport[1], screenMaxCoordY);

		// Piece size are 0 to 1; flare size is proportion of screen width; scale by flaredist/maxflaredist.
		width = (int)(scaleDistance * flarescale * flare->element[i].fSize);
		// Width gets clamped, to allows the off-axis flaresto keep a good size without letting the elements get big when centered.
		if (width > flaremaxsize)  width = flaremaxsize;

		height = (int)((float)m_viewport[3] / (float)m_viewport[2] * (float)width);
		memcpy(diffuse, flare->element[i].matDiffuse, 4 * sizeof(float));
		diffuse[3] *= scaleDistance;   //scale the alpha channel
		// printf("%d: %d %d %d %d %f\n", i, px, py, width, height, diffuse[3]);

		if (width > 1)
		{
			// send the material - diffuse color modulated with texture
			loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
			glUniform4fv(loc, 1, diffuse);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, FlareTextureArray[flare->element[i].textureId]);
			sendMaterial(flareMeshes[0].mat);
			pushMatrix(MODEL);
			translate(MODEL, (float)(px - width * 0.0f), (float)(py - height * 0.0f), 0.0f);
			scale(MODEL, (float)width, (float)height, 1);
			computeDerivedMatrix(PROJ_VIEW_MODEL);
			glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
			glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
			computeNormalMatrix3x3();
			glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

			glBindVertexArray(flareMeshes[0].vao);
			glDrawElements(flareMeshes[0].type, flareMeshes[0].numIndexes, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
			popMatrix(MODEL);
		}
	}
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}



float dotProduct(const float* vec1, const float* vec2) {
	return vec1[0] * vec2[0] + vec1[1] * vec2[1] + vec1[2] * vec2[2];
}
float magnitude(const float* vec) {
	return std::sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
}
float angleBetweenVectors(const float* vec1, const float* vec2) {
	float dot = dotProduct(vec1, vec2);
	float mag1 = magnitude(vec1);
	float mag2 = magnitude(vec2);
	return std::acos(dot / (mag1 * mag2));
}

static void renderSkybox(void) {
	glUniform1i(texMode_uniformId, 6);

	glDepthMask(GL_FALSE);
	glFrontFace(GL_CW);

	pushMatrix(MODEL);
	pushMatrix(VIEW);  //se quiser anular a translaÁ„o

	//  Fica mais realista se n„o anular a translaÁ„o da c‚mara 
	// Cancel the translation movement of the camera - de acordo com o tutorial do Antons
	mMatrix[VIEW][12] = 0.0f;
	mMatrix[VIEW][13] = 0.0f;
	mMatrix[VIEW][14] = 0.0f;

	scale(MODEL, 100.0f, 100.0f, 100.0f);
	translate(MODEL, -0.5f, -0.5f, -0.5f);

	// send matrices to OGL
	glUniformMatrix4fv(model_uniformId, 1, GL_FALSE, mMatrix[MODEL]); //TransformaÁ„o de modelaÁ„o do cubo unit·rio para o "Big Cube"
	computeDerivedMatrix(PROJ_VIEW_MODEL);
	glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);

	glBindVertexArray(skyMesh[0].vao);
	glDrawElements(skyMesh[0].type, skyMesh[0].numIndexes, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	popMatrix(MODEL);
	popMatrix(VIEW);

	glFrontFace(GL_CCW); // restore counter clockwise vertex order to mean the 
	glDepthMask(GL_TRUE);
	glUniform1i(texMode_uniformId, 0);

}

static void renderScene(void) {	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	setupRender();

	renderSkybox();
	renderTree();
	renderFloats();
	renderCreatures();
	renderBoat();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TextureArray[0]);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, TextureArray[1]);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, TextureArray[2]);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, TextureArray[3]);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, TextureArray[4]);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, TextureArray[5]);

	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_CUBE_MAP, TextureArray[6]);
	

	//Indicar aos tres samplers do GLSL quais os Texture Units a serem usados
	glUniform1i(tex_loc0, 0);
	glUniform1i(tex_loc1, 1);
	glUniform1i(tex_loc2, 2);
	glUniform1i(tex_loc3, 3);
	glUniform1i(tex_loc4, 4);
	glUniform1i(tex_normalMap_loc, 5);
	glUniform1i(tex_cube_loc, 6);


	float particle_color[4];
	GLint loc;

	for (int j = 1; j < 4; ++j) {
		// send the material
		sendMaterial(myMeshes[j].mat);
		pushMatrix(MODEL);

		if (j == 1) { // big island
			translate(MODEL, 50.0f, 0.0f, 50.0f);
			scale(MODEL, 1.5, 1, 1.5);
			islandBoundingSpheres[0] = BoundingSphere(50.0f, 0.0f, 50.0f, 15.0f);
		}
		else if (j == 2) { //medium island #1
			translate(MODEL, 60.0f, 0.0f, -45.0f);
			islandBoundingSpheres[1] = BoundingSphere(60.0f, 0.0f, -45.0f, 8.0f);
		}
		else if (j == 3) { //medium island #2
			glUniform1i(texMode_uniformId, 6); //cube mapping
			translate(MODEL, -50.0f, 0.0f, 0.0f);
			islandBoundingSpheres[2] = BoundingSphere(-50.0f, 0.0f, 0.0f, 8.0f);
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

		glUniform1i(texMode_uniformId, 0);
		popMatrix(MODEL);
	}

	renderWater();

	if (fireworks) {

		updateParticles();

		// draw fireworks particles

		glBindTexture(GL_TEXTURE_2D, TextureArray[4]); //particle.tga associated to TU0 

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glDepthMask(GL_FALSE);  //Depth Buffer Read Only

		glUniform1i(texMode_uniformId, 4); // draw modulated textured particles 

		for (int i = 0; i < MAX_PARTICULAS; i++)
		{
			if (particula[i].life > 0.0f) /* s√≥ desenha as que ainda est√£o vivas */
			{

				/* A vida da part√≠cula representa o canal alpha da cor. Como o blend est√° activo a cor final √© a soma da cor rgb do fragmento multiplicada pelo
				alpha com a cor do pixel destino */

				particle_color[0] = particula[i].r;
				particle_color[1] = particula[i].g;
				particle_color[2] = particula[i].b;
				particle_color[3] = particula[i].life;

				// send the material - diffuse color modulated with texture
				loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
				glUniform4fv(loc, 1, particle_color);

				pushMatrix(MODEL);
				translate(MODEL, particula[i].x, particula[i].y, particula[i].z);
				if (floats[6].pos[0] < 0.0f) {
					rotate(MODEL, 90, 0.0f, 1.0f, 0.0f);
				}

				// send matrices to OGL
				computeDerivedMatrix(PROJ_VIEW_MODEL);
				glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
				glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
				computeNormalMatrix3x3();
				glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

				glBindVertexArray(particleMeshes[0].vao);
				glDrawElements(particleMeshes[0].type, particleMeshes[0].numIndexes, GL_UNSIGNED_INT, 0);
				popMatrix(MODEL);
			}
			else dead_num_particles++;
		}

		glDepthMask(GL_TRUE); //make depth buffer again writeable

		if (dead_num_particles == MAX_PARTICULAS) {
			fireworks = 0;
			dead_num_particles = 0;
			goal_flag = true;
			printf("All particles dead\n");
		}

	}



	int flarePos[2];
	int m_viewport[4];
	glGetIntegerv(GL_VIEWPORT, m_viewport);

	pushMatrix(MODEL);
	loadIdentity(MODEL);
	computeDerivedMatrix(PROJ_VIEW_MODEL);  //pvm to be applied to lightPost. pvm is used in project function

	if (!project(pointLightPos[0], lightScreenPos, m_viewport))
		printf("Error in getting projected light in screen\n");  //Calculate the window Coordinates of the light position: the projected position of light on viewport
	flarePos[0] = clampi((int)lightScreenPos[0], m_viewport[0], m_viewport[0] + m_viewport[2] - 1);
	flarePos[1] = clampi((int)lightScreenPos[1], m_viewport[1], m_viewport[1] + m_viewport[3] - 1);
	popMatrix(MODEL);

	//viewer looking down at  negative z direction
	pushMatrix(PROJECTION);
	loadIdentity(PROJECTION);
	pushMatrix(VIEW);
	loadIdentity(VIEW);
	ortho(m_viewport[0], m_viewport[0] + m_viewport[2] - 1, m_viewport[1], m_viewport[1] + m_viewport[3] - 1, -1, 1);

	float camDir[3] = { cams[activeCamera].getTarget()[0] - cams[activeCamera].getPos()[0], cams[activeCamera].getTarget()[1] - cams[activeCamera].getPos()[1], cams[activeCamera].getTarget()[2] - cams[activeCamera].getPos()[2] };
	float lightDir[3] = { pointLightPos[0][0] - cams[activeCamera].getPos()[0], pointLightPos[0][1] - cams[activeCamera].getPos()[1], pointLightPos[0][2] - cams[activeCamera].getPos()[2] };
	float angle = angleBetweenVectors(camDir, lightDir);
	if (pointON && !spotON && angle < M_PI / 2) {
		render_flare(&AVTflare, flarePos[0], flarePos[1], m_viewport);
	}
	popMatrix(PROJECTION);
	popMatrix(VIEW);

	



	//Render text (bitmap fonts) in screen coordinates. So use ortoghonal projection with viewport coordinates.
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//the glyph contains transparent background colors and non-transparent for the actual character pixels. So we use the blending


	pushMatrix(PROJECTION);
	loadIdentity(PROJECTION);
	pushMatrix(VIEW);
	loadIdentity(VIEW);
	ortho(m_viewport[0], m_viewport[0] + m_viewport[2] - 1, m_viewport[1], m_viewport[1] + m_viewport[3] - 1, -1, 1);
	RenderText(shaderText, "Lives: " + std::to_string(lives), livesX, livesY, 0.8f, 1.0f, 1.0f, 1.0f);
	RenderText(shaderText, "Time: " + elapsedTime.formatTime(), timeX, timeY, 0.8f, 1.0f, 1.0f, 1.0f);
	if (paused) {
		RenderText(shaderText, "Pause! Press 'P' to unpause.", 500.0f, 300.0f, 0.8f, 1.0f, 1.0f, 1.0f);
	}
	if (lives == 0) {
		RenderText(shaderText, "Game Over! Press 'R' to restart.", 100.0f, 100.0f, 0.8f, 1.0f, 1.0f, 1.0f);
		restart = true;
	}
	if (goal_flag) {
		RenderText(shaderText, "You won! Press 'R' to try again!", 100.0f, 100.0f, 0.8f, 1.0f, 1.0f, 1.0f);
		restart = true;
	}
	popMatrix(PROJECTION);
	popMatrix(VIEW);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	




	


	glBindTexture(GL_TEXTURE_2D, 0);
	glutSwapBuffers();
}

// ------------------------------------------------------------
//
// Events from the Keyboard
//

void onKeyUp(unsigned char key, int xx, int yy) {
	switch (key) {
	case 'A': case 'a':
		aKey = 0;
		rightoar = 0;
		break;
	case 'D': case 'd':
		dKey = 0;
		leftoar = 0;
		break;
	}
}

void processKeys(unsigned char key, int xx, int yy)
{
	// These keys are valid anytime
	if (key == 'r' || key == 'R') {
		restartGame();
		restart = false;
		goal_flag = false;
	}

	if (restart) return;

	if (key == 'p' || key == 'P') {
		paused = !paused;
	}

	if (paused) return;

	// From here, only when the game is unpaused we will handle the keypress

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
		aKey = 1;
		if (dKey == 0) {
			boat.rotate(-2.0f); // Rotate by -2 degrees
		}

		boat.accelerate();
		rightoar = 1;
		break;
	case 'D': case 'd':
		// left paddle stroke rotates the boat slightly to the right
		dKey = 1;
		if (aKey == 0) {
			boat.rotate(2.0f); // Rotate by 2 degrees
		}

		boat.accelerate();
		leftoar = 1;
		break;
	case 'S': case 's':
		// Invert the paddle direction
		boat.directionModifier *= -1;
		if (sKey == 0) {
			sKey = 1;
		}
		else {
			sKey = 0;
		}
		break;
	case 'O': case 'o':
		// Increase paddle strength
		boat.addPaddleStrength(5.0f); // Increase by 5.0f
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
	case 'H': case 'h':
		//Toggle spot lights

		if (spotON == 1) {
			spotON = 0;
		}
		else {
			spotON = 1;
		}
		break;
	case 'F': case 'f':
		//Toggle fog effect
		if (fogON == 1) {
			fogON = 0;
		}
		else {
			fogON = 1;
		}
		break;
	case 'B': case 'b':
		//Toggle bump mapping
		if (bumpmapping == 1) {
			bumpmapping = 0;
		}
		else {
			bumpmapping = 1;
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

void processMouseMotion(int xx, int yy) {
	int deltaX, deltaY;

	deltaX = xx - startX;
	deltaY = yy - startY;

	if (tracking == 1) {  // If left mouse button is being tracked
		if (activeCamera == 2) {
			// Update alpha (horizontal) and beta (vertical) with mouse movement
			cams[activeCamera].setAlpha(cams[activeCamera].getAlpha() + deltaX * 0.1f);
			cams[activeCamera].setBeta(cams[activeCamera].getBeta() + deltaY * 0.1f);

			// Constrain beta so the camera doesn't flip upside down
			if (cams[activeCamera].getBeta() > 85.0f)
				cams[activeCamera].setBeta(85.0f);
			else if (cams[activeCamera].getBeta() < -85.0f)
				cams[activeCamera].setBeta(-85.0f);
		}

		startX = xx;
		startY = yy;
	}
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
	glBindAttribLocation(shader.getProgramIndex(), TEXTURE_COORD_ATTRIB, "texCoord");
	glBindAttribLocation(shader.getProgramIndex(), TANGENT_ATTRIB, "tangent");
	glBindAttribLocation(shader.getProgramIndex(), BITANGENT_ATTRIB, "bitangent");

	glLinkProgram(shader.getProgramIndex());
	printf("InfoLog for Model Rendering Shader\n%s\n\n", shaderText.getAllInfoLogs().c_str());

	if (!shader.isProgramValid()) {
		printf("GLSL Model Program Not Valid!\n");
		printf("InfoLog for Per Fragment Phong Lightning Shader\n%s\n\n", shader.getAllInfoLogs().c_str());
		//exit(1);
	}

	pvm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_pvm");
	vm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_viewModel");
	normal_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_normal");
	//lPos_uniformId = glGetUniformLocation(shader.getProgramIndex(), "l_pos");
	point_loc0 = glGetUniformLocation(shader.getProgramIndex(),"pointLights[0].position");
	point_loc1 = glGetUniformLocation(shader.getProgramIndex(),"pointLights[1].position");
	point_loc2 = glGetUniformLocation(shader.getProgramIndex(), "pointLights[2].position");
	point_loc3 = glGetUniformLocation(shader.getProgramIndex(), "pointLights[3].position");
	point_loc4 = glGetUniformLocation(shader.getProgramIndex(), "pointLights[4].position");
	point_loc5 = glGetUniformLocation(shader.getProgramIndex(), "pointLights[5].position");
	dir_loc = glGetUniformLocation(shader.getProgramIndex(), "dirLight.direction");
	spot_pos_loc0 = glGetUniformLocation(shader.getProgramIndex(),"spotLights[0].position");
	spot_pos_loc1 = glGetUniformLocation(shader.getProgramIndex(),"spotLights[1].position");
	spot_dir_loc0 = glGetUniformLocation(shader.getProgramIndex(), "spotLights[0].direction");
	spot_dir_loc1 = glGetUniformLocation(shader.getProgramIndex(), "spotLights[1].direction");
	spot_angle_loc0 = glGetUniformLocation(shader.getProgramIndex(), "spotLights[0].angle");
	spot_angle_loc1 = glGetUniformLocation(shader.getProgramIndex(), "spotLights[1].angle");
	tex_loc0 = glGetUniformLocation(shader.getProgramIndex(), "texmap0");
	tex_loc1 = glGetUniformLocation(shader.getProgramIndex(), "texmap1");
	tex_loc2 = glGetUniformLocation(shader.getProgramIndex(), "texmap2");
	tex_loc3 = glGetUniformLocation(shader.getProgramIndex(), "texmap3");
	tex_loc4 = glGetUniformLocation(shader.getProgramIndex(), "texmap4");
	tex_loc5 = glGetUniformLocation(shader.getProgramIndex(), "texmap5");
	tex_normalMap_loc = glGetUniformLocation(shader.getProgramIndex(), "normalMap2");
	tex_cube_loc = glGetUniformLocation(shader.getProgramIndex(), "cubeMap");
	normalMap_loc = glGetUniformLocation(shader.getProgramIndex(), "normalMap");
	specularMap_loc = glGetUniformLocation(shader.getProgramIndex(), "specularMap");
	diffMapCount_loc = glGetUniformLocation(shader.getProgramIndex(), "diffMapCount");
	texMode_uniformId = glGetUniformLocation(shader.getProgramIndex(), "texMode");
	model_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_Model");
	view_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_View");

	point_toggle = glGetUniformLocation(shader.getProgramIndex(), "pointON");
	dir_toggle = glGetUniformLocation(shader.getProgramIndex(), "dirON");
	spot_toggle = glGetUniformLocation(shader.getProgramIndex(), "spotON");
	fog_toggle = glGetUniformLocation(shader.getProgramIndex(), "fogON");

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
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	MyMesh amesh;

	/* Initialization of DevIL */
	if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION)
	{
		printf("wrong DevIL version \n");
		exit(0);
	}
	ilInit();

	//Texture Object definition

	glGenTextures(6, TextureArray);
	Texture2D_Loader(TextureArray, "stone.tga", 0);
	Texture2D_Loader(TextureArray, "water_quad.png", 1);
	Texture2D_Loader(TextureArray, "lightwood.tga", 2);
	Texture2D_Loader(TextureArray, "tree.tga", 3);
	Texture2D_Loader(TextureArray, "particle.tga", 4);
	Texture2D_Loader(TextureArray, "normal.tga", 5);


	//Flare elements textures
	glGenTextures(5, FlareTextureArray);
	Texture2D_Loader(FlareTextureArray, "crcl.tga", 0);
	Texture2D_Loader(FlareTextureArray, "flar.tga", 1);
	Texture2D_Loader(FlareTextureArray, "hxgn.tga", 2);
	Texture2D_Loader(FlareTextureArray, "ring.tga", 3);
	Texture2D_Loader(FlareTextureArray, "sun.tga", 4);

	const char* filenames[] = { "posx.jpg", "negx.jpg", "posy.jpg", "negy.jpg", "posz.jpg", "negz.jpg" };

	TextureCubeMap_Loader(TextureArray, filenames, 5);

	/// Initialization of freetype library with font_name file
	freeType_init(font_name);

	// set the camera position based on its spherical coordinates
	/*camX = r * sin(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camZ = r * cos(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camY = r * sin(beta * 3.14f / 180.0f);*/

	std::string filepath1 = "backpack/backpack.obj";

	//Import the model obj, a backpack
	if (!Import3DFromFile(filepath1, importer1, scene1, scaleFactor1)) {
		exit(0);
	}
	strcpy(model_dir, "backpack/");
	//creation of Mymesh array with VAO Geometry and Material and array of Texture Objs for the backpack model
	myMeshes1 = createMeshFromAssimp(scene1, textureIds1);

	cams[0].setPos({ 0, 200, 0 });
	cams[0].setUp({ 0, 0, 1 });

	cams[1].setPos({ 0, 150, 0 });	
	cams[1].setUp({ 0, 0, 1 });
	cams[1].setType(1);

	cams[2].setPos({ 4, 16, 4 });

	

	float h20_amb[] = { 0.0f, 0.0f, 0.25f, 0.7f };
	float h20_diff[] = { 0.1f, 0.1f, 0.8f, 0.7f };
	float h20_spec[] = { 0.9f, 0.9f, 0.9f, 0.7f };
	float emissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininess = 3000.0f;
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

		floats[i] = Redfloat();
	}

	floats[0].pos = { 19.0f, 0.2f, 75.0f };
	floats[1].pos = { -49.0f, 0.2f, -49.0f };
	floats[2].pos = { 50.0f, 0.2f, 0.0f };
	floats[3].pos = { -20.0f, 0.2f, 18.0f };
	floats[4].pos = { 10.0f, 0.2f, -55.0f };
	floats[5].pos = { -72.0f, 0.2f, 72.0f };

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

	//tree specular color
	float tree_spec[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	float tree_shininess = 10.0f;

	//billboard trees
	amesh = createQuad(6, 6);
	memcpy(amesh.mat.specular, tree_spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = tree_shininess;
	amesh.mat.texCount = texcount;
	mesh.push_back(amesh);

	float goal_amb[] = { 0.1f, 0.1f, 0.0f, 1.0f };
	float goal_diff[] = { 1.0f, 1.0f, 0.0f, 1.0f };
	float goal_spec[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	amesh = createCylinder(6.0f, 0.5f, 50);
	memcpy(amesh.mat.ambient, goal_amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, goal_diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, goal_spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	floatMeshes.push_back(amesh);


	amesh = createSphere(1.0f, 10);
	memcpy(amesh.mat.ambient, goal_amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, goal_diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, goal_spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	floatMeshes.push_back(amesh);

	floats[numOfFloats] = Redfloat();

	// create geometry and VAO of the quad for flare elements
	float flare_amb[] = { 0.75f, 0.75f, 0.75f, 1.0f };  
	float flare_diff[] = { 0.75f, 0.75f, 0.75f, 1.0f }; 
	float flare_spec[] = { 0.9f, 0.9f, 0.9f, 1.0f };

	amesh = createQuad(1, 1);
	memcpy(amesh.mat.ambient, flare_amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, flare_diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, flare_spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = shininess;
	amesh.mat.texCount = texcount;
	flareMeshes.push_back(amesh);

	//skybox

	float sky_amb[] = { 0.2f, 0.15f, 0.1f, 1.0f };
	float sky_diff[] = { 0.8f, 0.6f, 0.4f, 1.0f };
	float sky_spec[] = { 0.8f, 0.8f, 0.8f, 1.0f };

	amesh = createCube();
	memcpy(amesh.mat.ambient, sky_amb, 4 * sizeof(float));
	memcpy(amesh.mat.diffuse, sky_diff, 4 * sizeof(float));
	memcpy(amesh.mat.specular, sky_spec, 4 * sizeof(float));
	memcpy(amesh.mat.emissive, emissive, 4 * sizeof(float));
	amesh.mat.shininess = 100;
	amesh.mat.texCount = texcount;
	skyMesh.push_back(amesh);

	//Load flare from file
	loadFlareFile(&AVTflare, "flare.txt");

	// create geometry and VAO of the quad for particles
	amesh = createQuad(2, 2);
	amesh.mat.texCount = texcount;
	particleMeshes.push_back(amesh);



	goal_position();

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
	glutKeyboardUpFunc(onKeyUp);
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

unsigned int getTextureId(char* name) {
	int i;

	for (i = 0; i < NTEXTURES; ++i)
	{
		if (strncmp(name, flareTextureNames[i], strlen(name)) == 0)
			return i;
	}
	return -1;
}
void    loadFlareFile(FLARE_DEF* flare, char* filename)
{
	int     n = 0;
	FILE* f;
	char    buf[256];
	int fields;

	memset(flare, 0, sizeof(FLARE_DEF));

	f = fopen(filename, "r");
	if (f)
	{
		fgets(buf, sizeof(buf), f);
		sscanf(buf, "%f %f", &flare->fScale, &flare->fMaxSize);

		while (!feof(f))
		{
			char            name[8] = { '\0', };
			double          dDist = 0.0, dSize = 0.0;
			float			color[4];
			int				id;

			fgets(buf, sizeof(buf), f);
			fields = sscanf(buf, "%4s %lf %lf ( %f %f %f %f )", name, &dDist, &dSize, &color[3], &color[0], &color[1], &color[2]);
			if (fields == 7)
			{
				for (int i = 0; i < 4; ++i) color[i] = clamp(color[i] / 255.0f, 0.0f, 1.0f);
				id = getTextureId(name);
				if (id < 0) printf("Texture name not recognized\n");
				else
					flare->element[n].textureId = id;
				flare->element[n].fDistance = (float)dDist;
				flare->element[n].fSize = (float)dSize;
				memcpy(flare->element[n].matDiffuse, color, 4 * sizeof(float));
				++n;
			}
		}

		flare->nPieces = n;
		fclose(f);
	}
	else printf("Flare file opening error\n");
}