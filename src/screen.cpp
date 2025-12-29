#include "screen.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include "spreadsheet.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif
// ANSI escape codes for colors
#define ANSI_RESET "\033[0m"
#define ANSI_INVERT "\033[7m"
#define ANSI_WHITE_BG "\033[47m"
#define ANSI_BLACK_FG "\033[30m"

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

Screen::Screen() {}

void Screen::drawSpreadsheet(const Spreadsheet& sheet, int activeRow, int activeCol, int rowOffset, int colOffset, bool editMode, const std::string& editBuffer, bool isLabel, bool commandMode, bool storageMode, bool askFileName, const std::string& fileNameBuffer) {
    int termRows, termCols;
    getTerminalSize(termRows, termCols);
    int rowLabelWidth = 5;
    int minCellWidth = sheet.getMinCellWidth();
    int maxCols = sheet.getMaxCols();
    int availableWidth = termCols - rowLabelWidth;
    int visibleCols = availableWidth / minCellWidth;
    if (visibleCols > maxCols) visibleCols = maxCols;
    if (visibleCols < 1) visibleCols = 1;
    int baseColWidth = availableWidth / visibleCols;
    if (baseColWidth < minCellWidth) baseColWidth = minCellWidth;
    int extraSpace = availableWidth - (baseColWidth * visibleCols);
    int maxRows = sheet.getMaxRows();
    int visibleRows = termRows - 5;
    if (visibleRows > maxRows) visibleRows = maxRows;
    if (visibleRows < 1) visibleRows = 1;

    // First row: cell info
    std::string cellAddr = excelColName(activeCol) + std::to_string(activeRow + 1);
    std::string formatInd = "\\XX";
    const Cell& cell = sheet.getCell(activeRow, activeCol);
    char entryType = ' ';
    switch (cell.type) {
        case CellType::NUMBER: entryType = 'V'; break;
        case CellType::TEXT: entryType = 'L'; break;
        case CellType::FORMULA: entryType = 'V'; break;
        case CellType::EMPTY: entryType = 'V'; break;
    }
    std::string entryTypeStr = "(";
    entryTypeStr += entryType;
    entryTypeStr += ")";
    std::string cellContent = cell.rawContent;
    char calcMode = 'C';
    std::string info = cellAddr + " " + formatInd + "  " + entryTypeStr + "  " + cellContent;
    int infoLen = (int)info.size();
    int termColsForInfo = termCols - 2;
    std::cout << ANSI_INVERT << info;
    if (infoLen < termColsForInfo) {
        std::cout << std::setw(termColsForInfo - infoLen) << " ";
    }
    std::cout << calcMode << " " << ANSI_RESET << "\n";

    // Second row: edit mode indicator, command mode, storage mode, file name prompt, or blank
    if (askFileName) {
        std::string prompt = "Enter file name";
        std::cout << ANSI_INVERT << prompt << std::setw(termCols - (int)prompt.size()) << " " << ANSI_RESET << "\n";
    } else if (storageMode) {
        std::string storageStr = "STORAGE: L S D I Q #";
        std::cout << ANSI_INVERT << storageStr << std::setw(termCols - (int)storageStr.size()) << " " << ANSI_RESET << "\n";
    } else if (commandMode) {
        std::string cmdStr = "COMMAND:  BCDEFGIMPRSTVW-";
        std::cout << ANSI_INVERT << cmdStr << std::setw(termCols - (int)cmdStr.size()) << " " << ANSI_RESET << "\n";
    } else if (editMode) {
        std::string modeStr = isLabel ? "Label" : "Value";
        std::cout << ANSI_INVERT << modeStr << std::setw(termCols - (int)modeStr.size()) << " " << ANSI_RESET << "\n";
    } else {
        std::cout << ANSI_INVERT << std::setw(termCols) << " " << ANSI_RESET << "\n";
    }

    // Third row: edit buffer, file name input, or blank
    if (askFileName) {
        std::cout << fileNameBuffer << std::setw(termCols - (int)fileNameBuffer.size()) << " " << "\n";
    } else if (editMode) {
        std::cout << editBuffer << std::setw(termCols - (int)editBuffer.size()) << " " << "\n";
    } else {
        std::cout << std::setw(termCols) << " " << "\n";
    }

    // Fourth row: inverted, show column names
    std::cout << ANSI_INVERT;
    std::cout << std::setw(rowLabelWidth) << " ";
    for (int c = 0; c < visibleCols; ++c) {
        int thisColWidth = baseColWidth + (c < extraSpace ? 1 : 0);
        std::string colName = excelColName(c + colOffset);
        std::cout << std::setw(thisColWidth) << colName;
    }
    std::cout << ANSI_RESET << "\n";
    // Rows 5 and onward
    for (int r = 0; r < visibleRows; ++r) {
        std::cout << ANSI_INVERT << std::setw(rowLabelWidth - 1) << (r + 1 + rowOffset) << " " << ANSI_RESET;
        for (int c = 0; c < visibleCols; ++c) {
            int thisColWidth = baseColWidth + (c < extraSpace ? 1 : 0);
            const Cell& cell = sheet.getCell(r + rowOffset, c + colOffset);
            std::string display = cell.displayValue;
            if ((int)display.length() > thisColWidth - 1) display = display.substr(0, thisColWidth - 1);
            if ((r + rowOffset) == activeRow && (c + colOffset) == activeCol) {
                std::cout << ANSI_INVERT << std::setw(thisColWidth) << display << ANSI_RESET;
            } else {
                std::cout << std::setw(thisColWidth) << display;
            }
        }
        std::cout << "\n";
    }
}

void Screen::moveActiveCell(int& activeRow, int& activeCol, int maxRows, int maxCols, char key) {
    // Arrow keys: left=\x1B[D, right=\x1B[C, up=\x1B[A, down=\x1B[B
    // We'll use WASD for demo, but you can adapt for arrow keys
    switch (key) {
        case 'w': // up
            if (activeRow > 0) --activeRow;
            break;
        case 's': // down
            if (activeRow < maxRows - 1) ++activeRow;
            break;
        case 'a': // left
            if (activeCol > 0) --activeCol;
            break;
        case 'd': // right
            if (activeCol < maxCols - 1) ++activeCol;
            break;
        default:
            break;
    }
}
