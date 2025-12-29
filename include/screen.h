#ifndef SCREEN_H
#define SCREEN_H

#include <string>
#include "spreadsheet.h"

class Screen {
public:
    Screen();
    void drawSpreadsheet(const Spreadsheet& sheet, int activeRow, int activeCol, int rowOffset, int colOffset, bool editMode = false, const std::string& editBuffer = "", bool isLabel = false, bool commandMode = false, bool storageMode = false, bool askFileName = false, const std::string& fileNameBuffer = "");
    void moveActiveCell(int& activeRow, int& activeCol, int maxRows, int maxCols, char key);
};

#endif // SCREEN_H
