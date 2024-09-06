#include <iostream>
#include "./SDL2/include/SDL.h"
#include <fstream>

//TODO add command line args to take the name of ROM files to be loaded
int main(int argc, char *argv []) {
    //RAM - 4096 bytes or 4 kB
    //Program space starts at address 200
    char * memory [4096];
    //64x32 pixel display which can be either black or white
    bool display [64][32];
    //Points to locations in memory - 16 bits/2 bytes
    //Also called I
    unsigned short indexRegister;
    //Used for calling functions/subroutines
    unsigned short stack [16];
    //The program counter. Keeps track of which instruction should be fetched from memory. Program memory starts at 0x200
    unsigned short programCounter = 0x200;
    //Decremented at a rate of 60 Hz until it reaches 0
    unsigned char delayTimer;
    //Like the delay timer but it makes a sound when it's not 0
    unsigned char soundTimer;
    //Keeps the main loop running
    bool running = true;
    //TODO - 16 8 bit registers. Registers are labled V0 to VF. Note: VF is a special register that is used as a flag register
    short registers [16];

    //Font data
    unsigned char font [80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    //Loading font data into memory. Convention is to start storing the font data at 0x050 (0d80)
    for (unsigned int i = 0; i < 80; i++) {

        *memory[i + 80] = font[i];

    }

    //Initialize SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {

        printf("Error initializing SDL: %s\n", SDL_GetError());

    }

    //Create the SDL window
    SDL_Window * win = SDL_CreateWindow("CHIP8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 64, 32, 0);

    SDL_Surface * surface = SDL_GetWindowSurface(win);

    SDL_ShowWindow(win);

    while (true);

    //Load the ROM data into memory
    std::fstream rom;
    rom.open(argv[1], std::ios::in | std::ios::binary | std::ios::ate);

    if (rom.is_open()) {

        unsigned int i = 0;
        int size = rom.tellg();
        rom.seekg(0, std::ios::beg);

        while (i < size) {

            rom.read(memory[i + 512], 1);
            i++;

        }

    }
    else {

        std::cout << "Error: ROM could not be opened. Please make sure the file path is correct." << std::endl;

    }

    //The main loop
    while (running) {

        //Fetch an instruction from memory
        char upper, lower;
        upper = *memory[programCounter];
        lower = *memory[programCounter + 1];

        //Increment program counter by two to prepare to fetch next instruction
        programCounter += 2;

        //Instruction decode
        //First nibble of the instruction determines which instruction category is being run
        switch (upper & 0xF0) {

            //Execute machine language routine and clear screen instructions
            case 0x00:
                switch ((((short) upper & 0x0F) << 4) | (short) lower) {
                    //Clear instruction - sets all pixels to off
                    case 0x00E0:
                        break;
                    //Execute machine language routine - doesn't need to be implemented
                    default:
                        break;
                }
                break;
            //Jump instruction - takes the form 1NNN where NNN is the address the program counter is set to
            case 0x10:
                programCounter = (((short) upper & 0x0F) << 4) | lower;
                break;
            case 0x20:
                break;
            case 0x30:
                break;
            case 0x40:
                break;
            case 0x50:
                break;
            //Set register instruction - takes the form 6XNN; sets the register VX to the value NN
            case 0x60:
                {
                short reg = upper & 0x0F;
                registers[reg] = lower;
                }
                break;
            //Add instruction - takes the form 7XNN; adds NN to the register VX
            case 0x70:
                {
                short reg = upper & 0x0F;
                registers[reg] += lower;
                }
                break;
            case 0x80:
                break;
            case 0x90:
                break;
            //Set index register instruction - takes the form ANNN; sets I to NNNN;
            case 0xA0:
                indexRegister = (((short) upper & 0x0F) << 4) | lower;
                break;
            case 0xB0:
                break;
            case 0xC0:
                break;
            //Display instruction - takes the form DXYN; draws an N pixel tall sprite from the memory location stored at the index register
            //to the horizontal coordinate stored in VX and vertical coordinate stored in VY. If any pixels are turned off, then VF is set
            //to 1 (otherwise set to 0)
            case 0xD0:
                {
                char X = upper & 0x0F;
                char Y = lower & 0xF0;
                char N = lower & 0x0F;
                short xCoord = registers[X] & 63;
                short yCoord = registers[Y] & 31;
                
                registers[0xF] = 0;

                for (unsigned int i = 0; i < N; i++) {

                    if (Y + i > 31) break;

                    char spriteData = *memory[indexRegister + i];

                    for (unsigned int j = 0; j < 8; j++) {

                        if (X + j > 63) break;

                        //If the pixel is already on and the pixel in the sprite row is on, turn off the pixel and set VF to 1
                        if (display[X + j][Y + i] && ((spriteData << j) & 0x80) == 0x80) {

                            registers[0xF] = 1;
                            display[X + j][Y + i] = false;

                        }
                        else if (((spriteData << j) & 0x80) == 0x80) display[X + j][Y + i] = true;

                    }

                }

                //TODO - draw the screen with SDL
                }
                break;
            case 0xE0:
                break;
            case 0xF0:
                break;

        }

    }

    rom.close();

    return 0;
}

int WinMain() {
    return 0;
}