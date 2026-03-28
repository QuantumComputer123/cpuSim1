#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <sstream> // Required for stringstream parsing

using std::string;

const int memSize = 65536;
const int vMemSize = 24 * 32 * 3;

std::unordered_map<string, uint8_t> opcodes = {
    {"MOV",  0x00}, {"LD",   0x01}, {"LDI",  0x02},
    {"ST",   0x03}, {"STI",  0x04}, {"NOT",  0x05},
    {"ADD",  0x06}, {"SUB",  0x07}, {"MUL",  0x08},
    {"DIV",  0x09}, {"AND",  0x0A}, {"OR",   0x0B},
    {"XOR",  0x0C}, {"JMP",  0x0D}, {"JZ",   0x0E},
    {"JNZ",  0x0F}, {"NOP",  0xFE}, {"HALT", 0xFF}
};

int keywordToInt(string keyword) {
    if (keyword.empty()) return -1;

    if (keyword[0] == 'r' || keyword[0] == 'R') {
        try {
            return std::stoi(keyword.substr(1), 0);
        } catch (...) { return -1; }
    }

    if (keyword[0] == 'm' || keyword[0] == 'M') {
        try {
            return std::stoi("0x" + keyword.substr(1), nullptr, 16);
        } catch (...) { return -1; }
    }

    if (keyword[0] == 'x' || keyword[0] == 'X') {
        try {
            return std::stoi("0x" + keyword.substr(1), nullptr, 16);
        } catch (...) { return -1; }
    }

    if (keyword[0] == 'd' || keyword[0] == 'D') {
        try {
            return std::stoi(keyword.substr(1));
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
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename.sim>" << std::endl;
        return 1;
    }

    string inputPath = argv[1];
    if (inputPath.length() < 5) {
        std::cerr << "Error: Invalid filename (too short)" << std::endl;
        return 1;
    }

    std::ifstream inFile(inputPath);
    string outputFilename = inputPath.substr(0, inputPath.length() - 4).append(".bin");
    std::ofstream outFile(outputFilename, std::ios::out | std::ios::binary);

    if (!inFile.is_open()) {
        std::cerr << "Error: Could not open .sim file" << std::endl;
        return 1;
    }
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not create .bin file" << std::endl;
        return 1;
    }

    string line;
    while (std::getline(inFile, line)) {
        size_t commentPos = line.find(";");
        if (commentPos != string::npos) {
            line = line.substr(0, commentPos);
        }

        std::stringstream ss(line);
        string word;
        while (ss >> word) {
            int value = keywordToInt(word);
            
            if (value != -1) {
                uint8_t byteOut = static_cast<uint8_t>(value);
                outFile.write(reinterpret_cast<const char*>(&byteOut), sizeof(byteOut));
                std::cout << "Assembled: " << word << " -> " << (int)byteOut << std::endl;
            }
        }
    }

    std::cout << "Assembly complete. Check " << outputFilename << std::endl;

    inFile.close();
    outFile.close();
    return 0;
}