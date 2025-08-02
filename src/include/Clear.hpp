#pragma once

#include "include.h"

class Clear {
public:
    static void clearScreen() {
        std::cout << "\033[2J\033[H" << std::flush;
    }
};