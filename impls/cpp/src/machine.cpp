#include <bit>
#include <cassert>
#include <print>

#include "machine.hpp"


uint8_t Chip8Machine::nth_digit(uint16_t source, uint8_t n) {
    uint8_t mask = 0xF;
    return (source >> 4 * n) & mask;
}

Chip8Machine::Chip8Machine() {
    for(uint8_t sprite = 0; sprite <= 0xF; sprite++) {
        uint16_t sprite_address = this->sprite_address(sprite);
        std::println("SPRITE {} ADDRESS: {:04X}", sprite, sprite_address);
        this->write_sprite(sprite_address, &SPRITES[sprite][0], PREDEFINED_SPRITE_SIZE);
    }
}

uint8_t Chip8Machine::read_byte(uint16_t address) {
    assert(address <= 0xFFF && "Address to read out of bounds.");
    return std::byteswap(this->memory[address]);
}

void Chip8Machine::write_byte(uint16_t address, uint8_t byte) {
    assert(address <= 0xFFF && "Address to write out of bounds.");
    this->memory[address] = std::byteswap(byte);
}

void Chip8Machine::dump_region(uint16_t start_addr, uint16_t end_addr) {
    assert(start_addr <= 0xFFF && end_addr <= 0xFFF && "Addresses to dump out of bounds.");
    assert(start_addr <= end_addr && "Start address higher than end address.");

    // clear least 4 bits, start from 0xXX0
    uint16_t dump_start = (start_addr >> 4) << 4;
    // we need to complement the end address to the 0xXXF state
    uint16_t end_mask = 0xF - (end_addr & 0xF);
    uint16_t dump_end = end_addr + end_mask;

    for(int i = dump_start; i <= dump_end; i++) {
        if(nth_digit(i, 0) == 0) {
            std::print("\n{:02X} - {:02X}:  ", i, i + 0xF);
        }

        std::print("{:02X}  ", this->read_byte(i));

        if(nth_digit(i, 0) == 0xF) {
            std::print("\n");
        } else if(nth_digit(i, 0) == 7) {
            std::print("\t");
        }
    }
}

uint16_t Chip8Machine::sprite_address(uint8_t sprite_pos) {
    assert((sprite_pos >= 0x0 && sprite_pos <= 0xF) && "Only sprites 0 - F are available.");

    return sprite_pos << 4;
}

void Chip8Machine::write_sprite(uint16_t address, const uint8_t* bytes, uint8_t size) {
    assert(size > 0 && size <= 0xF && "Size of sprite can only be 0-15.");
    for(int i = 0; i < size; i++) {
        this->write_byte(address + i, bytes[i]);
    }
}

