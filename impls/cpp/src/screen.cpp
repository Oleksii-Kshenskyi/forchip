#include "screen.hpp"
#include <cstdint>

#include "raylib.h"

void Screen::set(uint16_t x, uint16_t y) {
    this->screen[x][y] = 1;
}

bool Screen::get(uint16_t x, uint16_t y) const {
    return this->screen[x][y];
}

void Screen::draw() {
    Point start {
        static_cast<uint16_t>(SCREEN_WIDTH / 2 - (SCREEN_WIDTH_CHIP / 2 * PIXEL_SIZE)),
        static_cast<uint16_t>(SCREEN_HEIGHT / 2 - (SCREEN_HEIGHT_CHIP / 2 * PIXEL_SIZE))
    };

    for(uint16_t x = 0; x < SCREEN_WIDTH_CHIP; x++) {
        for(uint16_t y = 0; y < SCREEN_HEIGHT_CHIP; y++) {
            auto draw_color = this->get(x, y) ? COLOR_SET : COLOR_UNSET;
            DrawRectangle(start.x + x * PIXEL_SIZE, start.y + y * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE, GetColor(draw_color));
        }
    }
}

void Screen::clear() {
    this->screen.fill({});
}