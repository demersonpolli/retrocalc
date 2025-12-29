#pragma once

#include <string>

constexpr int ROW_LABEL_WIDTH = 3;
constexpr int DEFAULT_COL_WIDTH = 9;
constexpr int HEADER_ROWS = 4;

enum class EditMode {
    Normal,
    Command,
    Editing
};

enum class InputType {
    None,
    Value,
    Label,
    Repeating,
    Goto
};

struct SpreadsheetView {
    int cursorRow = 0;
    int cursorCol = 0;
    int scrollRow = 0;
    int scrollCol = 0;
    EditMode mode = EditMode::Normal;
    InputType inputType = InputType::None;
    std::string inputBuffer;
};

void getTerminalSize(int& rows, int& cols);
void clearScreen();
void moveCursor(int row, int col);
void setReverse(bool on);
void hideCursor();
void showCursor();
void drawSpreadsheetScreen(const SpreadsheetView& view, const class Matrix& matrix);
int visibleRows();
int visibleCols();
std::string columnLabel(int col);
bool parseAddress(const std::string& addr, int& row, int& col);
