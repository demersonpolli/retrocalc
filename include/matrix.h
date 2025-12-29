#pragma once

#include "cell.h"
#include <unordered_map>

enum class CalcMode {
    Column,
    Row,
    Recalculating
};

class Matrix {
public:
    CalcMode calcMode = CalcMode::Column;
    Cell* getCellPtr(int row, int col);
    const Cell* getCellPtr(int row, int col) const;

    void setCell(int row, int col, const Cell& cell);
    bool hasCell(int row, int col) const;
    void clearCell(int row, int col);

    int getRowCount() const { return MAX_ROWS; }
    int getColCount() const { return MAX_COLS; }
    size_t usedCellCount() const { return cells.size(); }

private:
    static constexpr int cellKey(int row, int col) {
        return row * MAX_COLS + col;
    }

    std::unordered_map<int, Cell> cells;
    static Cell emptyCell;
};
