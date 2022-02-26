#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

void disassembleInstruction(uint8_t * program, int programCounter) { 
	// Function disassembles instruction at program counter
	// programCounter: Integer pointing to an address in memory
	// program is a pointer to the start of the data
	uint8_t * instruction = &program[programCounter]; // Instruction a programCounter address
	printf("%04x, %02x %02x", programCounter, instruction[0], instruction[1]);
	switch(instruction[0] >> 4) { // Checking the first nibble of the instruction
		case 0x00: // Checking the second nibble of the instruction
			switch (instruction[1]) {
				case 0xe0: printf("\tCLS"); break;
				case 0xee: printf("\tRTS"); break;
				case 0x00: printf("\tNOP"); break;
				default: printf("\tUnknown instruction");
			}
			break;
		case 0x01: printf("\tJP $%01x%02x", instruction[0]&0x0f, instruction[1]); break;
		case 0x02: printf("\tCALL $%01x%02x", instruction[0]&0x0f, instruction[1]); break;
		case 0x03: printf("\tSE V%01x,#$%02x", instruction[0]&0x0f, instruction[1]); break;
		case 0x04: printf("\tSNE V%01x,#$%02x", instruction[0]&0xf, instruction[1]); break;
		case 0x05: printf("\tSE V%01x,V%01x", instruction[0]&0x0f, instruction[1]>>4); break;
		case 0x06: printf("\tLD V%01x,#$%02x", instruction[0]&0x0f, instruction[1]); break;
		case 0x07: printf("\tADD V%01x,#$%02x", instruction[0]&0x0f, instruction[1]); break;
		case 0x08: // Checking the second nibble of the instruction
			switch(instruction[1]>>4) {
				case 0: printf("\tLD V%01x,V%01x", instruction[0]&0x0f, instruction[1]>>4); break;
				case 1: printf("\tOR V%01x,V%01x", instruction[0]&0x0f, instruction[1]>>4); break;
				case 2: printf("\tAND V%01x,V%01x", instruction[0]&0x0f, instruction[1]>>4); break;
				case 3: printf("\tXOR V%01x,V%01x", instruction[0]&0x0f, instruction[1]>>4); break;
				case 4: printf("\tADD V%01x,V%01x", instruction[0]&0xff, instruction[1]>>4); break;
				case 5: printf("\tSUB V%01x,V%01x", instruction[0]&0x0f, instruction[1]>>4); break;
				case 6: printf("\tSHR V%01x,V%01x", instruction[0]&0x0f, instruction[1]>>4); break;
				case 7: printf("\tSUBN V%01x,V%01x", instruction[0]&0x0f, instruction[1]>>4); break;
				case 0xe: printf("\tSHL V%01x,V%01x", instruction[0]&0x0f, instruction[1]>>4); break;
				default: printf("\tUnknown instruction");
			}
			break;
		case 0x09: printf("\tSNE V%01x,V%01x", instruction[0]&0x0f, instruction[1]>>4); break;
		case 0x0a: printf("\tMVI I,#$%01x%02x", instruction[0]&0x0f, instruction[1]);break;
		case 0x0b: printf("\tJP V0,$%01x%02x", instruction[0]&0x0f, instruction[1]); break;
		case 0x0c: printf("\tRND V%01x,#$%02x", instruction[0]&0x0f, instruction[1]); break;
		case 0x0d: printf("\tDRW V%01x,V%01x,#$%01x", instruction[0]&0x0f,instruction[1]>>4, instruction[1]&0x0f); break;
		case 0x0e: // Checking the second nibble of the instruction
			  switch(instruction[1]) {
				case 0x9e: printf("\tSKP V%01x", instruction[0]&0x0f); break;
				case 0xa1: printf("\tSKNP V%01x", instruction[0]&0x0f); break;
				default: printf("\tunknown instruction"); break;
			  }
			  break;
		case 0x0f: // Checking the second nibble of the instruction
			  switch(instruction[1]) {
				case 0x07: printf("\tLD V%01x,DT", instruction[0]&0x0f); break;
				case 0x0a: printf("\tLD V%01x,K", instruction[0]&0x0f); break;
				case 0x15: printf("\tLD DT,V%01x", instruction[0]&0x0f); break;
				case 0x18: printf("\tLD ST,V%01x", instruction[0]&0x0f); break;
				case 0x1e: printf("\tADD I,V%01x", instruction[0]&0x0f); break;
				case 0x29: printf("\tLD F,V%01x", instruction[0]&0x0f); break;
				case 0x33: printf("\tLD B,V%01x", instruction[0]&0x0f); break;
				case 0x55: printf("\tLD [I],V%01x", instruction[0]&0x0f); break;
				case 0x65: printf("\tLD V%01x,[I]", instruction[0]&0x0f); break;
				default: printf("\tunknown instruction"); break;
			  }
			  break;
	}
}


int main (int argc, char ** argv) { // Main Function
	FILE *file = fopen(argv[1], "rb"); // Opens file
	if (file == NULL) {
		printf("error: file does not exist\n");
		exit(1);
	}

	fseek(file, 0L, SEEK_END); // Find size of file
	int fileSize = ftell(file);
	fseek(file, 0L, SEEK_SET);

	unsigned char *buffer = malloc(fileSize+0x200); // Allocates memory then stores program
	fread(buffer+0x200, fileSize, 1, file);

	fclose(file); // Closes file

	int programCounter = 0x200;
	while (programCounter < (fileSize+0x200)) { // Looping through every instruction in program
		disassembleInstruction(buffer, programCounter);
		programCounter += 2;
		printf("\n");
	}
	return 0;
}

