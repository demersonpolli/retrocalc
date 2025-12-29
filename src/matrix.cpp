#include "matrix.h"

Cell* Matrix::getCellPtr(int row, int col) {
    if (row < 0 || row >= MAX_ROWS || col < 0 || col >= MAX_COLS) {
        return nullptr;
    }
    auto it = cells.find(cellKey(row, col));
    if (it == cells.end()) {
        return nullptr;
    }
    return &it->second;
}

const Cell* Matrix::getCellPtr(int row, int col) const {
    if (row < 0 || row >= MAX_ROWS || col < 0 || col >= MAX_COLS) {
        return nullptr;
    }
    auto it = cells.find(cellKey(row, col));
    if (it == cells.end()) {
        return nullptr;
    }
    return &it->second;
}

void Matrix::setCell(int row, int col, const Cell& cell) {
    if (row < 0 || row >= MAX_ROWS || col < 0 || col >= MAX_COLS) {
        return;
    }
    if (cell.isEmpty()) {
        cells.erase(cellKey(row, col));
    } else {
        cells[cellKey(row, col)] = cell;
    }
}

bool Matrix::hasCell(int row, int col) const {
    if (row < 0 || row >= MAX_ROWS || col < 0 || col >= MAX_COLS) {
        return false;
    }
    auto it = cells.find(cellKey(row, col));
    return it != cells.end() && !it->second.isEmpty();
}

void Matrix::clearCell(int row, int col) {
    if (row < 0 || row >= MAX_ROWS || col < 0 || col >= MAX_COLS) {
        return;
    }
    cells.erase(cellKey(row, col));
}
