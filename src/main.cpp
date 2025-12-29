#include <iostream>
#include <string>
#include <cstdlib>
#include <termios.h>
#include <unistd.h>
#include <algorithm>
#include <fstream>
#include "json.hpp"
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif
#include "spreadsheet.h"
#include "screen.h"

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    std::cout << "\033[2J\033[H";
#endif
}

void hideCursor() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hOut, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hOut, &cursorInfo);
#else
    std::cout << "\033[?25l";
#endif
}

void showCursor() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hOut, &cursorInfo);
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(hOut, &cursorInfo);
#else
    std::cout << "\033[?25h";
#endif
}

void displayWelcomeScreen() {
    clearScreen();
    hideCursor();
    
    std::cout << "\n\n\n";
    std::cout << "                        ================================\n";
    std::cout << "                                 RETROCALC              \n";
    std::cout << "                        ================================\n";
    std::cout << "\n";
    std::cout << "                          Electronic Spreadsheet\n";
    std::cout << "                               Version 1.0\n";
    std::cout << "\n\n";
    std::cout << "                     Copyright (C) 2025 RetroCalc\n";
    std::cout << "\n\n\n";
    std::cout << "                   Press ENTER to continue...\n";
    std::cout << "\n\n";
}

void setRawMode(bool enable) {
    static struct termios oldt, newt;
    if (enable) {
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    } else {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    }
}

static void getTerminalSize(int& rows, int& cols) {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if (GetConsoleScreenBufferInfo(hStdout, &csbi)) {
        cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    } else {
        cols = 80;
        rows = 24;
    }
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        cols = w.ws_col;
        rows = w.ws_row;
    } else {
        cols = 80;
        rows = 24;
    }
#endif
}

int main(int argc, char* argv[]) {
    displayWelcomeScreen();
    std::cin.get();
    showCursor();
    clearScreen();
    Spreadsheet sheet;
    Screen screen;
    int activeRow = 0, activeCol = 0;
    int rowOffset = 0, colOffset = 0;
    setRawMode(true);
    bool editMode = false;
    std::string editBuffer;
    bool isLabel = false;
    bool jumpMode = false;
    std::string jumpBuffer;
    bool commandMode = false;
    std::string commandBuffer;
    bool storageMode = false;
    std::string spreadsheetFileName;
    bool askFileName = false;
    std::string fileNameBuffer;
    bool askLoadFileName = false;
    std::string loadFileNameBuffer;
    while (true) {
        clearScreen();
        int termRows, termCols;
        getTerminalSize(termRows, termCols);
        int minCellWidth = sheet.getMinCellWidth();
        int rowLabelWidth = 5;
        int availableWidth = termCols - rowLabelWidth;
        int visibleCols = availableWidth / minCellWidth;
        if (visibleCols > sheet.getMaxCols()) visibleCols = sheet.getMaxCols();
        if (visibleCols < 1) visibleCols = 1;
        int visibleRows = termRows - 5;
        if (visibleRows > sheet.getMaxRows()) visibleRows = sheet.getMaxRows();
        if (visibleRows < 1) visibleRows = 1;
        if (activeRow < rowOffset) rowOffset = activeRow;
        if (activeRow >= rowOffset + visibleRows) rowOffset = activeRow - visibleRows + 1;
        if (activeCol < colOffset) colOffset = activeCol;
        if (activeCol >= colOffset + visibleCols) colOffset = activeCol - visibleCols + 1;
        screen.drawSpreadsheet(sheet, activeRow, activeCol, rowOffset, colOffset, editMode, editBuffer, isLabel, commandMode, storageMode);
        if (commandMode) {
            char key = getchar();
            if (key == 27) { // ESC to cancel
                commandMode = false;
                storageMode = false;
                commandBuffer.clear();
                continue;
            } else if (key == 127 || key == 8) { // Backspace
                if (!commandBuffer.empty()) commandBuffer.pop_back();
            } else if (isprint(key)) {
                commandBuffer += toupper(key);
                if (!storageMode && toupper(key) == 'S' && commandBuffer.size() == 2) {
                    storageMode = true;
                }
                // Check for /SQ or /S Q
                std::string cmd = commandBuffer;
                cmd.erase(std::remove(cmd.begin(), cmd.end(), ' '), cmd.end());
                if (cmd == "/SQ") {
                    break;
                }
                // /B: clear current cell
                if (cmd == "/B") {
                    sheet.clearCell(activeRow, activeCol);
                    commandMode = false;
                    commandBuffer.clear();
                    continue;
                }
                // /C: clear entire sheet
                if (cmd == "/C") {
                    for (int r = 0; r < sheet.getMaxRows(); ++r)
                        for (int c = 0; c < sheet.getMaxCols(); ++c)
                            sheet.clearCell(r, c);
                    commandMode = false;
                    commandBuffer.clear();
                    continue;
                }
                // /E: edit current cell content
                if (cmd == "/E") {
                    editMode = true;
                    editBuffer = sheet.getCell(activeRow, activeCol).rawContent;
                    isLabel = false;
                    commandMode = false;
                    commandBuffer.clear();
                    continue;
                }
                // /SS: save spreadsheet to JSON
                if (cmd == "/SS") {
                    if (!spreadsheetFileName.empty()) {
                        nlohmann::json j;
                        for (int r = 0; r < sheet.getMaxRows(); ++r) {
                            for (int c = 0; c < sheet.getMaxCols(); ++c) {
                                const Cell& cell = sheet.getCell(r, c);
                                if (cell.type != CellType::EMPTY) {
                                    std::string addr = excelColName(c) + std::to_string(r + 1);
                                    j[addr] = {
                                        {"type", cell.type == CellType::NUMBER ? "number" : cell.type == CellType::TEXT ? "text" : cell.type == CellType::FORMULA ? "formula" : "empty"},
                                        {"raw", cell.rawContent},
                                        {"display", cell.displayValue},
                                        {"value", cell.numericValue}
                                    };
                                }
                            }
                        }
                        std::ofstream out(spreadsheetFileName);
                        out << j.dump(2);
                        out.close();
                        commandMode = false;
                        commandBuffer.clear();
                        continue;
                    } else {
                        askFileName = true;
                        fileNameBuffer.clear();
                        commandMode = false;
                        commandBuffer.clear();
                        continue;
                    }
                }
                // /SL: load spreadsheet from JSON
                if (cmd == "/SL") {
                    askLoadFileName = true;
                    loadFileNameBuffer.clear();
                    commandMode = false;
                    commandBuffer.clear();
                    continue;
                }
            }
            continue;
        }
        if (jumpMode) {
            char key = getchar();
            if (key == '\n' || key == '\r') {
                // Parse jumpBuffer as coordinate
                if (!jumpBuffer.empty()) {
                    // Parse column (letters)
                    int col = 0, i = 0;
                    while (i < (int)jumpBuffer.size() && isalpha(jumpBuffer[i])) {
                        col = col * 26 + (toupper(jumpBuffer[i]) - 'A' + 1);
                        ++i;
                    }
                    col--;
                    // Parse row (digits)
                    int row = 0;
                    while (i < (int)jumpBuffer.size() && isdigit(jumpBuffer[i])) {
                        row = row * 10 + (jumpBuffer[i] - '0');
                        ++i;
                    }
                    if (col >= 0 && col < sheet.getMaxCols() && row > 0 && row <= sheet.getMaxRows()) {
                        activeCol = col;
                        activeRow = row - 1;
                    }
                }
                jumpMode = false;
                jumpBuffer.clear();
                continue;
            } else if (key == 27) { // ESC to cancel
                jumpMode = false;
                jumpBuffer.clear();
                continue;
            } else if (key == 127 || key == 8) { // Backspace
                if (!jumpBuffer.empty()) jumpBuffer.pop_back();
            } else if (isprint(key)) {
                jumpBuffer += key;
            }
            continue;
        }
        if (!editMode) {
            char key = getchar();
            if (key == '/') {
                commandMode = true;
                commandBuffer = "/";
                continue;
            }
            if (key == '>') {
                jumpMode = true;
                jumpBuffer.clear();
                continue;
            }
            if (key == '\033') { // Escape sequence
                char k1 = getchar();
                if (k1 == '[') {
                    char k2 = getchar();
                    switch (k2) {
                        case 'A': screen.moveActiveCell(activeRow, activeCol, sheet.getMaxRows(), sheet.getMaxCols(), 'w'); break;
                        case 'B': screen.moveActiveCell(activeRow, activeCol, sheet.getMaxRows(), sheet.getMaxCols(), 's'); break;
                        case 'C': screen.moveActiveCell(activeRow, activeCol, sheet.getMaxRows(), sheet.getMaxCols(), 'd'); break;
                        case 'D': screen.moveActiveCell(activeRow, activeCol, sheet.getMaxRows(), sheet.getMaxCols(), 'a'); break;
                        case 'O': {
                            char k3 = getchar();
                            if (k3 == 'P') { // F1
                                activeRow = 0;
                                activeCol = 0;
                            }
                        } break;
                    }
                }
            } else if (isprint(key)) {
                editMode = true;
                editBuffer = key;
                char ch = key;
                isLabel = !(isdigit(ch) || ch == '+' || ch == '-');
                continue;
            } else {
                screen.moveActiveCell(activeRow, activeCol, sheet.getMaxRows(), sheet.getMaxCols(), key);
            }
        } else {
            char key = getchar();
            if (key == '\n' || key == '\r') {
                // Save content
                sheet.setCellContent(activeRow, activeCol, editBuffer);
                editMode = false;
                continue;
            } else if (key == 27) { // ESC to cancel
                editMode = false;
                continue;
            } else if (key == 127 || key == 8) { // Backspace
                if (!editBuffer.empty()) editBuffer.pop_back();
            } else if (key == '\x1B') { // Escape sequence for arrows
                char k1 = getchar();
                if (k1 == '[') {
                    char k2 = getchar();
                    if (k2 == 'A' || k2 == 'B') {
                        // Ignore up/down arrows in edit mode
                        continue;
                    }
                }
            } else {
                editBuffer += key;
            }
            // Update isLabel: label unless starts with digit, +, -, (, ., #, @
            if (!editBuffer.empty()) {
                char ch = editBuffer[0];
                isLabel = !(isdigit(ch) || ch == '+' || ch == '-' || ch == '(' || ch == '.' || ch == '#' || ch == '@');
            } else {
                isLabel = false;
            }
        }
        if (askFileName) {
            clearScreen();
            int termRows, termCols;
            getTerminalSize(termRows, termCols);
            int minCellWidth = sheet.getMinCellWidth();
            int rowLabelWidth = 5;
            int availableWidth = termCols - rowLabelWidth;
            int visibleCols = availableWidth / minCellWidth;
            if (visibleCols > sheet.getMaxCols()) visibleCols = sheet.getMaxCols();
            if (visibleCols < 1) visibleCols = 1;
            int visibleRows = termRows - 5;
            if (visibleRows > sheet.getMaxRows()) visibleRows = sheet.getMaxRows();
            if (visibleRows < 1) visibleRows = 1;
            screen.drawSpreadsheet(sheet, activeRow, activeCol, rowOffset, colOffset, false, "", false, false, false, true, fileNameBuffer);
            char key = getchar();
            if (key == '\n' || key == '\r') {
                if (!fileNameBuffer.empty()) {
                    spreadsheetFileName = fileNameBuffer;
                    nlohmann::json j;
                    for (int r = 0; r < sheet.getMaxRows(); ++r) {
                        for (int c = 0; c < sheet.getMaxCols(); ++c) {
                            const Cell& cell = sheet.getCell(r, c);
                            if (cell.type != CellType::EMPTY) {
                                std::string addr = excelColName(c) + std::to_string(r + 1);
                                j[addr] = {
                                    {"type", cell.type == CellType::NUMBER ? "number" : cell.type == CellType::TEXT ? "text" : cell.type == CellType::FORMULA ? "formula" : "empty"},
                                    {"raw", cell.rawContent},
                                    {"display", cell.displayValue},
                                    {"value", cell.numericValue}
                                };
                            }
                        }
                    }
                    std::ofstream out(spreadsheetFileName);
                    out << j.dump(2);
                    out.close();
                }
                askFileName = false;
                fileNameBuffer.clear();
                continue;
            } else if (key == 27) { // ESC to cancel
                askFileName = false;
                fileNameBuffer.clear();
                continue;
            } else if (key == 127 || key == 8) { // Backspace
                if (!fileNameBuffer.empty()) fileNameBuffer.pop_back();
            } else if (isprint(key)) {
                fileNameBuffer += key;
            }
            continue;
        }
        if (askLoadFileName) {
            clearScreen();
            int termRows, termCols;
            getTerminalSize(termRows, termCols);
            int minCellWidth = sheet.getMinCellWidth();
            int rowLabelWidth = 5;
            int availableWidth = termCols - rowLabelWidth;
            int visibleCols = availableWidth / minCellWidth;
            if (visibleCols > sheet.getMaxCols()) visibleCols = sheet.getMaxCols();
            if (visibleCols < 1) visibleCols = 1;
            int visibleRows = termRows - 5;
            if (visibleRows > sheet.getMaxRows()) visibleRows = sheet.getMaxRows();
            if (visibleRows < 1) visibleRows = 1;
            screen.drawSpreadsheet(sheet, activeRow, activeCol, rowOffset, colOffset, false, "", false, false, false, true, loadFileNameBuffer);
            char key = getchar();
            if (key == '\n' || key == '\r') {
                if (!loadFileNameBuffer.empty()) {
                    std::ifstream in(loadFileNameBuffer);
                    if (in) {
                        nlohmann::json j;
                        in >> j;
                        in.close();
                        // Clear sheet first
                        for (int r = 0; r < sheet.getMaxRows(); ++r)
                            for (int c = 0; c < sheet.getMaxCols(); ++c)
                                sheet.clearCell(r, c);
                        // Load cells
                        for (auto& [addr, val] : j.items()) {
                            // Parse address
                            int col = 0, i = 0;
                            std::string s = addr;
                            while (i < (int)s.size() && isalpha(s[i])) {
                                col = col * 26 + (toupper(s[i]) - 'A' + 1);
                                ++i;
                            }
                            col--;
                            int row = 0;
                            while (i < (int)s.size() && isdigit(s[i])) {
                                row = row * 10 + (s[i] - '0');
                                ++i;
                            }
                            if (col >= 0 && col < sheet.getMaxCols() && row > 0 && row <= sheet.getMaxRows()) {
                                std::string raw = val.value("raw", "");
                                sheet.setCellContent(row - 1, col, raw);
                            }
                        }
                    }
                }
                askLoadFileName = false;
                loadFileNameBuffer.clear();
                continue;
            } else if (key == 27) { // ESC to cancel
                askLoadFileName = false;
                loadFileNameBuffer.clear();
                continue;
            } else if (key == 127 || key == 8) { // Backspace
                if (!loadFileNameBuffer.empty()) loadFileNameBuffer.pop_back();
            } else if (isprint(key)) {
                loadFileNameBuffer += key;
            }
            continue;
        }
    }
    setRawMode(false);
    return 0;
}
