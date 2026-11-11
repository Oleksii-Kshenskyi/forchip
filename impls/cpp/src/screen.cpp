#include "screen.hpp"
#include <cstdint>

void Screen::set(uint16_t x, uint16_t y) {
    this->screen[x][y] = 1;
}

bool Screen::get(uint16_t x, uint16_t y) const {
    return this->screen[x][y];
}