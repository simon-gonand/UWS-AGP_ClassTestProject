#include <iostream>
#include <string>
#include <SDL.h>
#include <GL/glew.h>

using namespace std;

SDL_Window* setupSDL(SDL_GLContext& context) {
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		cout << "SDL_Init Error: " << SDL_GetError() << endl;
		exit(1);
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	SDL_Window* window = SDL_CreateWindow("Class test Project", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (window == nullptr) {
		cout << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
		SDL_Quit();
		exit(1);
	}

	context = SDL_GL_CreateContext(window);

	return window;
}

int main(int argc, char* argv[]) {
	SDL_Window* window;
	SDL_GLContext context;
	window = setupSDL(context);

	bool finish = false;
	SDL_Event events;
	while (!finish) {
		SDL_WaitEvent(&events);
		const Uint8* keys = SDL_GetKeyboardState(NULL);

		if (events.window.event == SDL_WINDOWEVENT_CLOSE || keys[SDL_SCANCODE_ESCAPE])
			finish = true;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		SDL_GL_SwapWindow(window);
	}

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}