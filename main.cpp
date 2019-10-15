#include <iostream>
#include <string>
#include <stack>
#include <SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "rt3d.h"
#include "rt3dObjLoader.h"

using namespace std;

#define DEG_TO_RADIAN 0.017453293

GLuint meshIndexCount = 0;
GLuint meshObjects[1];

// shaders programs
GLuint skyboxProgram;
GLuint gouraudProgram;
GLuint phongProgram;
GLuint programChoose;

// Textures
GLuint skyboxTexture;
GLuint textures[1];

bool cameraMove = true;

stack<glm::mat4> drawStack;

GLfloat r = 0.0f;

glm::vec3 eye(-2.0f, 1.0f, 8.0f);
glm::vec3 at(0.0f, 1.0f, -1.0f);
glm::vec3 up(0.0f, 1.0f, 0.0f);

rt3d::lightStruct light = {
	{0.4f, 0.4f, 0.4f, 1.0f},
	{1.0f, 1.0f, 1.0f, 1.0f},
	{1.0f, 1.0f, 1.0f, 1.0f},
	{-5.0f, 2.0f, 2.0f, 1.0f}
};
glm::vec4 lightPos(-5.0f, 2.0f, 2.0f, 1.0f);

rt3d::materialStruct materialGroundPlane = {
	{0.2f, 0.4f, 0.2f, 1.0f},
	{0.5f, 1.0f, 0.5f, 1.0f},
	{0.0f, 0.1f, 0.0f, 1.0f},
	2.0f
};

SDL_Window* setupSDL(SDL_GLContext& context) {
	// initialize video
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		cout << "SDL_Init Error: " << SDL_GetError() << endl;
		exit(1);
	}

	// set OpenGL version 3
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	// set double buffer on
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);// antialiasing

	// create window
	SDL_Window* window = SDL_CreateWindow("Class test Project", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (window == nullptr) {
		cout << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
		SDL_Quit();
		exit(1);
	}

	context = SDL_GL_CreateContext(window);// create context

	return window;
}

GLuint loadTexture(const char* fileName) {
	GLuint textureID;
	glGenTextures(1, &textureID);

	// load file - using core SDL library
	SDL_Surface* tmpSurface;
	tmpSurface = SDL_LoadBMP(fileName);
	if (tmpSurface == nullptr) {
		std::cout << "Error loading bitmap" << std::endl;
	}

	// bind texture and set parameters
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tmpSurface->w, tmpSurface->h, 0,
		GL_BGR, GL_UNSIGNED_BYTE, tmpSurface->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);

	SDL_FreeSurface(tmpSurface);
	return textureID;
}

GLuint loadCubeMapTexture(const char *fileName[6], GLuint *textureID) {
	glGenTextures(1, textureID);
	GLenum sides[6] = { GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
						GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 
						GL_TEXTURE_CUBE_MAP_POSITIVE_X, 
						GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 
						GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 
						GL_TEXTURE_CUBE_MAP_NEGATIVE_Y };

	glBindTexture(GL_TEXTURE_CUBE_MAP, *textureID);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	SDL_Surface* tmpSurface;
	for (int i = 0; i < 6; ++i) {
		tmpSurface = SDL_LoadBMP(fileName[i]);
		if (!tmpSurface) {
			cout << "Error loading file" << endl;
			return *textureID;
		}

		SDL_PixelFormat* format = tmpSurface->format;
		GLuint externalFormat = (format->Rmask < format->Bmask) ? GL_RGB : GL_BGR;

		glTexImage2D(sides[i], 0, GL_RGB, tmpSurface->w, tmpSurface->h, 0, externalFormat, GL_UNSIGNED_BYTE, tmpSurface->pixels);
		SDL_FreeSurface(tmpSurface);
	}
	return *textureID;
}

void init() {
	gouraudProgram = rt3d::initShaders("src/gouraudShader.vert", "src/gouraudShader.frag");
	rt3d::setLight(gouraudProgram, light);
	rt3d::setMaterial(gouraudProgram, materialGroundPlane);
	programChoose = gouraudProgram;

	phongProgram = rt3d::initShaders("src/phongShader.vert", "src/phongShader.frag");
	rt3d::setLight(phongProgram, light);
	rt3d::setMaterial(phongProgram, materialGroundPlane);

	skyboxProgram = rt3d::initShaders("src/skyboxTexture.vert", "src/skyboxTexture.frag");

	const char* cubeTexFiles[6] = {
		"src/town-skybox/Town_bk.bmp", "src/town-skybox/Town_ft.bmp", "src/town-skybox/Town_rt.bmp", "src/town-skybox/Town_lf.bmp", "src/town-skybox/Town_up.bmp", "src/town-skybox/Town_dn.bmp"
	};
	loadCubeMapTexture(cubeTexFiles, &skyboxTexture);

	vector<GLfloat> verts;
	vector<GLfloat> norms;
	vector<GLfloat> tex_coords;
	vector<GLuint> indices;
	rt3d::loadObj("src/cube.obj", verts, norms, tex_coords, indices);
	meshIndexCount = indices.size();
	textures[0] = loadTexture("src/fabric.bmp");
	meshObjects[0] = rt3d::createMesh(verts.size() / 3, verts.data(), nullptr, norms.data(), tex_coords.data(), meshIndexCount, indices.data());

	glEnable(GL_DEPTH_TEST);
}

glm::vec3 moveForward(glm::vec3 pos, GLfloat angle, GLfloat d) {
	return glm::vec3(pos.x + d * std::sin(r * DEG_TO_RADIAN), pos.y, pos.z - d * std::cos(r * DEG_TO_RADIAN));
}

glm::vec3 moveRight(glm::vec3 pos, GLfloat angle, GLfloat d) {
	return glm::vec3(pos.x + d * std::cos(r * DEG_TO_RADIAN), pos.y, pos.z + d * std::sin(r * DEG_TO_RADIAN));
}

void update() {
	const Uint8* keys = SDL_GetKeyboardState(NULL);
	if (cameraMove) {
		if (keys[SDL_SCANCODE_W]) eye = moveForward(eye, r, 0.1f);
		if (keys[SDL_SCANCODE_S]) eye = moveForward(eye, r, -0.1f);
		if (keys[SDL_SCANCODE_D]) eye = moveRight(eye, r, 0.1f);
		if (keys[SDL_SCANCODE_A]) eye = moveRight(eye, r, -0.1f);
	}
	else {
		if (keys[SDL_SCANCODE_W]) lightPos[2] -= 0.1;
		if (keys[SDL_SCANCODE_A]) lightPos[0] -= 0.1;
		if (keys[SDL_SCANCODE_S]) lightPos[2] += 0.1;
		if (keys[SDL_SCANCODE_D]) lightPos[0] += 0.1;
	}

	if (keys[SDL_SCANCODE_R]) eye.y += 0.1f;
	if (keys[SDL_SCANCODE_F]) eye.y -= 0.1f;
	if (keys[SDL_SCANCODE_COMMA]) --r;
	if (keys[SDL_SCANCODE_PERIOD]) ++r;

	if (keys[SDL_SCANCODE_1]) {
		cout << "You choose the gouraud shader" << endl;
		programChoose = gouraudProgram;
	}
	if (keys[SDL_SCANCODE_2]) {
		cout << "You choose the phong shader with the matt mode" << endl;
		programChoose = phongProgram;
	}
	if (keys[SDL_SCANCODE_3]) {
		cout << "You choose the phong shader with the shiny mode" << endl;
		programChoose = phongProgram;
	}
	if (keys[SDL_SCANCODE_4]) {
		if (cameraMove) {
			cout << "You can now move the light with AWSD" << endl;
			cameraMove = false;
		}
		else {
			cout << "You can now move the camera with AWSD" << endl;
			cameraMove = true;
		}
	}
}

void draw(SDL_Window* window) {
	glEnable(GL_CULL_FACE);
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 projection = glm::perspective(float(60.0f * DEG_TO_RADIAN), 800.0f / 600.0f, 1.0f, 150.0f);
	rt3d::setUniformMatrix4fv(gouraudProgram, "projection", glm::value_ptr(projection));

	glm::mat4 modelView(1.0);
	drawStack.push(modelView);

	at = moveForward(eye, r, 1.0f);
	drawStack.top() = glm::lookAt(eye, at, up);

	// draw skybox
	glUseProgram(skyboxProgram);
	rt3d::setUniformMatrix4fv(skyboxProgram, "projection", glm::value_ptr(projection));

	glDepthMask(GL_FALSE);
	glm::mat3 mvRotOnlyMat3 = glm::mat3(drawStack.top());
	drawStack.push(glm::mat4(mvRotOnlyMat3));

	glCullFace(GL_FRONT);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
	drawStack.top() = glm::scale(drawStack.top(), glm::vec3(1.5f, 1.5f, 1.5f));
	rt3d::setUniformMatrix4fv(skyboxProgram, "modelView", glm::value_ptr(drawStack.top()));
	rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
	drawStack.pop();
	glCullFace(GL_BACK);

	glDepthMask(GL_TRUE);

	//simple cube for test
	glUseProgram(programChoose);
	rt3d::setUniformMatrix4fv(programChoose, "projection", glm::value_ptr(projection));

	glm::vec4 tmp = drawStack.top() * lightPos;
	light.position[0] = tmp.x;
	light.position[1] = tmp.y;
	light.position[2] = tmp.z;
	rt3d::setLightPos(programChoose, glm::value_ptr(tmp));

	glBindTexture(GL_TEXTURE_2D, textures[0]);
	drawStack.push(drawStack.top());
	drawStack.top() = glm::translate(drawStack.top(), glm::vec3(0.0f, 1.0f, -10.0f));
	drawStack.top() = glm::scale(drawStack.top(), glm::vec3(1.0f, 1.0f, 1.0f));
	rt3d::setUniformMatrix4fv(programChoose, "modelView", glm::value_ptr(drawStack.top()));
	rt3d::setMaterial(programChoose, materialGroundPlane);
	rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
	drawStack.pop();

	// cube for ground plane
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	drawStack.push(drawStack.top());
	drawStack.top() = glm::translate(drawStack.top(), glm::vec3(-10.0f, -0.1f, -10.0f));
	drawStack.top() = glm::scale(drawStack.top(), glm::vec3(20.0f, 0.1f, 20.0f));
	rt3d::setUniformMatrix4fv(programChoose, "modelView", glm::value_ptr(drawStack.top()));
	rt3d::setMaterial(programChoose, materialGroundPlane);
	rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
	drawStack.pop();

	// draw a small cube block at lightPos
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	drawStack.push(drawStack.top());
	drawStack.top() = glm::translate(drawStack.top(), glm::vec3(lightPos[0], lightPos[1], lightPos[2]));
	drawStack.top() = glm::scale(drawStack.top(), glm::vec3(0.25f, 0.25f, 0.25f));
	rt3d::setUniformMatrix4fv(programChoose, "modelView", glm::value_ptr(drawStack.top()));
	rt3d::setMaterial(programChoose, materialGroundPlane);
	rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
	drawStack.pop();

	SDL_GL_SwapWindow(window);
}

int main(int argc, char* argv[]) {
	// setup window
	SDL_Window* window;
	SDL_GLContext context;
	window = setupSDL(context);

	// initialize glew
	GLenum glew(glewInit());
	if (glew != GLEW_OK) {
		cout << "glewInit() Error: " << glewGetErrorString(glew) << endl;
		SDL_GL_DeleteContext(context);
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	// initialize stuff
	init();

	// main loop
	bool finish = false;
	SDL_Event events;
	while (!finish) {
		while (SDL_PollEvent(&events)) {
			if (events.type == SDL_QUIT)
				finish = true;
		}
		
		update();
		draw(window);
	}

	// destroy context and window 
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}