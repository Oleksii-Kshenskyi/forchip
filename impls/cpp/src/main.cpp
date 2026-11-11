#include "raylib.h"

#include "constants.hpp"
#include "screen.hpp"
#include "machine.hpp"

void draw_screen(const Screen& screen) {
    Point start {
        static_cast<uint16_t>(SCREEN_WIDTH / 2 - (SCREEN_WIDTH_CHIP / 2 * PIXEL_SIZE)),
        static_cast<uint16_t>(SCREEN_HEIGHT / 2 - (SCREEN_HEIGHT_CHIP / 2 * PIXEL_SIZE))
    };

    for(uint16_t x = 0; x < SCREEN_WIDTH_CHIP; x++) {
        for(uint16_t y = 0; y < SCREEN_HEIGHT_CHIP; y++) {
            auto draw_color = screen.get(x, y) ? COLOR_SET : COLOR_UNSET;
            DrawRectangle(start.x + x * PIXEL_SIZE, start.y + y * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE, GetColor(draw_color));
        }
    }
}

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "FORCHIP!!1");

    SetTargetFPS(60);

    Screen screen;
    screen.set(0, 0);
    screen.set(2, 0);
    screen.set(63, 0);
    screen.set(0, 31);
    screen.set(63, 31);
    screen.set(32, 1);

    Chip8Machine machine;
    machine.dump_region(0x000, 0x200);

    while(!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(GetColor(COLOR_DARK_BACKGROUND));
            draw_screen(screen);
        EndDrawing();
    }

    return 0;
}