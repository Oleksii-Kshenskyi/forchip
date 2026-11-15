#pragma once

#include <array>

#include "constants.hpp"

struct Point {
    Point(uint16_t x, uint16_t y): x(x), y(y) {}
    Point(): x(0), y(0) {}

    uint16_t x;
    uint16_t y;
};

template<typename std::size_t Row>
using Matrix2D = std::array<uint64_t, Row>;

class Screen {
    public:
        void set(uint16_t x, uint16_t y);
        bool get(uint16_t x, uint16_t y) const;
        void display_byte(uint8_t x, uint8_t y, uint8_t byte);
        void draw();
        void clear();

    private:
        Matrix2D<SCREEN_HEIGHT_CHIP> screen {};
};