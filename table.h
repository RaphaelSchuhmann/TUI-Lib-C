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

/*
Generates a new, empty Table.

Arguments:
   console - the current instance of the console
   rows - the amount of rows the table should have
   cols - the amount of columns the table should have

Returns:
   Table
*/
Table createTable(Console *con, int32_t rows, int32_t cols);

/*
Sets the content, foreground color, and background color for any given cell
in a Table.

Arguments:
   table - the table containing the cell
   value - the content to set in form of a string
   row - the row of the given cell in the table
   col - the column of the given cell in the table

Returns:
   Void
*/
void setCellValue(Table *table, char *value, int32_t row, int32_t col, ColorForeground fgColor, ColorBackground bgColor);

/*
Clears all values and colors in a table and resets them to their default

Arguments:
   table - the table to clear
   con - your console object to reset the framebuffer
   hConsole - the windows stdout handle
   hlt - a flag to wait for user input before continuing

Returns:
   Void
*/
void clearTable(Table *table, Console con, HANDLE hConsole, bool hlt);

/*
Prints out debug information for a given table.

Arguments:
   con - your console object
   table - the table to clear

Returns:
   Void
*/
void debugTableCellsByChar(Console *con, Table *table);

/*
Cuts a string to a given length.

Arguments:
   str - the string to modify
   begin - the start of the output string
   len - the new length of the string

Returns:
   Void
*/
void contentCut(char *str, int32_t begin, int32_t len);

/*
Re calculates the table and reflows the cell contents.

Arguments:
   table - the table to update
   con - your console object
   hConsole - a windows stdout handle
   hlt - a flag to halt execution and wait for user input

Returns:
   Void
*/
void reDrawTable(Table *table, Console *con, HANDLE hConsole, bool hlt);

/*
Removes a given table from the console and clears the console.

Arguments:
   table - the table to clear
   con - your console object
   hConsole - a windows stdout handle
   hlt - a flag to halt execution and wait for user input

Returns:
   Void
*/
void clearTableConsole(Table *table, Console *con, HANDLE hConsole, bool hlt);

/*
Removes a given table.

Arguments:
   table - the table to remove

Returns:
   Void
*/
void removeTable(Table *table);

/*
Adds a new row to the end of a given table.

Arguments:
   table - the table to add a row
   con - the current Console object
   hConsole - a windows stdout handle
   hlt - a flag to halt execution and wait for user input

Returns:
   Void
*/
void addTableRow(Table *table, Console *con, HANDLE hConsole, bool hlt);

/*
Removes a given row from a table.

Arguments:
   table - the table to remove the row from
   con - the current Console object
   row - the row to remove
   hConsole - a windows stdout handle
   hlt - a flag to halt execution and wait for user input

Returns:
   Void
*/
void removeTableRow(Table *table, Console *con, int32_t row, HANDLE hConsole, bool hlt);

/*
Adds a new column to the end of a given table.

Arguments:
   table - the table to add the column
   con - the current Console object
   hConsole - a windows stdout handle
   hlt - a flag to halt execution and wait for user input

Returns:
   Void
*/
void addTableCol(Table *table, Console *con, HANDLE hConsole, bool hlt);

/*
Removes a given column from a table.

Arguments:
   table - the table to remove the column from
   con - the current Console object
   col - the column to remove
   hConsole - a windows stdout handle
   hlt - a flag to halt execution and wait for user input

Returns:
   Void
*/
void removeTableCol(Table *table, Console *con, int32_t col, HANDLE hConsole, bool hlt);

#endif