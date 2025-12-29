#pragma once

void initTerminal();
void restoreTerminal();
int getKey();

constexpr int KEY_ESC = 27;
constexpr int KEY_ARROW_UP = 1000;
constexpr int KEY_ARROW_DOWN = 1001;
constexpr int KEY_ARROW_RIGHT = 1002;
constexpr int KEY_ARROW_LEFT = 1003;
constexpr int KEY_F1 = 1010;
constexpr int KEY_F2 = 1011;
