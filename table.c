#include "tcon.h"
#include "table.h"
#include <string.h>
#include <stdlib.h>

// TODO: Re Draw

Table createTable(Console *con, int rows, int cols)
{
    Table table;
    table.rows = rows;
    table.cols = cols;

    // Allocate cells array in table
    table.cells = malloc(rows * sizeof(TableCell *));
    for (int r = 0; r < rows; r++)
        table.cells[r] = malloc(cols * sizeof(TableCell));

    // Prepare separator columns
    int *separators = malloc(cols * sizeof(int));
    int colWidth = con->cols / cols;
    for (int j = 0; j < cols; j++)
        separators[j] = (j + 1) * colWidth;

    // Create column borders in framebuffer
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
            setCellData(con, i, separators[j], FWHITE, BBLACK, '|');
    }

    // Link console cells to table cells
    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            TableCell *cell = &table.cells[r][c];

            int startCol = (c == 0) ? 0 : separators[c - 1] + 1;
            int endCol = separators[c] - 1;
            cell->size = endCol - startCol + 1;

            if (cell->size <= 0)
            {
                cell->size = 0;
                cell->conCells = NULL;
                continue; // skip this cell
            }

            // Allocate array of pointers to console cells
            cell->conCells = malloc(cell->size * sizeof(Cell *));
            int k = 0;

            // Map framebuffer cells to this table cell
            for (int fc = startCol; fc <= endCol; fc++) // inclusive of endCol
            {
                cell->conCells[k++] = &con->framebuffer[r][fc];
            }
        }
    }

    free(separators);
    return table;
}

void setCellValue(Table *table, char *value, int row, int col, ColorForeground fgColor, ColorBackground bgColor)
{
    int len = strlen(value);
    TableCell *cell = &table->cells[row][col];

    bool overflow = len > table->cells[row][col].size ? true : false;

    for (int j = 0; j < cell->size; j++)
    {
        cell->conCells[j]->Foreground = fgColor;
        cell->conCells[j]->Background = fgColor;
        if (j == cell->size - 1 && overflow)
        {
            j = j - 3;
            for (int i = 0; i < 3; i++)
            {
                cell->conCells[j + i]->Char = L'.';
            }
            break;
        }

        if (j < len)
            cell->conCells[j]->Char = value[j];
        else
            cell->conCells[j]->Char = L' ';
    }
}

void clearTable(Table *table, Console con, HANDLE hConsole, bool hlt)
{
    for (int i = 0; i < table->rows; i++)
    {
        for (int j = 0; j < table->cols; j++)
        {
            TableCell *cell = &table->cells[i][j];
            for (int k = 0; k < cell->size; k++)
            {
                cell->conCells[k]->Char = L' ';
                cell->conCells[k]->Foreground = FWHITE;
                cell->conCells[k]->Background = BBLACK;
            }
        }
    }

    renderConsole(con, hConsole, hlt);
}

void debugTableCellsByChar(Console *con, Table *table)
{
    printf("=== Table Debug Start ===\n");
    for (int r = 0; r < table->rows; r++)
    {
        for (int c = 0; c < table->cols; c++)
        {
            TableCell *cell = &table->cells[r][c];
            printf("Cell [%d,%d], size=%d: ", r, c, cell->size);

            for (int k = 0; k < cell->size; k++)
            {
                wchar_t ch = cell->conCells[k]->Char;

                if (ch == L'|' || ch == L'┃')
                    printf("[%lc|SEP] ", ch);
                else
                    printf("%lc ", ch);
            }
            printf("\n");
        }
    }
    printf("=== Table Debug End ===\n");
}