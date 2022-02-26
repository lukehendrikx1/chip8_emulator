#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "chip8_emulator.h"
#include "font.h"
#include <string.h>

static void unknownInstruction(state *s) { // Handles instruction that is incorrect or unemplementd
	printf("Error: Unknown Instruction\n");
	exit(1);
}

static void instruction0(state *s, uint8_t *code) { // Handles instructions starting with 0
	switch(code[1]) {
		case 0xe0:
			memset(s->screen, 0, 64*32/8); // Clears screen
			s->programCounter += 2;
			break;
		case 0xee: ;
			uint16_t address = (s->memory[s->stackPointer] << 8) | s->memory[s->stackPointer + 1];
			s->stackPointer += 2; // Returns from subroutine and return the stack pointer to its previous position
			s->programCounter = address;
			break;
		default:
			unknownInstruction(s);
			break;
	}
}

static void instruction1(state *s, uint8_t *code) { // Handles instruction starting with 1
	uint16_t address = ((code[0]&0x0f)<<8) | code[1]; // Jumps to a given address
	if (address == s->programCounter) {
		printf("Infinte loop, halt flag set\n");
		s->haltInterupt = 1;
	}
	s->programCounter = address;
}

static void instruction2(state *s, uint8_t *code) { // Handles instruction starting with 2
	s->stackPointer -= 2; // Calls a subroutine at an address, and adds to the stack
	s->memory[s->stackPointer] = ((s->programCounter + 2) & 0xff00) >> 8;
	s->memory[s->stackPointer + 1] = (s->programCounter + 2) & 0xff;
	s->programCounter - ((code[0]&0xf) << 8) | code[1];	
}

static void instruction3(state *s, uint8_t *code) { // Handles instruction starting with 3
	uint8_t regi = code[0] & 0xf; // Skips the next instruction if the given value is equal to the given register
	if (s->V[regi] == code[1])
		s->programCounter += 2;
	s->programCounter += 2;
}

static void instruction4(state *s, uint8_t *code) { // Handles instruction starting with 4
	uint8_t regi = code[0] & 0xf; // Skips the next instruction if the given value is not equal to the given register
	if (s->V[regi] != code[1])
		s->programCounter += 2;
	s->programCounter += 2;
}

static void instruction5(state *s, uint8_t *code) { // Handles instruction starting with 5
	uint8_t regi1 = code[0] & 0xf; // Skips the next instruction if the given registers are equal
	uint8_t regi2 = (code[1] & 0xf0) >> 4;
	if (s->V[regi1] == s->V[regi2]) 
		s->programCounter += 2;
	s->programCounter += 2;
}

static void instruction6(state *s, uint8_t *code) { // Handles instruction starting with 6
	uint8_t regi = code[0] & 0xf; // Loads the given value into the given register
	s->V[regi] = code[1];
	s->programCounter += 2;
}

static void instruction7(state *s, uint8_t *code) { // Handles instruction starting with 7
	uint8_t regi = code[0] & 0xf; // Adds the given value to the the given register
	s->V[regi] += code[1];
	s->programCounter += 2;
}

static void instruction8(state *s, uint8_t *code) {// Handles instructions starting with 8
	uint8_t regiX = code[0] & 0xf;
	uint8_t regiY = (code[1] & 0x0f) >> 4;
	switch(code[1] & 0xf) {
		case 0: s->V[regiX] = s->V[regiY]; break; // Sets one given register to the value of another given register
		case 1: s->V[regiX] |= s->V[regiY]; break; // ORs the two given registers
		case 2: s->V[regiX] &= s->V[regiY]; break; // ANDs the two given registers
		case 3: s->V[regiX] ^= s->V[regiY]; break; // XORs the two given registers
		case 4: ; // Adds the two given registers, set VF as carry
			uint16_t result = s->V[regiX] + s->V[regiY];
			if (result & 0xff00)
				s->V[0xf] = 1;
			else
				s->V[0xf] = 0;
			s->V[regiX] = result & 0xff;
			break;
		case 5: ; // Subtracts the two given registers, sets VF as NOT borrow
			int borrow = (s->V[regiX] > s->V[regiY]);
			s->V[regiX] -= s->V[regiY];
			s->V[0xf] = borrow;
			break;
		case 6: ; // Sets VF to the least sigfinicant value of the given register then divides the given register by 2
			uint8_t regiF = s->V[regiX] & 0x1;
			s->V[regiX] = s->V[regiX] >> 1;
			s->V[0xf] = regiF;
			break;
		case 7: ; // Subtracts the two given registers, set VF as NOT borrow (opposite order to 8xy5)
			int borrow2 = (s->V[regiY] > s->V[regiX]);
			s->V[regiX] = s->V[regiY] - s->V[regiX];
			s->V[0xf] = borrow2;
			break;
		case 0xe: ; // Sets VF to the least sigfinicant value of the given register then multiplies the given register by 2
			uint8_t regiF2 = (0x80 == (s->V[regiX] & 0x80));
			s->V[regiX] = s->V[regiX] << 1;
			s->V[0xf] = regiF2;
		default:
			unknownInstruction(s);
			break;
	}
	s->programCounter += 2;
}

static void instruction9(state *s, uint8_t *code) { // Handles instruction starting with 9
	uint8_t regi1 = code[0] & 0xf; // Skips the next instruction if the given registers are not equal
	uint8_t regi2 = (code[1]&0xf0) >> 4;
	if (s->V[regi1] != s->V[regi2]) 
		s->programCounter += 2;
	s->programCounter += 2;
}

static void instructionA(state *s, uint8_t *code) { // Handles instruction starting with A
	s->I = ((code[0] & 0xf) << 8) | code[1]; // Set I to the given address
	s->programCounter += 2;
}

static void instructionB(state *s, uint8_t *code) { // Handles instruction starting with B
	s->programCounter = ((uint16_t)s->V[0] + (((code[0] & 0xf) << 8) | code[1])); // Jumps to given address + V0
}

static void instructionC(state *s, uint8_t *code) { // Handles instruction starting with C
	uint8_t regi = code[0] & 0xf; // Generates a random byte then ANDs it with the given value the stores it in the given register
	s->V[regi] = random() & code[1];
	s->programCounter += 2;
}

static void instructionD(state *s, uint8_t *code) { // Handles instruction starting with D
	int lines = code[1] & 0xf; // Draws a sprite from memory location I
	int xCoor = s->V[code[0] & 0xf]; // At given registers, with length of n
	int yCoor = s->V[(code[1] & 0xf) >> 4]; // If there is a collision VF is set to 1
	s->V[0xf] = 0;
	for (int i = 0; i < lines; i++) {
		uint8_t *sprite = &s->memory[s->I+i];
		int spriteBit = 7;
		for (int j = xCoor; j < (xCoor + 8) && j < 64; j++) {
			int jOver8 = j / 8;
			int jMod8 = j % 8;
			uint8_t srcBit = (*sprite >> spriteBit) & 0x1;
			if (srcBit) {
				uint8_t *destByte_p = &s->screen[(i+yCoor) *(64/8) + jOver8];
				uint8_t destByte = *destByte_p;
				uint8_t destMask = (0x80 >> jMod8);
				uint8_t destBit = destByte & destMask;
				srcBit = srcBit << (7 - jMod8);
				if (srcBit & destBit)
					s->V[0xf] = 1;
				destBit ^= srcBit;
				destByte = (destBit & ~destMask) | destBit;
				*destByte_p = destByte;
			}
			spriteBit--;
		}
	}
	s->programCounter += 2;
}

static void instructionE(state *s, uint8_t *code) { // Handles instructions starting with E
	int regi = code[0] & 0xf;
	switch (code[1]) {
		case 0x9e: // Skips next instruction if the key with the value of the given register is pressed
			if (s->keyState[s->V[regi]] != 0) 
				s->programCounter += 2;
			break;
		case 0xa1: // Skips next instruction if the key with the value of the given register is not pressed
			if (s->keyState[s->V[regi]] == 0)
				s->programCounter += 2;
			break;
		default:
			unknownInstruction(s);
			break;
	}
	s->programCounter += 2;
}

static void instructionF(state *s, uint8_t *code) { // Handles instruction starting with F
	int regi = code[0] & 0xf;
	switch(code[1]) {
		case 0x07: s->V[regi] = s->delay; break; // Loads the given register with the delay timer value
		case 0x0a: // Wait for a key press then load the value of the key into the given register
			   if (s->keyInterupt == 0) {
				   memcpy(&s->savedKeyState, &s->keyState, 16);
				   s->keyInterupt = 1;
				   return;
			   } else {
				   for (int i = 0; i < 16; i++) {
					   if((s->savedKeyState[i] == 0) && (s->keyState[i] == 1)) {
						   s->keyInterupt = 0;
						   s->V[regi] = i;
						   s->programCounter += 2;
						   return;
					   }
					   s->savedKeyState[i] = s->keyState[i];
				   }
				   return;
			   }
			   break;
		case 0x15: s->delay = s->V[regi]; break; // Load the delay timer with the value of the given register
		case 0x18: s->sound = s->V[regi]; break; // Load the sound timer with the value of the given regsiter
		case 0x1e: s->I += s->V[regi]; break; // Adds the given register to I
		case 0x29: s->I = FONT_BASE + (s->V[regi] * 5); break; // Sets I to the address of the font character for value of given register
		case 0x33: ; // Store the BCD representation of the given register in memory
			   uint8_t one, ten, hundred;
			   uint8_t magnitude = s->V[regi];
			   one = magnitude % 10;
			   magnitude /= 10;
			   ten = magnitude % 10;
			   magnitude /= 10;
			   hundred = magnitude % 10;
			   s->memory[s->I] = hundred;
			   s->memory[s->I + 1] = ten;
			   s->memory[s->I + 2] = one;
			   break;
		case 0x55: // Stores the register values into memory
			   for (int i = 0; i <= regi; i++) 
				   s->memory[s->I+i] = s->V[i];
			   s->I += (regi+1);
			   break;
		case 0x65: // Transfers memory to registers from registers from V0 to the given register
			   for (int i = 0; i <= regi; i++)
				   s->V[i] = s->memory[s->I+i];
			   s->I += (regi+1);
			   break;
		default:
			unknownInstruction(s);
			break;
	}
	s->programCounter += 2;
}

state *intializeState(void) { // Initalizes emulator and sets the memory and pointers to the starting states
	state *s = calloc(sizeof(state), 1);
	s->memory = calloc(1024*4, 1);
	s->screen = &s->memory[0xf00]; // Allocating memory for the screen
	s->stackPointer = 0xfa0; // Allocating memory for the stack
	s->programCounter = 0x200;
	memcpy(&s->memory[FONT_BASE], font, FONT_SIZE); // Loading the font into memory
	return s;
}

void emulate(state *s) {
	uint8_t *instruction = &s->memory[s->programCounter]; // Execute instruction at the program counter

	switch ((*instruction & 0xf0) >> 4) { // Checks first character of the instruction
		case 0x00: instruction0(s, instruction); break;
		case 0x01: instruction1(s, instruction); break;
		case 0x02: instruction2(s, instruction); break;
		case 0x03: instruction3(s, instruction); break;
		case 0x04: instruction4(s, instruction); break;
		case 0x05: instruction5(s, instruction); break;
		case 0x06: instruction6(s, instruction); break;
		case 0x07: instruction7(s, instruction); break;
		case 0x08: instruction8(s, instruction); break;
		case 0x09: instruction9(s, instruction); break;
		case 0x0a: instructionA(s, instruction); break;
		case 0x0b: instructionB(s, instruction); break;
		case 0x0c: instructionC(s, instruction); break;
		case 0x0d: instructionD(s, instruction); break;
		case 0x0e: instructionE(s, instruction); break;
		case 0x0f: instructionF(s, instruction); break;
	}
}
