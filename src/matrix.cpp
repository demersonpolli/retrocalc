#include "matrix.h"
#include <fstream>

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

void Matrix::clearAll() {
    cells.clear();
}

bool Matrix::saveToFile(const std::string& fname) {
    if (fname.empty()) return false;

    std::ofstream file(fname);
    if (!file.is_open()) return false;

    for (const auto& pair : cells) {
        int key = pair.first;
        const Cell& cell = pair.second;
        if (cell.isEmpty()) continue;

        int row = key / MAX_COLS;
        int col = key % MAX_COLS;

        char typeChar = 'E';
        switch (cell.type) {
            case CellType::Value: typeChar = 'V'; break;
            case CellType::Label: typeChar = 'L'; break;
            case CellType::Repeating: typeChar = 'R'; break;
            default: continue;
        }

        file << row << "," << col << "," << typeChar << "," << cell.text << "\n";
    }

    filename = fname;
    return true;
}

bool Matrix::saveToFile() {
    return saveToFile(filename);
}

bool Matrix::loadFromFile(const std::string& fname) {
    if (fname.empty()) return false;

    clearAll();

    std::ifstream file(fname);
    if (!file.is_open()) {
        filename = fname;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        size_t pos1 = line.find(',');
        if (pos1 == std::string::npos) continue;
        size_t pos2 = line.find(',', pos1 + 1);
        if (pos2 == std::string::npos) continue;
        size_t pos3 = line.find(',', pos2 + 1);
        if (pos3 == std::string::npos) continue;

        int row = std::stoi(line.substr(0, pos1));
        int col = std::stoi(line.substr(pos1 + 1, pos2 - pos1 - 1));
        char typeChar = line[pos2 + 1];
        std::string text = line.substr(pos3 + 1);

        Cell cell;
        switch (typeChar) {
            case 'V':
                cell.setValue(text, 0.0);
                break;
            case 'L':
                cell.setLabel(text);
                break;
            case 'R':
                if (!text.empty()) cell.setRepeating(text[0]);
                break;
            default:
                continue;
        }
        setCell(row, col, cell);
    }

    filename = fname;
    return true;
}
