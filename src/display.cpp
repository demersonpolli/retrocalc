#include "display.h"
#include "matrix.h"
#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>

std::string columnLabel(int col) {
    std::string label;
    col++;
    while (col > 0) {
        col--;
        label = char('A' + col % 26) + label;
        col /= 26;
    }
    return label;
}

bool parseAddress(const std::string& addr, int& row, int& col) {
    if (addr.empty()) return false;

    size_t i = 0;
    col = 0;
    while (i < addr.length() && std::isalpha(addr[i])) {
        col = col * 26 + (std::toupper(addr[i]) - 'A' + 1);
        i++;
    }
    if (i == 0 || col == 0) return false;
    col--;

    if (i >= addr.length() || !std::isdigit(addr[i])) return false;
    row = 0;
    while (i < addr.length() && std::isdigit(addr[i])) {
        row = row * 10 + (addr[i] - '0');
        i++;
    }
    if (row == 0) return false;
    row--;

    if (row >= MAX_ROWS || col >= MAX_COLS) return false;
    return true;
}

#ifdef _WIN32
#include <windows.h>

void getTerminalSize(int& rows, int& cols) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
}

#else
#include <sys/ioctl.h>
#include <unistd.h>

void getTerminalSize(int& rows, int& cols) {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    rows = w.ws_row;
    cols = w.ws_col;
}

#endif

void clearScreen() {
    std::cout << "\033[2J\033[H" << std::flush;
}

void moveCursor(int row, int col) {
    std::cout << "\033[" << row << ";" << col << "H" << std::flush;
}

void setReverse(bool on) {
    if (on) {
        std::cout << "\033[7m";
    } else {
        std::cout << "\033[0m";
    }
}

void hideCursor() {
    std::cout << "\033[?25l" << std::flush;
}

void showCursor() {
    std::cout << "\033[?25h" << std::flush;
}

int visibleRows() {
    int termRows, termCols;
    getTerminalSize(termRows, termCols);
    return termRows - HEADER_ROWS;
}

int visibleCols() {
    int termRows, termCols;
    getTerminalSize(termRows, termCols);
    return (termCols - ROW_LABEL_WIDTH) / DEFAULT_COL_WIDTH;
}

void drawSpreadsheetScreen(const SpreadsheetView& view, const Matrix& matrix) {
    int termRows, termCols;
    getTerminalSize(termRows, termCols);

    clearScreen();

    for (int row = 1; row <= termRows; row++) {
        moveCursor(row, 1);

        if (row == 1) {
            setReverse(true);

            std::string coord = columnLabel(view.cursorCol) + std::to_string(view.cursorRow + 1);
            const Cell* cell = matrix.getCellPtr(view.cursorRow, view.cursorCol);

            std::string formatStr = "   ";
            char typeChar = 'V';
            std::string contentStr;

            if (cell && !cell->isEmpty()) {
                formatStr = cell->format;
                switch (cell->type) {
                    case CellType::Value:
                        typeChar = 'V';
                        contentStr = cell->getText();
                        break;
                    case CellType::Label:
                        typeChar = 'L';
                        contentStr = cell->getText();
                        break;
                    case CellType::Repeating:
                        typeChar = '/';
                        contentStr = std::string(1, cell->getRepeating());
                        break;
                    default:
                        break;
                }
            }

            char calcChar = 'C';
            switch (matrix.calcMode) {
                case CalcMode::Column: calcChar = 'C'; break;
                case CalcMode::Row: calcChar = 'R'; break;
                case CalcMode::Recalculating: calcChar = '!'; break;
            }

            std::string line = " " + coord + " " + formatStr + "   (" + typeChar + ")   " + contentStr;
            int endLen = 2;
            int maxContent = termCols - endLen - (int)line.length();
            if (maxContent < 0) {
                line = line.substr(0, termCols - endLen);
            } else {
                line += std::string(maxContent, ' ');
            }
            line += calcChar;
            line += ' ';

            std::cout << line;
        } else if (row == 2) {
            setReverse(true);
            std::string row2Content;
            if (view.mode == EditMode::Editing) {
                switch (view.inputType) {
                    case InputType::Value: row2Content = "Value"; break;
                    case InputType::Label: row2Content = "Label"; break;
                    case InputType::Repeating: row2Content = "Repeating"; break;
                    default: break;
                }
            }
            std::cout << row2Content;
            for (int col = row2Content.length() + 1; col <= termCols; col++) {
                std::cout << ' ';
            }
        } else if (row == 3) {
            setReverse(false);
            if (view.mode == EditMode::Editing || view.inputType == InputType::Goto) {
                std::cout << view.inputBuffer;
                for (size_t col = view.inputBuffer.length() + 1; col <= (size_t)termCols; col++) {
                    std::cout << ' ';
                }
            } else {
                for (int col = 1; col <= termCols; col++) {
                    std::cout << ' ';
                }
            }
        } else if (row == 4) {
            setReverse(true);
            std::cout << std::string(ROW_LABEL_WIDTH, ' ');

            int sheetCol = view.scrollCol;
            int screenCol = ROW_LABEL_WIDTH + 1;
            while (screenCol + DEFAULT_COL_WIDTH <= termCols + 1) {
                std::string lbl = columnLabel(sheetCol);
                int width = DEFAULT_COL_WIDTH;
                int padding = (width - lbl.length()) / 2;
                std::cout << std::string(padding, ' ') << lbl;
                std::cout << std::string(width - padding - lbl.length(), ' ');
                screenCol += width;
                sheetCol++;
            }
            for (; screenCol <= termCols; screenCol++) {
                std::cout << ' ';
            }
        } else {
            int sheetRow = view.scrollRow + (row - HEADER_ROWS - 1);
            setReverse(true);
            std::cout << std::setw(ROW_LABEL_WIDTH) << (sheetRow + 1);
            setReverse(false);

            int sheetCol = view.scrollCol;
            int screenCol = ROW_LABEL_WIDTH + 1;
            while (screenCol + DEFAULT_COL_WIDTH <= termCols + 1) {
                bool isActive = (sheetRow == view.cursorRow && sheetCol == view.cursorCol);
                setReverse(isActive);

                const Cell* cell = matrix.getCellPtr(sheetRow, sheetCol);
                std::string cellDisplay;
                if (cell && !cell->isEmpty()) {
                    if (cell->type == CellType::Value) {
                        std::ostringstream oss;
                        oss << cell->getValue();
                        cellDisplay = oss.str();
                    } else if (cell->type == CellType::Label) {
                        cellDisplay = cell->getText();
                    }
                }

                if (cellDisplay.length() > DEFAULT_COL_WIDTH) {
                    cellDisplay = cellDisplay.substr(0, DEFAULT_COL_WIDTH);
                }
                std::cout << std::setw(DEFAULT_COL_WIDTH) << cellDisplay;

                screenCol += DEFAULT_COL_WIDTH;
                sheetCol++;
            }
            setReverse(false);
            for (; screenCol <= termCols; screenCol++) {
                std::cout << ' ';
            }
        }
    }

    setReverse(false);
    moveCursor(1, 1);
    std::cout << std::flush;
}
