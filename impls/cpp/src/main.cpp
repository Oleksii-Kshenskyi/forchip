#include "raylib.h"

#include "constants.hpp"
#include "machine.hpp"
#include <cassert>
#include <print>
#include <fstream>
#include <iostream>

void burn_test_program() {
    std::ofstream ofs("mytest.ch8", std::ios::trunc | std::ios::binary);
    //const uint8_t bytes[] = { 0x00, 0xE0, 0x22, 0x07, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x00, 0xEE };
    //const uint8_t bytes[] = { 0x00, 0xE0, 0x63, 0xCC, 0x73, 0x03, 0x33, 0xCF, 0xFF, 0xFF, 0x00, 0x00 };
    //const uint8_t bytes[] = { 0x63, 0xEC, 0x6D, 0xED, 0x93, 0xD0, 0xFF, 0xFF, 0xA3, 0x7B };
    //const uint8_t bytes[] = { 0x60, 0x03, 0xBF, 0xFD };
    //const uint8_t bytes[] = { 0xC3, 0xFF, 0xCC, 0x7F, 0xCE, 0x0F };
    const uint8_t bytes[] = { 0x63, 0xFC };
    ofs.write(reinterpret_cast<const char*>(&bytes), sizeof(bytes));
    ofs.close();
}

int main(int argc, char** argv) {
    if(argc != 2) {
        std::println("usage: `./forchip <path-to-rom-file>`");
        return -1;
    }

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "FORCHIP!!1");

    SetTargetFPS(60);

    if(std::string { argv[1] } == "mytest.ch8") {
        std::println("[DEBUG MODE] ROM is mytest.ch8, burning and executing the test program into the .ch8 file!");
        burn_test_program();
    }
    Chip8Machine machine { argv[1] };

    while(!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(GetColor(COLOR_DARK_BACKGROUND));
            auto result = machine.execute_next();
            if(result.has_value() && result.value() == InstructionType::HALT) {
                std::println("REACHED SYS, DBG PRINTOUT => EXIT:");
                machine.dump_region(0x200, 0x200 + machine.get_rom_size());
                machine.dump_registers();
                std::cin.get();
                return 0;
            }
            if(!result.has_value()) {
                std::println("ERROR: {}", result.error());
            }
            machine.draw_screen();
        EndDrawing();
    }

    return 0;
}