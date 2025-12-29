#include "parser.h"
#include "matrix.h"
#include <cctype>
#include <cstdlib>

double parseValue(const std::string& text, const Matrix& matrix) {
    (void)matrix;

    if (text.empty()) return 0.0;

    size_t pos = 0;
    bool hasDigit = false;
    bool hasDot = false;

    if (text[pos] == '+' || text[pos] == '-') {
        pos++;
    }

    while (pos < text.length()) {
        if (std::isdigit(text[pos])) {
            hasDigit = true;
            pos++;
        } else if (text[pos] == '.' && !hasDot) {
            hasDot = true;
            pos++;
        } else {
            break;
        }
    }

    if (hasDigit && pos == text.length()) {
        return std::stod(text);
    }

    return 0.0;
}
