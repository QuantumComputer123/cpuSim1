#include <iostream>
#include <vector>
#include <bitset>
#include <array>
#include <fstream>
#include <iterator>
#include <string>
#include "raylib.h"
#include <algorithm>

using std::vector, std::array, std::bitset, std::string;

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
0x0D JMP: NUM : PC <- NUM
0x0E JZ: NUM : ZERO ? PC <- NUM
0x0F JNZ: NUM : !ZERO ? PC <- NUM

0xFE NOP
0xFF HALT

cd build && ../assembler ../test.sim && ./main ../test.bin
*/

const bool debug = false;
const int screenWidth = 32;
const int screenHeight = 24;
const int pixelSize = 35;

vector<byte> readBinaryFile(const string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    
    if (!file.is_open()) {
        std::cout << "no open file\n";
        return {};
    }

    std::streamsize size = file.tellg();
    if (size <= 0) return {};

    file.seekg(0, std::ios::beg);

    vector<byte> buffer(static_cast<size_t>(size));

    if (file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        return buffer;
    } else {
        std::cout << ":(";
    }

    return {};
}

void drawToScreen(array<byte, 65536>& memory) {
    const int videoMemoryStart = 65536 - (screenWidth * screenHeight * 3);

    for (int y = 0; y < screenHeight; y++) {
        for (int x = 0; x < screenWidth; x++) {
            int pixelIdx = videoMemoryStart + ((y * screenWidth + x) * 3);

            DrawRectangle(
                x * pixelSize, 
                y * pixelSize, 
                pixelSize, 
                pixelSize, 
                Color{
                    memory[pixelIdx],     // R
                    memory[pixelIdx + 1], // G
                    memory[pixelIdx + 2], // B
                    255                   // A
                }
            );
        }   
    }
}

const int TARGET_KEYS[] = {
    // Letters A-Z
    KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J,
    KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T,
    KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,
    // Numbers 0-9
    KEY_ZERO, KEY_ONE, KEY_TWO, KEY_THREE, KEY_FOUR, 
    KEY_FIVE, KEY_SIX, KEY_SEVEN, KEY_EIGHT, KEY_NINE,
    // Arrows
    KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT
};

void updatePressedKeys(array<byte, 65536>& memory) {
    for (int i = 0; i < 40; i++) {
        int key = TARGET_KEYS[i];
        
        unsigned char state = 0;
        if (IsKeyDown(key))     state |= 0x8; // 1000
        if (IsKeyPressed(key))  state |= 0x4; // 0100
        if (IsKeyReleased(key)) state |= 0x2; // 0010
        if (IsKeyUp(key))       state |= 0x1; // 0001

        int byteIndex = i / 2;
        if (i % 2 == 0) {
            memory[byteIndex] = (state << 4); 
        } else {
            memory[byteIndex] |= (state & 0x0F);
        }
    }
}

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
        if (op >= 13 && op <= 15) return 1;
        return 0;
        break;
    }
}

void updateFlags(bitset<8>& flags, byte registerNum) {
    bitset<8> reg = byteToBitset(registerNum);

    if (reg.to_ulong() == 0) flags.set(0, true);
    if (reg.to_ulong() != 0) flags.set(0, false);
}

int executeStep(vector<byte>& instructions, array<byte, 65536>& memory, array<byte, 16>& registers, int& pc, bitset<8>& flags) {
    if (instructions.empty()) {
        std::cout << "Error: No instructions loaded\n";
        return 2;
    }
    byte currentInstruction = instructions[pc];

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
        case 13: pc = instructions[pc+1]; return 0; break;
        case 14: if (flags.test(0)) { pc = instructions[pc+1]; return 0; break; }
        case 15: if (!flags.test(0)) { pc = instructions[pc+1]; return 0; break; }
        case 254: break; // NOP
        case 255: return 1; break; // HALT, successful execution
        default: return 2; break; // syntax error
    }
    if (currentInstruction != 3 && currentInstruction != 4 && currentInstruction < 13) {
        (flags, registers[instructions[pc+1]]);
    }
    pc += numArgs(currentInstruction);
    ++pc;
    registers[0] = 0;
    return 0;
}

int main(int argc, char *argv[]) { 
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <binary_file>" << std::endl;
        return 1;
    }

    InitWindow(screenWidth * pixelSize, screenHeight * pixelSize, ":D");
    SetTargetFPS(60);

    vector<byte> instructions = readBinaryFile(argv[1]);
    array<byte, 65536> memory = {};
    array<byte, 16> registers = {}; 
    bitset<8> flags = 0;

    if (instructions.empty()) {
        std::cerr << "Error: Could not read " << argv[1] << " or file is empty." << std::endl;
        return 1;
    }

    float lastCpuTime = 0;
    float cpuInterval = 1.0f / 10.0f;

    float lastScreenTime = 0;
    float screenInterval = 1.0f;

    int pc = 0;

    BeginDrawing();
    ClearBackground(BLACK);
    drawToScreen(memory);
    EndDrawing();

    while (!WindowShouldClose()) {
        float currentTime = GetTime();

        if (currentTime - lastCpuTime >= cpuInterval) {
            executeStep(instructions, memory, registers, pc, flags);
            updatePressedKeys(memory);
            lastCpuTime = currentTime;
        }

        if (currentTime - lastScreenTime >= screenInterval) {
            BeginDrawing();
                ClearBackground(BLACK);
                drawToScreen(memory);
            EndDrawing();
            lastScreenTime = currentTime;
        }
    }

    if (debug) {
        int i = 0;
        for (byte r : registers) { 
            std::cout << "register " << i << ": " << (int)r << '\n'; 
            i++;
        }
    }

    CloseWindow();
    return 0;
}