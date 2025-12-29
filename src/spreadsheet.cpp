#include "spreadsheet.h"
#include <cctype>
#include <sstream>

Spreadsheet::Spreadsheet() {
    // Cells are initialized by default constructor
}

Cell& Spreadsheet::getCell(int row, int col) {
    if (row >= 0 && row < MAX_ROWS && col >= 0 && col < MAX_COLS) {
        return cells[row][col];
    }
    // Return first cell as fallback (should handle error better in production)
    return cells[0][0];
}

const Cell& Spreadsheet::getCell(int row, int col) const {
    if (row >= 0 && row < MAX_ROWS && col >= 0 && col < MAX_COLS) {
        return cells[row][col];
    }
    return cells[0][0];
}

void Spreadsheet::setCellContent(int row, int col, const std::string& content) {
    if (row < 0 || row >= MAX_ROWS || col < 0 || col >= MAX_COLS) {
        return;
    }
    
    Cell& cell = cells[row][col];
    cell.rawContent = content;
    
    if (content.empty()) {
        cell.type = CellType::EMPTY;
        cell.numericValue = 0.0;
        cell.displayValue = "";
        return;
    }
    // Check if it's a formula (starts with '=' or '+')
    if (content[0] == '=' || content[0] == '+') {
        cell.type = CellType::FORMULA;
        cell.displayValue = content;
        return;
    }
    // Try to parse as number
    std::istringstream iss(content);
    double value;
    if ((iss >> value && iss.eof()) || content[0] == '+' || content[0] == '-') {
        cell.type = CellType::NUMBER;
        cell.numericValue = value;
        cell.displayValue = content;
    } else {
        // It's text/label
        cell.type = CellType::TEXT;
        cell.numericValue = 0.0;
        if (!content.empty() && content[0] == '\'')
            cell.displayValue = content.substr(1);
        else
            cell.displayValue = content;
    }
}

void Spreadsheet::clearCell(int row, int col) {
    if (row >= 0 && row < MAX_ROWS && col >= 0 && col < MAX_COLS) {
        cells[row][col] = Cell();
    }
}

std::string excelColName(int col) {
    std::string name;
    while (col >= 0) {
        name = char('A' + (col % 26)) + name;
        col = col / 26 - 1;
    }
    return name;
}
