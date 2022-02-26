// chip8_handler.c
// Owns and operates the emulator

#include "chip8_handler.h" // Includes header file, includes libraries, header files and SDL, defines constants

SDL_Window* createWindow() { // Creates the SDL window and returns it
	SDL_Window *window = NULL;
	
        if (SDL_Init(SDL_INIT_VIDEO) != 0) { // Initliaizes the window and throws an error if it fails
                fprintf(stderr, "SDL failed to initialise: %s\n", SDL_GetError());
                exit(1);
        }

        window = SDL_CreateWindow("Chip8 Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, 0); // Creates window

        if (window == NULL) { // Throws error if the window is not created properly
                fprintf(stderr, "SDL window failed to initialize: %s\n", SDL_GetError());
                exit(1);
        }

	return window;
}

void createSurface(SDL_Window* window) { // Creates a base surface for the window
        SDL_Surface* surface = NULL;

        surface = SDL_GetWindowSurface(window); // Gets the surface from the window
        SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 0x00, 0x00, 0x00)); // Fills it as a black rectangle

        SDL_UpdateWindowSurface(window); // Updates the window
}

int main(int argc, char **argv) {
        // TODO: 
        // Initliaize all variables of the emulator
        // Create game loop for program
          // Check & handle flags
          // Update the screen
          // Handle keypresses
          // Handle sound
          // Handle timers
        if (argc == 1) {
                fprintf(stderr, "Error: not enough arguments\n");
                exit(1);
        } else if (argc > 2) {
                fprintf(stderr, "Error: too many arguments\n");
                exit(1);
        }

	SDL_Window *window = createWindow(); // Creates the window
        createSurface(window);


        state *s = intializeState(); // Creates the emulator

        FILE *program = fopen(argv[1], "rb"); // Opening the file
        if (program == NULL) {
                fprintf(stderr, "Error: file does not exist\n");
                exit(1);
        }

        fseek(program, 0L, SEEK_END); // Finding the size of the file
        int programSize = ftell(program);
        fseek(program, 0L, SEEK_SET);
        if (programSize + 0x200 > 0xf00) {
                fprintf(stderr, "Error: program too large for memory\n");
        }

        memcpy(&s->memory[s->programCounter], program, programSize);// Putting program into memory
        
        fclose(program); // Closing file

	SDL_Delay(DELAY);

	SDL_DestroyWindow(window); // Destroys window then quits SDL
	SDL_Quit();
	
	return 0;
}

