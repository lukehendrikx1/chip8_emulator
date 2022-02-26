#ifndef CHIP8_HANDLER_H
#define CHIP8_HANDLER_H

// Required standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Required created header files
#include "chip8_emulator.h"
#include "font.h"

// Importing SDL based on OS
#ifdef _WIN32 // Windows
#include "SDL/SDL.h"
#else // Linux and OSX
#include "SDL2/SDL.h"
#endif

// Defining constants
#define WIDTH 600
#define HEIGHT 800
#define DELAY 3000

#endif