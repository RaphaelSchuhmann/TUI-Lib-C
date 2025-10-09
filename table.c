#include "tcon.h"
#include "table.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

Table createTable(Console *con, int32_t rows, int32_t cols)
{
    Table table;
    table.rows = rows;
    table.cols = cols;

    // Allocate cells array in table
    table.cells = malloc(rows * sizeof(TableCell *));
    for (int32_t r = 0; r < rows; r++)
        table.cells[r] = malloc(cols * sizeof(TableCell));

    // Prepare separator columns
    int32_t *separators = malloc(cols * sizeof(int));
    int32_t colWidth = con->cols / cols;
    for (int32_t j = 0; j < cols; j++)
        separators[j] = (j + 1) * colWidth;

    // Create column borders in framebuffer
    for (int32_t i = 0; i < rows; i++)
    {
        for (int32_t j = 0; j < cols; j++)
            setCellData(con, i, separators[j], FWHITE, BBLACK, '|');
    }

    // Link console cells to table cells
    for (int32_t r = 0; r < rows; r++)
    {
        for (int32_t c = 0; c < cols; c++)
        {
            TableCell *cell = &table.cells[r][c];

            int32_t startCol = (c == 0) ? 0 : separators[c - 1] + 1;
            int32_t endCol = separators[c] - 1;
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
            int32_t k = 0;

            // Map framebuffer cells to this table cell
            for (int32_t fc = startCol; fc <= endCol; fc++) // inclusive of endCol
            {
                cell->conCells[k++] = &con->framebuffer[r][fc];
            }
        }
    }

    free(separators);
    return table;
}

void setCellValue(Table *table, char *value, int32_t row, int32_t col, ColorForeground fgColor, ColorBackground bgColor)
{
    TableCell *cell = &table->cells[row][col];

    int32_t spaceCounter = 0;
    for (int32_t i = 0; i < strlen(value); i++)
    {
        if (isspace(value[i]))
            spaceCounter++;
    }

    if (spaceCounter == strlen(value) && cell->size != strlen(value))
        contentCut(value, cell->size - 1, strlen(value));

    int32_t len = strlen(value);
    bool overflow = len > table->cells[row][col].size ? true : false;

    cell->content = value;
    cell->fgColor = fgColor;
    cell->bgColor = bgColor;

    int32_t maxContent = overflow ? cell->size - 3 : cell->size;

    for (int32_t j = 0; j < maxContent; j++)
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

void contentCut(char *str, int32_t begin, int32_t len)
{
    int32_t l = strlen(str);

    if (len < 0)
        len = l - begin;
    if (begin + len > l)
        len = l - begin;
    memmove(str + begin, str + begin + len, l - len + 1);
}

void clearTable(Table *table, Console con, HANDLE hConsole, bool hlt)
{
    for (int32_t i = 0; i < table->rows; i++)
    {
        for (int32_t j = 0; j < table->cols; j++)
        {
            TableCell *cell = &table->cells[i][j];
            for (int32_t k = 0; k < cell->size; k++)
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
    for (int32_t r = 0; r < table->rows; r++)
    {
        for (int32_t c = 0; c < table->cols; c++)
        {
            TableCell *cell = &table->cells[r][c];
            printf("Cell [%d,%d], size=%d: ", r, c, cell->size);

            for (int32_t k = 0; k < cell->size; k++)
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

void reDrawTable(Table *table, Console *con, HANDLE hConsole, bool hlt)
{
    getWindowSize(con, hConsole);

    // Recalculate separators
    int32_t *separators = malloc(table->cols * sizeof(int));
    int32_t colWidth = con->cols / table->cols;
    for (int32_t j = 0; j < table->cols; j++)
        separators[j] = (j + 1) * colWidth;

    for (int32_t i = 0; i < table->rows; i++)
    {
        for (int32_t j = 0; j < table->cols; j++)
            setCellData(con, i, separators[j], FWHITE, BBLACK, '|');
    }

    // Update framebuffer cells for each table cell
    for (int32_t tr = 0; tr < table->rows; tr++)
    {
        for (int32_t tc = 0; tc < table->cols; tc++)
        {
            TableCell *cell = &table->cells[tr][tc];

            int32_t startCol = (tc == 0) ? 0 : separators[tc - 1] + 1;
            int32_t endCol = separators[tc] - 1;
            cell->size = endCol - startCol + 1;

            if (cell->size <= 0)
            {
                cell->size = 0;
                cell->conCells = NULL;
                continue; // skip this cell
            }

            int32_t k = 0;
            for (int32_t fc = startCol; fc <= endCol; fc++) // inclusive of endCol
            {
                cell->conCells[k++] = &con->framebuffer[tr][fc];
            }
        }
    }

    // Reflow cell content
    for (int32_t tr = 0; tr < table->rows; tr++)
    {
        for (int32_t tc = 0; tc < table->cols; tc++)
        {
            TableCell *cell = &table->cells[tr][tc];
            char *content = cell->content;
            setCellValue(table, content, tr, tc, cell->fgColor, cell->bgColor);
        }
    }

    // Rerender framebuffer
    renderConsole(*con, hConsole, hlt);
}

void removeTable(Table *table)
{
    for (int32_t r = 0; r < table->rows; r++)
    {
        for (int32_t c = 0; c < table->cols; c++)
        {
            TableCell *cell = &table->cells[r][c];

            if (cell->content)
            {
                cell->content = NULL;
                free(cell->content);
            }

            if (cell->conCells) 
            {
                cell->conCells = NULL;
                free(cell->conCells);
            }

            cell->size = 0;
            cell->fgColor = FWHITE;
            cell->bgColor = BBLACK;
        }
    }
}

void clearTableConsole(Table *table, Console *con, HANDLE hConsole, bool hlt)
{
    removeTable(table);
    clearScreen(hConsole, con, hlt);
}

void resetTableConsole(Table *table, Console *con, HANDLE hConsole)
{
    removeTable(table);
    resetConsole(con, hConsole);
}