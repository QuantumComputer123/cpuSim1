#include <iostream>
#include <vector>
#include <bitset>
#include <array>

using std::vector, std::array, std::bitset;

typedef u_char byte;

/* INSTRUCTIONS

MEM is 2 bytes long, pass in as 2 values

0x00 MOV: REG1, REG2 : REG1 <- REG2
0x01 LD: REG1, MEM : REG1 <- MEM
0x02 LDI: REG1, NUM : REG1 <- NUM
0x03 ST: MEM, REG1 : MEM <- REG1
0x04 STI: MEM, NUM : MEM <- NUM
0x05 NOT: REG1, REG2 : REG1 <- !REG2
0x06 ADD: REG1, REG2, REG3 : REG1 <- REG2 + REG3
0x07 SUB: REG1, REG2, REG3 : REG1 <- REG2 - REG3
0x08 MUL: REG1, REG2, REG3 : REG1 <- REG2 * REG3
0x09 DIV: REG1, REG2, REG3 : REG1 <- REG2 / REG3
0x0A AND: REG1, REG2, REG3 : REG1 <- REG2 & REG3
0x0B OR: REG1, REG2, REG3 : REG1 <- REG2 | REG3
0x0C XOR: REG1, REG2, REG3 : REG1 <- REG2 ^ REG3

0xFE NOP
0xFF HALT
*/

byte bitsetToByte(bitset<8> bitset) {
    byte b = bitset.to_ulong();
    return b;
}

bitset<8> byteToBitset(byte b) {
    bitset<8> bitset = b;
    return bitset;
}

char intToChar(int i) {
    char c = i % 128;
    return c;
}

int numArgs(int op) {
    switch (op)
    {
    case 0:
    case 2:
    case 5:
        return 2;
        break;
    case 1:
    case 3:
    case 4:
        return 3;
        break;
    case 254:
    case 255:
        return 1;
    default:
        if (op >= 6 && op <= 13) return 3;
        return 0;
        break;
    }
}

bool debug = true;

int executeStep(vector<byte>& instructions, array<byte, 65536>& memory, array<byte, 16>& registers) {
    byte pc = registers[15];
    byte currentInstruction = instructions[pc];
    if (debug) {
        std::cout << "current instruction " << (int)currentInstruction << '\n';
        std::cout << "r1: " << (int)registers[instructions[pc+1]] << '\n';
        std::cout << "r2: " << (int)registers[instructions[pc+2]] << '\n';
    }
    switch (currentInstruction) {
        case 0: registers[instructions[pc+1]] = registers[instructions[pc+2]]; break; // MOV
        case 1: registers[instructions[pc+1]] = memory[instructions[pc+2] + (256 * instructions[pc+3])]; break; // LD
        case 2: registers[instructions[pc+1]] = instructions[pc+2]; break; // LDI
        case 3: memory[instructions[pc+1] + (256 * instructions[pc+2])] = registers[instructions[pc+3]]; break; // ST
        case 4: memory[instructions[pc+1] + (256 * instructions[pc+2])] = instructions[pc+3]; break; // STI
        case 5: registers[instructions[pc+1]] = bitsetToByte(byteToBitset(registers[instructions[pc+2]]).flip()); break; // NOT
        case 6: registers[instructions[pc+1]] = intToChar(registers[instructions[pc+2]] + registers[instructions[pc+3]]); break; // ADD
        case 7: registers[instructions[pc+1]] = intToChar(registers[instructions[pc+2]] - registers[instructions[pc+3]]); break; // SUB
        case 8: registers[instructions[pc+1]] = intToChar(registers[instructions[pc+2]] * registers[instructions[pc+3]]); break; // MUL
        case 9: registers[instructions[pc+1]] = intToChar(registers[instructions[pc+2]] / registers[instructions[pc+3]]); break; // DIV
        case 10: registers[instructions[pc+1]] = bitsetToByte(byteToBitset(registers[instructions[pc+2]]) & byteToBitset(registers[instructions[pc+3]])); break; // AND
        case 11: registers[instructions[pc+1]] = bitsetToByte(byteToBitset(registers[instructions[pc+2]]) | byteToBitset(registers[instructions[pc+3]])); break; // OR
        case 12: registers[instructions[pc+1]] = bitsetToByte(byteToBitset(registers[instructions[pc+2]]) ^ byteToBitset(registers[instructions[pc+3]])); break; // XOR
        case 254: break; // NOP
        case 255: return 1; break; // HALT, successful execution
        default: return 2; break; // syntax error
    }
    pc += numArgs(currentInstruction);
    ++pc;
    registers[15] = pc;
    registers[0] = 0;
    return 0;
}

int main() { 
    vector<byte> instructions = { 0x02, 0x01, 0x15, 0xFF };
    array<byte, 65536> memory = {};
    array<byte, 16> registers = {}; // R0 is always 0, RF (R15) is the pc

    bool running = true;

    while (running) {
        if (executeStep(instructions, memory, registers) != 0) {
            running = false;
        }
    }

    if (debug) {
        int i = 0;
        for (byte r : registers) { std::cout << "register " << i << ": " << (int)r << '\n'; i++;}
    }

    return 0;
}