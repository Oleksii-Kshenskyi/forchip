#include "screen.hpp"
#include <cstdint>
#include <print>

#include "raylib.h"

void Screen::set(uint16_t x, uint16_t y) {
    uint64_t& row = this->screen[y];
    uint64_t mask = 1ULL << x;
    std::println("[SET ({:02},{:02})]: row = {:08X}; mask = {:08X};",
        x, y, row ,mask);
    row |= mask;
    std::println("=> ROW AFTER MASK: {:08X};", row);
    std::println("=> ROW AFTER MASK/this->screen: {:08X};", this->screen[y]);
}

bool Screen::get(uint16_t x, uint16_t y) const {
    const uint64_t& row = this->screen[y];
    return static_cast<bool>((row >> x) & 0x1);
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