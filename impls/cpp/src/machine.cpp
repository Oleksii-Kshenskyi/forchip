#include <bit>
#include <cassert>
#include <expected>
#include <print>
#include <filesystem>
#include <format>
#include <fstream>
#include <cstdlib>

#include "machine.hpp"
#include "constants.hpp"

std::expected<void, std::string> Chip8Machine::read_from_file(std::string rom_path) {
    if(!std::filesystem::exists(rom_path)) {
        return std::unexpected { std::format("ERROR: ROM path {} doesn\'t exist.", rom_path) };
    }
    this->rom_size = std::filesystem::file_size(rom_path);
    if( this->rom_size > MAX_ROM_SIZE) {
        return std::unexpected { std::format("ERROR: Expected max ROM size <= {}, got a {} bytes long file.", MAX_ROM_SIZE, rom_size) };
    }

    std::ifstream ifrom { rom_path, std::ios::in | std::ios::binary };
    uint8_t buffer_num;
    for(uint16_t ptr = 0x200; ifrom >> buffer_num; ptr++) {
        this->memory[ptr] = buffer_num;
    }
    ifrom.close();

    return {};
}

uint8_t Chip8Machine::nth_digit(uint16_t source, uint8_t n) {
    uint8_t mask = 0xF;
    return (source >> 4 * n) & mask;
}

Chip8Machine::Chip8Machine(std::string rom_path) {
    for(uint8_t sprite = 0; sprite <= 0xF; sprite++) {
        uint16_t sprite_address = this->sprite_address(sprite);
        this->write_sprite(sprite_address, &SPRITES[sprite][0], PREDEFINED_SPRITE_SIZE);
    }

    auto result = this->read_from_file(rom_path);
    if(!result.has_value()) {
        std::println("{}", result.error());
        std::exit(-1);
    }

    this->screen.set(0, 0);
    this->screen.set(2, 0);
    this->screen.set(63, 0);
    this->screen.set(0, 31);
    this->screen.set(63, 31);
    this->screen.set(32, 1);
    this->stack[2] = 0x0B0B;
    this->sp = 2;
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

void Chip8Machine::dump_registers() {
    std::println("========= REGISTER STATE ==============");
    std::print("Vs: ");
    for(int i = 0; i < 16; i++) {
        std::print("V{:1X} = {}; ", i, this->v_registers[i]);
    }
    std::println("\nI = {}", this->i_register);
    std::println("PC = {}", this->pc_register);
    std::println("SP = {}", this->sp);
    std::print("stack: [ ");
    for(int i = 0; i < 16; i++) {
        std::print("{} ", this->stack[i]);
    }
    std::println("]");
    std::println("DT = {}", this->dt_register);
    std::println("ST = {}", this->st_register);
    std::println("LOADED A ROM of {} BYTES.", this->rom_size);

    std::println("======== END REGISTER STATE ============\n");
}

void Chip8Machine::draw_screen() {
    this->screen.draw();
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

void Chip8Machine::clear_screen() {
    this->screen.clear();
}

void Chip8Machine::ret() {
    auto top_of_stack = this->stack[this->sp];
    this->pc_register = top_of_stack;
    assert(sp > 0 && "Tried to decrement stack pointer, but SP == 0!");
    this->sp -= 1;
    
}

std::expected<void, std::string> Chip8Machine::execute_instruction(uint16_t instruction) {
    uint8_t split_digit = this->nth_digit(instruction, 3);
    switch(split_digit) {
        case 0:
            return this->execute_0s(instruction);
        case 1:
            return this->execute_1s(instruction);
        case 2:
            return this->execute_2s(instruction);
        case 3:
            return this->execute_3s(instruction);
        case 4:
            return this->execute_4s(instruction);
        case 5:
            return this->execute_5s(instruction);
        case 6:
            return this->execute_6s(instruction);
        case 7:
            return this->execute_7s(instruction);
        case 8:
            return this->execute_8s(instruction);
        case 9:
            return this->execute_9s(instruction);
        case 0xA:
            return this->execute_as(instruction);
        case 0xB:
            return this->execute_bs(instruction);
        case 0xC:
            return this->execute_cs(instruction);
        case 0xD:
            return this->execute_ds(instruction);
        case 0xE:
            return this->execute_es(instruction);
        case 0xF:
            return this->execute_fs(instruction);
        default:
            return std::unexpected { "Chip8Machine::execute_instruction(): UNREACHABLE: switched on one digit, yet digit outside of 0-F range..." };
    }
}

std::expected<void, std::string> Chip8Machine::execute_next() {
    uint16_t instruction = this->memory[this->pc_register] << 8 | this->memory[this->pc_register + 1];
    std::println("=> Executing instr: [{:04X}];", instruction);
    auto result = this->execute_instruction(instruction);
    if(this->pc_register >= 0xFFE) {
        std::println("FATAL: PC OVERFLOW, REACHED END OF MEMORY, ABORTING!");
        std::abort();
    }
    // FIXME: in cases when PC needs to be set to a specific value during an instruction,
    //        like RET setting PC to the stack value, this increment breaks execution flow.
    this->pc_register += 2;
    return result;
}

std::expected<void, std::string> Chip8Machine::execute_0s(uint16_t instruction) {
    if(instruction == 0x00E0) {
        this->clear_screen();
        return {};
    } else if (instruction == 0x00EE) {
        this->ret();
        return {};
    } else {
        return std::unexpected { std::format("Old {:4X} SYS instruction called; ignored!", instruction) };
    }
}

std::expected<void, std::string> Chip8Machine::execute_1s(uint16_t instruction) {
    return std::unexpected { "NOT IMPLEMENTED YET" };
}

std::expected<void, std::string> Chip8Machine::execute_2s(uint16_t instruction) {
    return std::unexpected { "NOT IMPLEMENTED YET" };
}

std::expected<void, std::string> Chip8Machine::execute_3s(uint16_t instruction) {
    return std::unexpected { "NOT IMPLEMENTED YET" };
}

std::expected<void, std::string> Chip8Machine::execute_4s(uint16_t instruction) {
    return std::unexpected { "NOT IMPLEMENTED YET" };
}

std::expected<void, std::string> Chip8Machine::execute_5s(uint16_t instruction) {
    return std::unexpected { "NOT IMPLEMENTED YET" };
}

std::expected<void, std::string> Chip8Machine::execute_6s(uint16_t instruction) {
    return std::unexpected { "NOT IMPLEMENTED YET" };
}

std::expected<void, std::string> Chip8Machine::execute_7s(uint16_t instruction) {
    return std::unexpected { "NOT IMPLEMENTED YET" };
}

std::expected<void, std::string> Chip8Machine::execute_8s(uint16_t instruction) {
    return std::unexpected { "NOT IMPLEMENTED YET" };
}

std::expected<void, std::string> Chip8Machine::execute_9s(uint16_t instruction) {
    return std::unexpected { "NOT IMPLEMENTED YET" };
}

std::expected<void, std::string> Chip8Machine::execute_as(uint16_t instruction) {
    return std::unexpected { "NOT IMPLEMENTED YET" };
}

std::expected<void, std::string> Chip8Machine::execute_bs(uint16_t instruction) {
    return std::unexpected { "NOT IMPLEMENTED YET" };
}

std::expected<void, std::string> Chip8Machine::execute_cs(uint16_t instruction) {
    return std::unexpected { "NOT IMPLEMENTED YET" };
}

std::expected<void, std::string> Chip8Machine::execute_ds(uint16_t instruction) {
    return std::unexpected { "NOT IMPLEMENTED YET" };
}

std::expected<void, std::string> Chip8Machine::execute_es(uint16_t instruction) {
    return std::unexpected { "NOT IMPLEMENTED YET" };
}

std::expected<void, std::string> Chip8Machine::execute_fs(uint16_t instruction) {
    return std::unexpected { "NOT IMPLEMENTED YET" };
}
