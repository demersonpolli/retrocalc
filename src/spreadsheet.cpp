#include "spreadsheet.h"
#include "terminal.h"
#include "display.h"
#include "matrix.h"
#include <iostream>
#include <cctype>

static bool isValueTrigger(int ch) {
    return std::isdigit(ch) || ch == '+' || ch == '-' || ch == '(' || ch == '.' || ch == '#' || ch == '@';
}

static bool isLabelTrigger(int ch) {
    return std::isalpha(ch) || ch == '\'';
}

void runSpreadsheet() {
    initTerminal();
    hideCursor();

    SpreadsheetView view;
    Matrix matrix;

    drawSpreadsheetScreen(view, matrix);

    bool running = true;
    while (running) {
        int key = getKey();

        if (view.inputType == InputType::Goto) {
            if (key == KEY_ESC) {
                view.inputType = InputType::None;
                view.inputBuffer.clear();
                drawSpreadsheetScreen(view, matrix);
            } else if (key == '\r' || key == '\n') {
                int newRow, newCol;
                if (parseAddress(view.inputBuffer, newRow, newCol)) {
                    view.cursorRow = newRow;
                    view.cursorCol = newCol;
                    if (view.cursorRow < view.scrollRow) {
                        view.scrollRow = view.cursorRow;
                    } else if (view.cursorRow >= view.scrollRow + visibleRows()) {
                        view.scrollRow = view.cursorRow - visibleRows() + 1;
                    }
                    if (view.cursorCol < view.scrollCol) {
                        view.scrollCol = view.cursorCol;
                    } else if (view.cursorCol >= view.scrollCol + visibleCols()) {
                        view.scrollCol = view.cursorCol - visibleCols() + 1;
                    }
                }
                view.inputType = InputType::None;
                view.inputBuffer.clear();
                drawSpreadsheetScreen(view, matrix);
            } else if (key == 127 || key == 8) {
                if (!view.inputBuffer.empty()) {
                    view.inputBuffer.pop_back();
                    drawSpreadsheetScreen(view, matrix);
                }
            } else if (key >= 32 && key < 127) {
                view.inputBuffer += static_cast<char>(key);
                drawSpreadsheetScreen(view, matrix);
            }
        } else if (view.mode == EditMode::Editing) {
            if (key == KEY_ESC) {
                view.mode = EditMode::Normal;
                view.inputType = InputType::None;
                view.inputBuffer.clear();
                drawSpreadsheetScreen(view, matrix);
            } else if (key == '\r' || key == '\n') {
                if (!view.inputBuffer.empty()) {
                    Cell cell;
                    if (view.inputType == InputType::Value) {
                        cell.setValue(view.inputBuffer, 0.0);
                    } else if (view.inputType == InputType::Label) {
                        cell.setLabel(view.inputBuffer);
                    }
                    matrix.setCell(view.cursorRow, view.cursorCol, cell);
                }
                view.mode = EditMode::Normal;
                view.inputType = InputType::None;
                view.inputBuffer.clear();
                drawSpreadsheetScreen(view, matrix);
            } else if (key == 127 || key == 8) {
                if (!view.inputBuffer.empty()) {
                    view.inputBuffer.pop_back();
                    drawSpreadsheetScreen(view, matrix);
                }
            } else if (key >= 32 && key < 127) {
                view.inputBuffer += static_cast<char>(key);
                drawSpreadsheetScreen(view, matrix);
            }
        } else {
            switch (key) {
                case KEY_ESC:
                    running = false;
                    break;

                case KEY_ARROW_UP:
                    if (view.cursorRow > 0) {
                        view.cursorRow--;
                        if (view.cursorRow < view.scrollRow) {
                            view.scrollRow = view.cursorRow;
                        }
                        drawSpreadsheetScreen(view, matrix);
                    }
                    break;

                case KEY_ARROW_DOWN:
                    if (view.cursorRow < MAX_ROWS - 1) {
                        view.cursorRow++;
                        if (view.cursorRow >= view.scrollRow + visibleRows()) {
                            view.scrollRow = view.cursorRow - visibleRows() + 1;
                        }
                        drawSpreadsheetScreen(view, matrix);
                    }
                    break;

                case KEY_ARROW_LEFT:
                    if (view.cursorCol > 0) {
                        view.cursorCol--;
                        if (view.cursorCol < view.scrollCol) {
                            view.scrollCol = view.cursorCol;
                        }
                        drawSpreadsheetScreen(view, matrix);
                    }
                    break;

                case KEY_ARROW_RIGHT:
                    if (view.cursorCol < MAX_COLS - 1) {
                        view.cursorCol++;
                        if (view.cursorCol >= view.scrollCol + visibleCols()) {
                            view.scrollCol = view.cursorCol - visibleCols() + 1;
                        }
                        drawSpreadsheetScreen(view, matrix);
                    }
                    break;

                case '>':
                    view.inputType = InputType::Goto;
                    view.inputBuffer.clear();
                    drawSpreadsheetScreen(view, matrix);
                    break;

                case KEY_F1:
                    view.cursorRow = 0;
                    view.cursorCol = 0;
                    view.scrollRow = 0;
                    view.scrollCol = 0;
                    drawSpreadsheetScreen(view, matrix);
                    break;

                case KEY_F2:
                    {
                        const Cell* cell = matrix.getCellPtr(view.cursorRow, view.cursorCol);
                        if (cell && !cell->isEmpty()) {
                            view.mode = EditMode::Editing;
                            if (cell->type == CellType::Value) {
                                view.inputType = InputType::Value;
                            } else if (cell->type == CellType::Label) {
                                view.inputType = InputType::Label;
                            }
                            view.inputBuffer = cell->getText();
                            if (!view.inputBuffer.empty()) {
                                view.inputBuffer.pop_back();
                            }
                            drawSpreadsheetScreen(view, matrix);
                        }
                    }
                    break;

                default:
                    if (isValueTrigger(key)) {
                        view.mode = EditMode::Editing;
                        view.inputType = InputType::Value;
                        view.inputBuffer = std::string(1, static_cast<char>(key));
                        drawSpreadsheetScreen(view, matrix);
                    } else if (isLabelTrigger(key)) {
                        view.mode = EditMode::Editing;
                        view.inputType = InputType::Label;
                        view.inputBuffer = std::string(1, static_cast<char>(key));
                        drawSpreadsheetScreen(view, matrix);
                    }
                    break;
            }
        }
    }

    showCursor();
    restoreTerminal();
    clearScreen();
    std::cout << "Goodbye!" << std::endl;
}
