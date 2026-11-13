#include "raylib.h"

#include "constants.hpp"
#include "machine.hpp"
#include <cassert>
#include <print>
#include <fstream>
#include <iostream>



int main(int argc, char** argv) {
    if(argc != 2) {
        std::println("usage: `./forchip <path-to-rom-file>`");
        return -1;
    }

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "FORCHIP!!1");

    SetTargetFPS(60);

    Chip8Machine machine { argv[1] };

    while(!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(GetColor(COLOR_DARK_BACKGROUND));
            auto result = machine.execute_next();
            if(!result.has_value() && result.error().contains("SYS instruction")) {
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

int main_2() {
    std::ofstream ofs("mytest.ch8", std::ios::trunc | std::ios::binary);
    const uint8_t bytes[] = { 0x00, 0xE0, 0x00, 0xEE };
    ofs.write(reinterpret_cast<const char*>(&bytes), sizeof(bytes));
    ofs.close();

    return 0;
}