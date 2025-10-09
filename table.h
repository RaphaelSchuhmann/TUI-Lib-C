#include <windows.h>
#include <stdbool.h>
#include <inttypes.h>
#include "tcon.h"

#ifndef TABLE_H
#define TABLE_H

typedef struct TableCell
{
    int32_t size;
    Cell **conCells;
    ColorForeground fgColor;
    ColorBackground bgColor;
    char *content;
} TableCell;

typedef struct Table
{
    int32_t rows;
    int32_t cols;
    TableCell **cells;
} Table;

Table createTable(Console *con, int32_t rows, int32_t cols);
void setCellValue(Table *table, char *value, int32_t row, int32_t col, ColorForeground fgColor, ColorBackground bgColor);
void clearTable(Table *table, Console con, HANDLE hConsole, bool hlt);
void debugTableCellsByChar(Console *con, Table *table);
void contentCut(char *str, int32_t begin, int32_t len);
void reDrawTable(Table *table, Console *con, HANDLE hConsole, bool hlt);
void clearTableConsole(Table *table, Console *con, HANDLE hConsole, bool hlt);
void removeTable(Table *table);

#endif