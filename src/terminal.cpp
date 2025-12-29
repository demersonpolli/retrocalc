#include "terminal.h"

#ifdef _WIN32
#include <conio.h>
#include <windows.h>

static DWORD originalMode;
static HANDLE hStdin;

void initTerminal() {
    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(hStdin, &originalMode);
    SetConsoleMode(hStdin, originalMode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT));
}

void restoreTerminal() {
    SetConsoleMode(hStdin, originalMode);
}

int getKey() {
    int c = _getch();
    if (c == 0 || c == 224) {
        c = _getch();
        switch (c) {
            case 72: return KEY_ARROW_UP;
            case 80: return KEY_ARROW_DOWN;
            case 75: return KEY_ARROW_LEFT;
            case 77: return KEY_ARROW_RIGHT;
            case 59: return KEY_F1;
        }
    }
    return c;
}

#else
#include <termios.h>
#include <unistd.h>

static struct termios originalTermios;

void initTerminal() {
    tcgetattr(STDIN_FILENO, &originalTermios);
    struct termios raw = originalTermios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void restoreTerminal() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalTermios);
}

int getKey() {
    unsigned char c;
    if (read(STDIN_FILENO, &c, 1) == 1) {
        if (c == 27) {
            unsigned char seq[2];
            if (read(STDIN_FILENO, &seq[0], 1) != 1) return KEY_ESC;
            if (read(STDIN_FILENO, &seq[1], 1) != 1) return KEY_ESC;
            if (seq[0] == '[') {
                switch (seq[1]) {
                    case 'A': return KEY_ARROW_UP;
                    case 'B': return KEY_ARROW_DOWN;
                    case 'C': return KEY_ARROW_RIGHT;
                    case 'D': return KEY_ARROW_LEFT;
                }
                if (seq[1] == '[') {
                    unsigned char fkey;
                    if (read(STDIN_FILENO, &fkey, 1) == 1) {
                        if (fkey == 'A') return KEY_F1;
                    }
                }
            } else if (seq[0] == 'O') {
                if (seq[1] == 'P') return KEY_F1;
            }
            return KEY_ESC;
        }
        return c;
    }
    return -1;
}

#endif
