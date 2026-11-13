#pragma once

#include <string>
#include <expected>

#include "constants.hpp"
#include "screen.hpp"


// Memory Layout:
// 0x000 - 0x004 - Sprite 0
// 0x010 - 0x014 - Sprite 1
// 0x020 - 0x024 - Sprite 2
// ...
// 0x0F0 - 0x0F4 - Sprite F
// 0x200 - ... => program from ROM


// REFACTOR: maybe extract memory into a separate composed class?
class Chip8Machine {
    public:
        Chip8Machine(std::string rom_path);

        uint8_t read_byte(uint16_t address);
        void write_byte(uint16_t address, uint8_t byte);

        void dump_region(uint16_t start_addr, uint16_t end_addr);
        void dump_registers();
        void draw_screen();
        void clear_screen();

        std::expected<void, std::string> execute_next();

        // Command execution methods
        void ret();

        uint16_t get_rom_size() {
            return this->rom_size;
        }
    private:
        uint16_t sprite_address(uint8_t sprite_pos);
        void write_sprite(uint16_t address, const uint8_t* bytes, uint8_t size);
        uint8_t nth_digit(uint16_t source, uint8_t n);

        std::expected<void, std::string> read_from_file(std::string rom_path);

        std::expected<void, std::string> execute_instruction(uint16_t instruction);
        std::expected<void, std::string> execute_0s(uint16_t instruction);
        std::expected<void, std::string> execute_1s(uint16_t instruction);
        std::expected<void, std::string> execute_2s(uint16_t instruction);
        std::expected<void, std::string> execute_3s(uint16_t instruction);
        std::expected<void, std::string> execute_4s(uint16_t instruction);
        std::expected<void, std::string> execute_5s(uint16_t instruction);
        std::expected<void, std::string> execute_6s(uint16_t instruction);
        std::expected<void, std::string> execute_7s(uint16_t instruction);
        std::expected<void, std::string> execute_8s(uint16_t instruction);
        std::expected<void, std::string> execute_9s(uint16_t instruction);
        std::expected<void, std::string> execute_as(uint16_t instruction);
        std::expected<void, std::string> execute_bs(uint16_t instruction);
        std::expected<void, std::string> execute_cs(uint16_t instruction);
        std::expected<void, std::string> execute_ds(uint16_t instruction);
        std::expected<void, std::string> execute_es(uint16_t instruction);
        std::expected<void, std::string> execute_fs(uint16_t instruction);

        uint8_t v_registers[16] {};
        uint16_t i_register {};
        uint16_t pc_register {};
        uint8_t sp {};
        uint16_t stack[16] {};
        uint8_t dt_register {};
        uint8_t st_register {};

        uint8_t memory[4 * KB] {};
        uint16_t program_pointer { 0x200 };

        size_t rom_size;

        Screen screen;
};