#include <iostream>
#include "./SDL2/include/SDL.h"
#include <fstream>
#include <random>
#include <unordered_map>

#undef main

const int SCREEN_WIDTH = 640, SCREEN_HEIGHT = 320, LOGICAL_WIDTH = 64, LOGICAL_HEIGHT = 32;

//TODO add command line args to take the name of ROM files to be loaded
int main(int argc, char *argv []) {
    //printf("Test test");
    //RAM - 4096 bytes or 4 kB
    //Program space starts at address 200
    char * memory = new char[4096];
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
    bool originalRightShift = true;
    bool originalLeftShift = true;
    bool originalOffsetJmp = true;

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

    //Maps SDL2 scancodes to the hex digits they would correspond to on a hex keypad
    std::unordered_map<int, int> scanCodes = {
        {SDL_SCANCODE_1, 1}, {SDL_SCANCODE_2, 2}, {SDL_SCANCODE_3, 3}, {SDL_SCANCODE_4, 0xC},
        {SDL_SCANCODE_Q, 4}, {SDL_SCANCODE_W, 5}, {SDL_SCANCODE_E, 6}, {SDL_SCANCODE_R, 0xD},
        {SDL_SCANCODE_A, 7}, {SDL_SCANCODE_S, 8}, {SDL_SCANCODE_D, 9}, {SDL_SCANCODE_F, 0xE},
        {SDL_SCANCODE_Z, 0xA}, {SDL_SCANCODE_X, 0}, {SDL_SCANCODE_C, 0xB}, {SDL_SCANCODE_V, 0xF}
    };

    //printf("This is a test\n");

    //Loading font data into memory. Convention is to start storing the font data at 0x050 (0d80)
    for (unsigned int i = 0; i < 80; i++) {

        (memory[i + 80]) = font[i];

    }

    //printf("memory[1]: %d\n", memory[1]);

    //Initialize SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {

        printf("Error initializing SDL: %s\n", SDL_GetError());

    }

    //Create the SDL window and renderer
    SDL_Window * win = NULL;
    SDL_Renderer * render = NULL;

    win = SDL_CreateWindow("CHIP8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);

    if (win == NULL) {

        printf("Error creating SDL window: %s\n", SDL_GetError());

    }

    render = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    if (render == NULL) {

        printf("Error creating SDL renderer: %s\n", SDL_GetError());

    }
    
    SDL_RenderSetLogicalSize(render, LOGICAL_WIDTH, LOGICAL_HEIGHT);

    // //Test code

    // SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
    // SDL_RenderClear(render);

    // SDL_SetRenderDrawColor(render, 255, 255, 255, 255);
    // SDL_RenderDrawPoint(render, 0, 0);

    // SDL_RenderPresent(render);

    // SDL_Delay(10000);

    //Load the ROM data into memory
    std::fstream rom;
    rom.open(".\\IBM Logo.ch8", std::ios::in | std::ios::binary | std::ios::ate);

    if (rom.is_open()) {

        unsigned int i = 0;
        int size = rom.tellg();
        rom.seekg(0, std::ios::beg);

        while (i < size) {

            rom.read((char *)(&memory[i + 512]), 1);
            i++;

        }

    }
    else {

        std::cout << "Error: ROM could not be opened. Please make sure the file path is correct." << std::endl;
        return -1;

    }

    Uint8 memtest [4096];

    for (int i = 0; i < 4096; i++) {
        memtest[i] = memory[i];
    }

    //The main loop
    while (running) {

        //Allows user to close window
        SDL_Event event;
        while (SDL_PollEvent(&event)) {

            switch(event.type) {

                case SDL_QUIT:
                    running = false;
                    break;
    
            }

        }

        //Fetch an instruction from memory
        Uint8 upper, lower;
        Uint16 instruction;
        upper = memory[programCounter];
        lower = memory[programCounter + 1];

        //Increment program counter by two to prepare to fetch next instruction
        programCounter += 2;

        //Instruction decode
        //First nibble of the instruction determines which instruction category is being run
        short X = upper & 0x0F;
        short Y = (lower & 0xF0) >> 4;
        switch (upper & 0xF0) {

            //Execute machine language routine and clear screen instructions
            case 0x00:
                switch ((((short) upper & 0x0F) << 8) | (short) lower) {
                    //Clear instruction - sets all pixels to off
                    case 0x00E0:
                        SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
                        SDL_RenderClear(render);
                        for (int i = 0; i < 64; i++) {

                            for (int j = 0; j < 32; j++) {

                                display[i][j] = false;

                            }

                        }
                        break;
                    //Execute machine language routine - doesn't need to be implemented
                    default:
                        break;
                }
                break;
            //Jump instruction - takes the form 1NNN where NNN is the address the program counter is set to
            case 0x10:
                programCounter = (((short) upper & 0x0F) << 8) | lower;
                break;
            case 0x20:
                break;
            //Conditional jump instruction - takes the form 3XNN where if the value of VX == NN then the next instruction is skipped
            case 0x30:
                if (registers[X] == lower) {

                    programCounter += 2;

                }
                break;
            //Conditional jump instruction - takes the form 4XNN where if the values of VX != NN then the next instruction is skipped
            case 0x40:
                if (registers[X] != lower) {

                    programCounter += 2;

                }
                break;
            //Conditional jump instruction - takes the form 5XY0 where if VX == VY then the next instruction is skipped
            case 0x50:
                if (registers[X] == registers[Y]) {

                    programCounter += 2;

                }
                break;
            //Set register instruction - takes the form 6XNN; sets the register VX to the value NN
            case 0x60:
                registers[X] = lower;
                break;
            //Add instruction - takes the form 7XNN; adds NN to the register VX
            case 0x70:
                registers[X] += lower;
                break;
            //All 8000 instructions are arithmetic or logical - the exact instruction is determined by the lowest nibble; note that none of
            //these instructions affect VY
            case 0x80:
                switch (lower & 0x0F) {

                    //Set instruction - takes form 8XY0 and sets the value of VX to the value of VY
                    case 0x00:
                        registers[X] = registers[Y];
                        break;
                    //Binary OR instruction - takes form 8XY1 and sets the value of VX to VY | VX
                    case 0x01:
                        registers[X] = registers[X] | registers[Y];
                        break;
                    //Binary AND instruction - sets value of VX to VX & VY
                    case 0x02:
                        registers[X] = registers[X] & registers[Y];
                        break;
                    //Logical XOR - sets value of VX to VX XOR VY
                    case 0x03:
                        registers[X] = (~registers[X] & registers[Y]) | (registers[X] & ~registers[Y]);
                        break;
                    //Add - Sets the value of VX to VX + VY; affects carry flag
                    case 0x04:
                        {
                        unsigned short prev = registers[X];
                        registers[X] += registers[Y];
                        registers[0xF] = (prev > registers[X]) ? 1 : 0;
                        }
                        break;
                    //Subtract - sets the value of VX to VX - VY; VF is set to 1 if VX > VY
                    case 0x05:
                        registers[0xF] = (registers[X] > registers[Y]) ? 1 : 0;
                        registers[X] -= registers[Y];
                        break;
                    //THIS INSTRUCTION IS DIFFERENT IN SOME IMPLEMENTATIONS
                    //Right shift - in the original implementation, set VX = VY and shift VX right by one; set VF to the shifted bit
                    //In later implementations, shift VX in place and ignore VY
                    case 0x06:
                        if (originalRightShift) {
                            registers[X] = registers[Y];
                            registers[0x0F] = ((registers[X] & 1) == 1) ? 1 : 0;
                            registers[X] >> 1;
                        }
                        else {
                            registers[0x0F] = ((registers[X] & 1) == 1) ? 1 : 0;
                            registers[X] >> 1;
                        }
                        break;
                    //Subtract - sets the value of VX to VY - VX; VF is set to 1 if VX < VY
                    case 0x07:
                        registers[0xF] = (registers[X] < registers[Y]) ? 1 : 0;
                        registers[X] = registers[Y] - registers[X];
                        break;
                    //THIS INSTRUCTION IS DIFFERENT IN SOME IMPLEMENTATIONS
                    //Left shift - in the original implementation, set VX = VY and shift VX left by one; set VF to the shifted bit
                    //In later implementations, shift VX in place and ignore VY
                    case 0x0E:
                        if (originalLeftShift) {
                            registers[X] = registers[Y];
                            registers[0x0F] = ((registers[X] & 0x8000) == 0x8000) ? 1 : 0;
                            registers[X] << 1;
                        }
                        else {
                            registers[0x0F] = ((registers[X] & 0x8000) == 0x8000) ? 1 : 0;
                            registers[X] << 1;
                        }
                        break;

                }
                break;
            //Conditional jump instruction - takes the form 9XY0 where if VX != VY then the next instruction is skipped
            case 0x90:
                if (registers[X] != registers[Y]) {

                    programCounter += 2;

                }
                break;
            //Set index register instruction - takes the form ANNN; sets I to NNNN;
            case 0xA0:
                indexRegister = (((short) upper & 0x0F) << 8) | (short) lower;
                break;
            //THIS INSTRUCTION IS DIFFERENT IN SOME IMPLEMENTATIONS
            //Jump with offset - Takes form DNNN in the original implementation; In the original implementation, jumps to address NNN plus
            //the value of V0. In later implementations it takes the form DXNN and jumps to address XNN plus the value of VX
            case 0xB0:
                {
                    int jmp = (((short) upper & 0x0F) << 8) | (short) lower;
                    programCounter = (originalOffsetJmp) ? jmp + registers[0] : jmp + registers[X];
                }
                break;
            //Generate random number - Takes form CXNN; generates a random number, ANDs it with NN and puts the value in VX
            case 0xC0:
                {
                std::random_device dev;
                std::mt19937 rng(dev());
                std::uniform_int_distribution<std::mt19937::result_type> dist(-32768, 32767);
                registers[X] = dist(rng) & lower;
                }
                break;
            //Display instruction - takes the form DXYN; draws an N pixel tall sprite from the memory location stored at the index register
            //to the horizontal coordinate stored in VX and vertical coordinate stored in VY. If any pixels are turned off, then VF is set
            //to 1 (otherwise set to 0)
            case 0xD0:
                {
                char N = lower & 0x0F;
                short xCoord = registers[X] & 63;
                short yCoord = registers[Y] & 31;
                
                registers[0xF] = 0;

                for (unsigned int i = 0; i < N; i++) {

                    if (Y + i > 31) break;

                    char spriteData = memory[indexRegister + i];

                    for (unsigned int j = 0; j < 8; j++) {

                        if (xCoord + j > 63) break;

                        //If the pixel is already on and the pixel in the sprite row is on, turn off the pixel and set VF to 1
                        if (display[xCoord + j][yCoord + i] && ((spriteData << j) & 0x80) == 0x80) {

                            registers[0xF] = 1;
                            display[xCoord + j][yCoord + i] = false;
                            SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
                            SDL_RenderDrawPoint(render, xCoord + j, yCoord + i);

                        }
                        //If the pixel in the sprite row is on but not already on, turn the pixel on
                        else if (((spriteData << j) & 0x80) == 0x80) {

                            display[xCoord + j][yCoord + i] = true;
                            SDL_SetRenderDrawColor(render, 255, 255, 255, 255);
                            SDL_RenderDrawPoint(render, xCoord + j, yCoord + i);

                        }

                    }

                }

                SDL_RenderPresent(render);

                }
                break;
            //Skip if instructions - both instructions skip based on if a key is currently being pressed or not
            //CHIP 8 uses a hexidecimal keypad so each code corresponds to a hex digit
            case 0xE0:
                switch (lower) {
                    //Skip if key pressed - takes form EX9E; skips the next instruction if the key corresponding to the number in VX
                    //is pressed
                    case 0x9E:
                        {
                        if (SDL_PollEvent(&event)) {
                            if (event.type == SDL_KEYDOWN) {
                                SDL_Scancode sc = event.key.keysym.scancode;
                                short hxVal = scanCodes[sc];
                                programCounter += (hxVal == registers[X]) ? 2 : 0;
                            }
                        }
                        }
                        break;
                    //Skip if not key pressed - takes form EXA1; skips the next instruction if the key corresponding to the number in VX
                    //is not being pressed
                    case 0xA1:
                        {
                        if (SDL_PollEvent(&event)) {
                            if (event.type == SDL_KEYDOWN) {
                                SDL_Scancode sc = event.key.keysym.scancode;
                                short hxVal = scanCodes[sc];
                                programCounter += (hxVal != registers[X]) ? 2 : 0;
                            }
                        }
                        }
                        break;
                }
                break;
            //Timers and miscellaneous instructions
            case 0xF0:
                break;

        }

    }

    //Cleanup
    rom.close();
    SDL_DestroyWindow(win);
    SDL_DestroyRenderer(render);
    SDL_Quit();
    delete [] memory;

    return 0;
}