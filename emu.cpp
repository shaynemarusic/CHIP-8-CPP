#include <iostream>
#include <SDL.h>
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
    //TODO - 16 8 bit registers. Unsure if I want this as an array of unsigned chars or as an unordered map mapping each register name to it's content

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

            case 0x00:
                break;
            case 0x10:
                break;
            case 0x20:
                break;
            case 0x30:
                break;
            case 0x40:
                break;
            case 0x50:
                break;
            case 0x60:
                break;
            case 0x70:
                break;
            case 0x80:
                break;
            case 0x90:
                break;
            case 0xA0:
                break;
            case 0xB0:
                break;
            case 0xC0:
                break;
            case 0xD0:
                break;
            case 0xE0:
                break;
            case 0xF0:
                break;

        }
        
    }

    return 0;
}