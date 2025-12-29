#include <iostream>
#include "welcome.h"

void showWelcomeScreen() {
    std::cout << "\n==============================\n";
    std::cout << "      RETROCALC (C++)         \n";
    std::cout << "==============================\n";
    std::cout << "\nCross-platform C++ starter\n";
    std::cout << "Compatible with Linux, Unix, Mac, Windows\n";
    std::cout << "\nPress ENTER to continue...\n";
    std::cin.get();
}
