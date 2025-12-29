#ifndef SPREADSHEET_H
#define SPREADSHEET_H

#include <string>
#include <variant>

constexpr int MAX_ROWS = 256;
constexpr int MAX_COLS = 64;

enum class CellType {
    EMPTY,
    TEXT,
    NUMBER,
    FORMULA
};

struct Cell {
    CellType type;
    std::string rawContent;      // The original input (text, number as string, or formula)
    double numericValue;          // Computed value for numbers and formulas
    std::string displayValue;     // What gets displayed
    
    Cell() : type(CellType::EMPTY), rawContent(""), numericValue(0.0), displayValue("") {}
};

class Spreadsheet {
private:
    Cell cells[MAX_ROWS][MAX_COLS];
    static constexpr int MIN_CELL_WIDTH = 6; // Minimum cell width for display
    
public:
    Spreadsheet();
    
    // Get cell at position
    Cell& getCell(int row, int col);
    const Cell& getCell(int row, int col) const;
    
    // Set cell content
    void setCellContent(int row, int col, const std::string& content);
    
    // Clear a cell
    void clearCell(int row, int col);
    
    // Get dimensions
    int getMaxRows() const { return MAX_ROWS; }
    int getMaxCols() const { return MAX_COLS; }
    int getMinCellWidth() const { return MIN_CELL_WIDTH; }
};

// Utility function to convert column number to Excel-style column name (e.g., 0 -> "A", 1 -> "B", ..., 25 -> "Z", 26 -> "AA", ...)
std::string excelColName(int col);

#endif // SPREADSHEET_H
