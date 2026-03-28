#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>
#include <unordered_map>

using std::string;

const int memSize = 65536;
const int vMemSize = 2304;

std::unordered_map<string, uint8_t> opcodes = {
    {"MOV",  0x00}, {"LD",   0x01}, {"LDI",  0x02},
    {"ST",   0x03}, {"STI",  0x04}, {"NOT",  0x05},
    {"ADD",  0x06}, {"SUB",  0x07}, {"MUL",  0x08},
    {"DIV",  0x09}, {"AND",  0x0A}, {"OR",   0x0B},
    {"XOR",  0x0C}, {"NOP",  0xFE}, {"HALT", 0xFF}
};

int keywordToInt(string keyword) {
    if (keyword.empty()) return -1;

    if (keyword[0] == 'r' || keyword[0] == 'R') {
        try {
            return std::stoi(keyword.substr(1), 0);
        } catch (...) { return -1; }
    }

    if (keyword[0] == 'x' || keyword[0] == 'X') {
        try {
            return std::stoi("0x" + keyword.substr(1), nullptr, 16);
        } catch (...) { return -1; }
    }

    auto it = opcodes.find(keyword);
    if (it != opcodes.end()) {
        return it->second;
    }

    std::cerr << "Syntax Error: Unknown word '" << keyword << "'" << std::endl;
    return -1;
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        if (((string)argv[1]).length() < 5) {
            std::cerr << "error with filename\n";
        }
        std::ifstream inFile(argv[1]);
        string outputFilename = ((string)argv[1]).substr(0, ((string)argv[1]).length() - 4).append(".bin");
        std::ofstream outFile(outputFilename, std::ios::out | std::ios::binary);

        if (!inFile.is_open()) {
            std::cerr << "Error: Could not open .sim file" << std::endl;
            return 1;
        }
        if (!outFile.is_open()) {
            std::cerr << "Error: Could not create .bin file" << std::endl;
            return 1;
        }

        string word;
        while (inFile >> word) {
            int value = keywordToInt(word);
            
            if (value != -1) {
                uint8_t byteOut = static_cast<uint8_t>(value);
                outFile.write(reinterpret_cast<const char*>(&byteOut), sizeof(byteOut));
                std::cout << "Assembled: " << word << " -> " << (int)byteOut << std::endl;
            }
        }

        std::cout << "Assembly complete. Check output.bin" << std::endl;

        inFile.close();
        outFile.close();
        return 0;
    }
}