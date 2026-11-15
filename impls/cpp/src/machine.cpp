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
    uint8_t byte = 0; bool bad = false;
    for(uint16_t ptr = 0x200; !bad; ptr++) {
        ifrom.read(reinterpret_cast<char*>(&byte), sizeof(byte));
        bad = ifrom.fail();
        if(!bad) {
            this->memory[ptr] = byte;
        }
    }
    ifrom.close();

    return {};
}

uint8_t Chip8Machine::nth_digit(uint16_t source, uint8_t n) {
    uint8_t mask = 0xF;
    return (source >> 4 * n) & mask;
}

Chip8Machine::Chip8Machine(std::string rom_path)
    : rng_gen(std::random_device {}()),
      dist(0, 255) {
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
        std::print("V{:1X} = {:02X}; ", i, this->v_registers[i]);
    }
    std::println("\nI = {:04X}", this->i_register);
    std::println("PC = {:04X}", this->pc_register);
    std::println("SP = {:02X}", this->sp);
    std::print("stack: [ ");
    for(int i = 0; i < 16; i++) {
        std::print("{:04X} ", this->stack[i]);
    }
    std::println("]");
    std::println("DT = {:02X}", this->dt_register);
    std::println("ST = {:02X}", this->st_register);
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

uint8_t Chip8Machine::random_byte() {
    return this->dist(this->rng_gen);
}

std::expected<InstructionType, std::string> Chip8Machine::execute_instruction(uint16_t instruction) {
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

std::expected<InstructionType, std::string> Chip8Machine::execute_next() {
    uint16_t instruction = this->memory[this->pc_register] << 8 | this->memory[this->pc_register + 1];
    //std::println("Byte 0: {:02X}; Byte 1: {:02X};", this->memory[this->pc_register], this->memory[this->pc_register + 1]);
    std::println("=> Executing instr: [{:04X}];", instruction);
    auto result = this->execute_instruction(instruction);

    if(this->pc_register >= 0xFFE) {
        std::println("FATAL: PC OVERFLOW, REACHED END OF MEMORY, ABORTING!");
        std::abort();
    }

    if(result.has_value() && result.value() == InstructionType::Linear) {
        this->pc_register += 2;
    }
    return result;
}

std::expected<InstructionType, std::string> Chip8Machine::execute_0s(uint16_t instruction) {
    if(instruction == 0x00E0) {
        this->clear_screen();
        return { InstructionType::Linear };
    } else if (instruction == 0x00EE) {
        this->ret();
        return { InstructionType::ControlFlow };
    } else {
        return { InstructionType::HALT };
    }
}

// Only one instruction possible on 1s - Jump to addr.
std::expected<InstructionType, std::string> Chip8Machine::execute_1s(uint16_t instruction) {
    uint16_t addr = instruction & 0x0FFF; // need first 3 digits of the opcode
    std::println("@@@ Instr: {:04X}; addr: {:04X};", instruction, addr);
    this->pc_register = addr;
    std::println("@@@ PC set to: {:04X};", this->pc_register);
    return { InstructionType::ControlFlow };
}

// Only one instruction possible on 2s - CALL addr
std::expected<InstructionType, std::string> Chip8Machine::execute_2s(uint16_t instruction) {
    uint16_t addr = instruction & 0x0FFF; // need first 3 digits for addr to call
    this->stack[++this->sp] = this->pc_register + 2;
    this->pc_register = addr;
    return { InstructionType::ControlFlow };
}

std::expected<InstructionType, std::string> Chip8Machine::execute_3s(uint16_t instruction) {
    uint8_t rgstr = this->nth_digit(instruction, 2);
    uint8_t comparison_byte = instruction & 0x00FF;
    if(this->v_registers[rgstr] == comparison_byte) {
        this->pc_register += 2;
        std::println("Vx == KK, skipping next instruction!");
    } else {
        std::println("Vx != KK, not skipping the next instruction.");
    }
    return { InstructionType::Linear };
}

std::expected<InstructionType, std::string> Chip8Machine::execute_4s(uint16_t instruction) {
    uint8_t rgstr = this->nth_digit(instruction, 2);
    uint8_t comparison_byte = instruction & 0x00FF;
    if(this->v_registers[rgstr] != comparison_byte) {
        this->pc_register += 2;
        std::println("[SNE] Vx != KK, skipping next instruction!");
    } else {
        std::println("[SNE] Vx == KK, not skipping the next instruction.");
    }
    return { InstructionType::Linear };
}

std::expected<InstructionType, std::string> Chip8Machine::execute_5s(uint16_t instruction) {
    uint8_t vx = this->nth_digit(instruction, 2);
    uint8_t vy = this->nth_digit(instruction, 1);
    uint8_t control_zero = this->nth_digit(instruction, 0);
    if(this->v_registers[vx] == this->v_registers[vy]) {
        this->pc_register += 2;
        std::println("[5xy0] Vx == Vy, skipping next instruction!");
    } else {
        std::println("[5xy0] Vx != Vy, not skipping the next instruction.");
    }

    if (control_zero == 0) {
        return { InstructionType::Linear };
    }
    else {
        return std::unexpected { "WARNING: instruction 5xy0, expected last digit to be 0!" };
    }
}

std::expected<InstructionType, std::string> Chip8Machine::execute_6s(uint16_t instruction) {
    uint8_t vx = this->nth_digit(instruction, 2);
    uint8_t new_vx_value = instruction & 0x00FF;
    this->v_registers[vx] = new_vx_value;
    std::println("[ASSIGN] V{:01X} = {:02X};", vx, new_vx_value);
    return { InstructionType::Linear };
}

std::expected<InstructionType, std::string> Chip8Machine::execute_7s(uint16_t instruction) {
    uint8_t vx = this->nth_digit(instruction, 2);
    uint8_t vx_increment = instruction & 0x00FF;
    this->v_registers[vx] += vx_increment;
    std::println("[INCREMENT] V{:01X} += {:02X};", vx, vx_increment);
    return { InstructionType::Linear };
}

std::expected<InstructionType, std::string> Chip8Machine::execute_8s(uint16_t instruction) {
    return std::unexpected { "NOT IMPLEMENTED YET" };
}

std::expected<InstructionType, std::string> Chip8Machine::execute_9s(uint16_t instruction) {
    uint8_t vx = this->nth_digit(instruction, 2);
    uint8_t vy = this->nth_digit(instruction, 1);
    uint8_t control_zero = this->nth_digit(instruction, 0);
    if(this->v_registers[vx] != this->v_registers[vy]) {
        this->pc_register += 2;
        std::println("[9xy0] Vx != Vy, skipping next instruction!");
    } else {
        std::println("[9xy0] Vx == Vy, not skipping the next instruction.");
    }

    if (control_zero == 0) {
        return { InstructionType::Linear };
    }
    else {
        return std::unexpected { "WARNING: instruction 9xy0, expected last digit to be 0!" };
    }
}

std::expected<InstructionType, std::string> Chip8Machine::execute_as(uint16_t instruction) {
    uint16_t new_i_val = instruction & 0x0FFF;
    this->i_register = new_i_val;
    std::println("[ASSIGN] I = {:03X};", new_i_val);
    return { InstructionType::Linear };
}

std::expected<InstructionType, std::string> Chip8Machine::execute_bs(uint16_t instruction) {
    uint16_t addr = instruction &0x0FFF;
    this->pc_register = addr + this->v_registers[0];
    std::println("[B{:03X}] PC = {:03X} + V0 = {:03X} + {:02X} = {:04X};", addr, addr, addr, this->v_registers[0], this->pc_register);
    return { InstructionType::ControlFlow };
}

std::expected<InstructionType, std::string> Chip8Machine::execute_cs(uint16_t instruction) {
    uint8_t vx = this->nth_digit(instruction, 2);
    uint8_t and_byte = instruction & 0x00FF;
    uint8_t random_byte = this->random_byte();
    this->v_registers[vx] = random_byte & and_byte;
    std::println("[C] V{:01X} = RNG[{:02X}] & {:02X} = {:02X};", vx, random_byte, and_byte, this->v_registers[vx]);
    return { InstructionType::Linear };
}

// The only possible instruction for D is Dxyn, which is
// "Display n-byte sprite at memory location I, coordinates (Vx, Vy)".
std::expected<InstructionType, std::string> Chip8Machine::execute_ds(uint16_t instruction) {
    uint8_t vx = this->nth_digit(instruction, 2);
    uint8_t vy = this->nth_digit(instruction, 1);
    uint8_t sprite_size = this->nth_digit(instruction, 0);
    return { InstructionType::Linear };
}

std::expected<InstructionType, std::string> Chip8Machine::execute_es(uint16_t instruction) {
    return std::unexpected { "NOT IMPLEMENTED YET" };
}

std::expected<InstructionType, std::string> Chip8Machine::execute_fs(uint16_t instruction) {
    return std::unexpected { "NOT IMPLEMENTED YET" };
}
