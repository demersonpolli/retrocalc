#pragma once

#include <string>

constexpr int MAX_ROWS = 256;
constexpr int MAX_COLS = 64;

enum class CellType {
    Empty,
    Value,
    Label,
    Repeating
};

struct Cell {
    CellType type = CellType::Empty;
    std::string text;
    double numericValue = 0.0;
    std::string format = "   ";

    bool isEmpty() const { return type == CellType::Empty; }

    void clear() {
        type = CellType::Empty;
        text.clear();
        numericValue = 0.0;
    }

    void setValue(const std::string& expr, double val) {
        type = CellType::Value;
        text = expr;
        numericValue = val;
    }

    void setLabel(const std::string& label) {
        type = CellType::Label;
        text = label;
        numericValue = 0.0;
    }

    void setRepeating(char ch) {
        type = CellType::Repeating;
        text = std::string(1, ch);
        numericValue = 0.0;
    }

    double getValue() const {
        return numericValue;
    }

    const std::string& getText() const {
        return text;
    }

    char getRepeating() const {
        return text.empty() ? ' ' : text[0];
    }
};
