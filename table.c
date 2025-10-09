#include "tcon.h"
#include "table.h"
#include <stdio.h>
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

            // Initialize content field
            cell->content = malloc((cell->size + 1) * sizeof(char));
            if (!cell->content)
            {
                perror("malloc");
                exit(1);
            }
            memset(cell->content, ' ', cell->size);
            cell->content[cell->size] = '\0';

            // Set default colors
            cell->bgColor = BBLACK;
            cell->fgColor = FWHITE;

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
    TableCell *cell = &table->cells[row][col];

    int spaceCounter = 0;
    for (int i = 0; i < strlen(value); i++)
    {
        if (isspace(value[i]))
            spaceCounter++;
    }

    if (spaceCounter == strlen(value) && cell->size != strlen(value))
        contentCut(value, cell->size - 1, strlen(value));

    int len = strlen(value);
    bool overflow = len > table->cells[row][col].size ? true : false;

    cell->content = value;
    cell->fgColor = fgColor;
    cell->bgColor = bgColor;

    int maxContent = overflow ? cell->size - 3 : cell->size;

    for (int j = 0; j < maxContent; j++)
    {
        cell->conCells[j]->Foreground = fgColor;
        cell->conCells[j]->Background = bgColor;
        cell->conCells[j]->Char = (j < len) ? value[j] : L' ';
    }

    if (overflow)
    {
        cell->conCells[cell->size - 3]->Char = L'.';
        cell->conCells[cell->size - 2]->Char = L'.';
        cell->conCells[cell->size - 1]->Char = L'.';
    }
}

void contentCut(char *str, int begin, int len)
{
    int l = strlen(str);

    if (len < 0)
        len = l - begin;
    if (begin + len > l)
        len = l - begin;
    memmove(str + begin, str + begin + len, l - len + 1);
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