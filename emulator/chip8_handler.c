#include <stdio.h>
#include <SDL2/SDL.h>

#define WIDTH 800
#define HEIGHT 600
#define DELAY 3000

SDL_Window* createWindow() {
	SDL_Window *window = NULL;
	SDL_Surface *screenSurface = NULL;
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
                fprintf(stderr, "SDL failed to initialise: %s\n", SDL_GetError());
                exit(1);
        }
        window = SDL_CreateWindow("SDL Example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, 0);
        if (window == NULL) {
                fprintf(stderr, "SDL window failed to initialize: %s\n", SDL_GetError());
                exit(1);
        }
	return window;
}

int main(int argc, char **argv) {
	SDL_Window *window = createWindow();

	SDL_Delay(DELAY);

	SDL_DestroyWindow(window);

	SDL_Quit();
	
	return 0;
}

