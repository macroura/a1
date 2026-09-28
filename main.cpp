// main.cpp - input loop and output formatting for extractIPv4.

#include "ipv4.h"

#include <iostream>
#include <string>

int main() {
    std::string line;

    while (true) {
        std::cout << "Enter a string (or 'END' to quit): ";
        if (!std::getline(std::cin, line)) break;              // EOF: quit cleanly
        if (!line.empty() && line.back() == '\r') line.pop_back();  // CRLF input

        if (line == "END") break;                              // case-sensitive

        unsigned long address = 0;
        int port = -1;

        if (extractIPv4(line, address, port)) {
            std::cout << "Extracted IPv4 address: "
                      << ((address >> 24) & 0xFF) << '.'
                      << ((address >> 16) & 0xFF) << '.'
                      << ((address >> 8) & 0xFF) << '.'
                      << (address & 0xFF)
                      << " (decimal value: " << address << ", port: ";
            if (port < 0) std::cout << "none";
            else          std::cout << port;
            std::cout << ")\n";
        } else {
            std::cout << "Invalid input: no valid IPv4 address found\n";
        }
    }

    std::cout << "Program terminated.\n";
    return 0;
}
