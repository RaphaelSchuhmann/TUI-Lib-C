#include <windows.h>
#include <stdbool.h>
#include "tcon.h"

#ifndef TABLE_H
#define TABLE_H

typedef struct TableCell
{
    int size;
    Cell **conCells;
    ColorForeground fgColor;
    ColorBackground bgColor;
    char *content;
} TableCell;

typedef struct Table
{
    int rows;
    int cols;
    TableCell **cells;
} Table;

Table createTable(Console *con, int rows, int cols);
void setCellValue(Table *table, char *value, int row, int col, ColorForeground fgColor, ColorBackground bgColor);
void clearTable(Table *table, Console con, HANDLE hConsole, bool hlt);
void debugTableCellsByChar(Console *con, Table *table);
void contentCut(char *str, int begin, int len);
void reDrawTable(Table *table, Console *con, HANDLE hConsole, bool hlt);
void clearTableConsole(Table *table, Console *con, HANDLE hConsole, bool hlt);
void removeTable(Table *table);

#endif