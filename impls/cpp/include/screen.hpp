#pragma once

#include <array>

#include "constants.hpp"

struct Point {
    Point(uint16_t x, uint16_t y): x(x), y(y) {}
    Point(): x(0), y(0) {}

    uint16_t x;
    uint16_t y;
};

template<typename T, std::size_t Row, std::size_t Col>
using Matrix2D = std::array<std::array<T, Col>, Row>;

class Screen {
    public:
        void set(uint16_t x, uint16_t y);
        bool get(uint16_t x, uint16_t y) const;

    private:
        Matrix2D<uint8_t, SCREEN_WIDTH_CHIP, SCREEN_HEIGHT_CHIP> screen {};
};