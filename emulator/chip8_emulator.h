#include <stdint.h>

typedef struct state { // Variables for registers, pointers and memory
        uint8_t V[16]; // Array of registers
        uint16_t I; 
        uint16_t stackPointer;
        uint16_t programCounter;
        uint8_t delay;
        uint8_t sound;
        uint8_t *memory; // Memory variable
        uint8_t *screen; // Screen location within memory
	uint8_t keyState[16]; // State of each key
	uint8_t savedKeyState[16]; // Stored state of each key
	int keyInterupt;
	uint8_t haltInterupt;
} state;


state *initializeState(void);
void emulate(state *s);
