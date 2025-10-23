#include "tcon.h"
#include "table.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

// TODO: Consider auto calling redraw after adding / removing a row or column
// TODO: Implement proper error handling (Consider a custom error handler)

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
    int32_t usableCols = con->cols;
    if (usableCols % 2 != 0)
        usableCols--;

    int32_t colWidth = usableCols / cols;

    int32_t *separators = malloc(cols * sizeof(int32_t));
    if (!separators)
        return table;

    for (int32_t j = 0; j < cols; j++)
        separators[j] = (j + 1) * colWidth;

    // Create column borders in framebuffer
    for (int32_t i = 0; i < rows; i++)
    {
        for (int32_t j = 0; j < cols - 1; j++)
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
    if (row >= table->rows || col >= table->cols)
        return;

    TableCell *cell = &table->cells[row][col];

    int32_t spaceCounter = 0;
    for (int32_t i = 0; i < strlen(value); i++)
    {
        if (isspace(value[i]))
            spaceCounter++;
    }

    if (spaceCounter == strlen(value) && cell->size < strlen(value))
        contentCut(value, cell->size - 1, strlen(value));

    int32_t len = strlen(value);
    bool overflow = len > table->cells[row][col].size ? true : false;

    cell->content = value;
    cell->fgColor = fgColor;
    cell->bgColor = bgColor;

    int32_t maxContent = overflow ? cell->size - 3 : cell->size;

    for (int32_t j = 0; j < maxContent; j++)
    {
        if (!cell->conCells[j])
            continue;
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
    int32_t usableCols = con->cols;
    if (usableCols % 2 != 0)
        usableCols--;

    int32_t colWidth = usableCols / table->cols;

    int32_t *separators = malloc(table->cols * sizeof(int32_t));
    if (!separators)
        return;

    for (int32_t j = 0; j < table->cols; j++)
        separators[j] = (j + 1) * colWidth;

    for (int32_t i = 0; i < table->rows; i++)
    {
        for (int32_t j = 0; j < table->cols - 1; j++)
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

            cell->conCells = malloc(cell->size * sizeof(Cell *));
            int32_t k = 0;

            // Map framebuffer cells to this table cell
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

    free(separators);

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

void addTableRow(Table *table, Console *con)
{
    int32_t oldRows = table->rows;
    int32_t newRows = table->rows + 1;

    table->cells = realloc(table->cells, newRows * sizeof(TableCell *));
    if (!table->cells)
        return;

    for (int32_t r = oldRows; r < newRows; r++)
    {
        table->cells[r] = malloc(table->cols * sizeof(TableCell));
        if (!table->cells[r])
            return;

        for (int32_t c = 0; c < table->cols; c++)
        {
            TableCell *cell = &table->cells[r][c];

            cell->size = table->cells[0][0].size;
            cell->fgColor = FWHITE;
            cell->bgColor = BBLACK;

            cell->content = malloc((cell->size + 1) * sizeof(char));
            if (!cell->content)
            {
                perror("malloc");
                exit(1);
            }
            memset(cell->content, ' ', cell->size);
            cell->content[cell->size] = '\0';

            cell->conCells = malloc(cell->size * sizeof(Cell *));
            if (!cell->conCells)
                return;

            for (int i = 0; i < cell->size; i++)
            {
                int fbRow = r;
                int fbCol = c * cell->size + i;

                if (fbCol >= con->cols)
                {
                    cell->conCells[i] = NULL;
                    continue;
                }

                cell->conCells[i] = &con->framebuffer[fbRow][fbCol];
            }
        }
    }

    table->rows = newRows;
}

void removeTableRow(Table *table, Console *con, int32_t row)
{
    if (row < 0 || row + 1 > table->rows)
        return;

    // If Last row set fb row to empty
    if (row + 1 == table->rows)
    {
        // Free cell memory and reset their values
        for (int32_t tc = 0; tc < table->cols; tc++)
        {
            // Free table cell memory
            TableCell *cell = &table->cells[row][tc];

            if (cell->content)
            {
                free(cell->content);
                cell->content = NULL;
            }

            if (cell->conCells)
            {
                free(cell->conCells);
                cell->conCells = NULL;
            }

            cell->size = 0;
            cell->fgColor = FWHITE;
            cell->bgColor = BBLACK;
        }

        // Empty framebuffer cells
        for (int32_t fc = 0; fc < con->cols; fc++)
        {
            setCellData(con, row, fc, FWHITE, BBLACK, L' ');
        }
    }
    else
    {
        // Else remove row and move all below one row up
        for (int32_t tr = row; tr < table->rows; tr++)
        {
            // Break if row is last row
            if (tr + 1 == table->rows)
            {
                continue;
            }

            for (int32_t tc = 0; tc < table->cols; tc++)
            {
                TableCell *nextCell = &table->cells[tr + 1][tc];
                setCellValue(table, nextCell->content, tr, tc, nextCell->fgColor, nextCell->bgColor);
            }
        }

        // Remove last row which should be empty
        // Free cell memory and reset their values
        for (int32_t tc = 0; tc < table->cols; tc++)
        {
            // Free table cell memory
            TableCell *cell = &table->cells[table->rows - 1][tc];

            if (cell->content)
            {
                free(cell->content);
                cell->content = NULL;
            }

            if (cell->conCells)
            {
                free(cell->conCells);
                cell->conCells = NULL;
            }

            cell->size = 0;
            cell->fgColor = FWHITE;
            cell->bgColor = BBLACK;
        }

        // Empty framebuffer cells
        for (int32_t fc = 0; fc < con->cols; fc++)
        {
            setCellData(con, table->rows - 1, fc, FWHITE, BBLACK, L' ');
        }
    }

    table->rows--;
}

void addTableCol(Table *table, Console *con)
{
    // Calculate separators for new cell size
    int32_t usableCols = con->cols;
    if (usableCols % 2 != 0)
        usableCols--;

    int32_t colWidth = usableCols / (table->cols + 1);

    int32_t *separators = malloc((table->cols + 1) * sizeof(int32_t));
    if (!separators)
        return;

    for (int32_t j = 0; j < table->cols + 1; j++)
        separators[j] = (j + 1) * colWidth;

    // Temporarily store copies of all cells to set them again later
    TableCell tableCells[table->rows][table->cols];
    // char *tableContents[table->rows][table->cols];
    for (int32_t r = 0; r < table->rows; r++)
    {
        for (int32_t c = 0; c < table->cols; c++)
        {
            tableCells[r][c] = table->cells[r][c];
        }
    }

    for (int32_t r = 0; r < table->rows; r++)
    {
        table->cells[r] = malloc((table->cols + 1) * sizeof(TableCell));
        if (!table->cells[r])
            return;

        for (int32_t c = 0; c < table->cols + 1; c++)
        {
            TableCell *cell = &table->cells[r][c];

            int32_t startCol = (c == 0) ? 0 : separators[c - 1] + 1;
            int32_t endCol = separators[c] - 1;
            cell->size = endCol - startCol + 1;

            if (cell->size <= 0)
            {
                cell->size = 0;
                cell->conCells = NULL;
                continue; // skip this cell
            }

            cell->fgColor = FWHITE;
            cell->bgColor = BBLACK;

            cell->content = malloc((cell->size + 1) * sizeof(char));
            if (!cell->content)
            {
                perror("Column content malloc");
                exit(1);
            }
            memset(cell->content, ' ', cell->size);
            cell->content[cell->size] = '\0';

            cell->conCells = malloc(cell->size * sizeof(Cell *));
            if (!cell->conCells)
                return;

            for (int i = 0; i < cell->size; i++)
            {
                int fbRow = r;
                int fbCol = c * cell->size + i;

                if (fbCol >= con->cols)
                {
                    cell->conCells[i] = NULL;
                    continue;
                }

                cell->conCells[i] = &con->framebuffer[fbRow][fbCol];
            }
        }
    }

    // Restore old cell contents
    for (int32_t r = 0; r < table->rows; r++)
    {
        for (int32_t c = 0; c < table->cols; c++)
        {
            setCellValue(table, tableCells[r][c].content, r, c, tableCells[r][c].fgColor, tableCells[r][c].bgColor);
        }
    }

    free(separators);

    table->cols++;
}

void removeTableCol(Table *table, Console *con, int32_t col)
{
    if (col < 0 || col + 1 > table->cols)
        return;

    // Calculate separators for new cell size
    int32_t usableCols = con->cols;
    if (usableCols % 2 != 0)
        usableCols--;

    int32_t colWidth = usableCols / (table->cols - 1);

    int32_t *separators = malloc((table->cols - 1) * sizeof(int32_t));
    if (!separators)
        return;

    for (int32_t j = 0; j < table->cols - 1; j++)
        separators[j] = (j + 1) * colWidth;

    // If last column set fb column to empty
    if (col + 1 == table->cols)
    {
        for (int32_t tr = 0; tr < table->rows; tr++)
        {
            TableCell *cell = &table->cells[tr][col];

            if (cell->content)
            {
                free(cell->content);
                cell->content = NULL;
            }

            if (cell->conCells)
            {
                free(cell->conCells);
                cell->conCells = NULL;
            }

            cell->size = 0;
            cell->fgColor = FWHITE;
            cell->bgColor = BBLACK;
        }

        for (int32_t fr = 0; fr < con->rows; fr++)
        {
            for (int32_t fc = usableCols - table->cells[0][table->cols - 1].size; fc < con->cols; fc++)
            {
                setCellData(con, fr, fc, FWHITE, BBLACK, L' ');
            }
        }
    }
    else
    {
        // Else remove column and shift all columns to its right one to the left
        // Temporarily store all cell contents to set them again later
        char *tableContents[table->rows][table->cols - 1];
        for (int32_t r = 0; r < table->rows; r++)
        {
            for (int32_t c = 0; c < table->cols - 1; c++)
            {
                if (c == col)
                    continue;
                tableContents[r][c] = table->cells[r][c].content;
            }
        }

        // Shift columns
        for (int32_t tr = 0; tr < table->rows; tr++)
        {
            for (int32_t tc = col; tc < table->cols - 1; tc++)
            {
                if (tc + 1 == table->cols)
                {
                    break;
                }

                TableCell *nextCell = &table->cells[tr][tc + 1];
                setCellValue(table, nextCell->content, tr, tc, nextCell->fgColor, nextCell->bgColor);
            }
        }

        // Remove last column
        for (int32_t tr = 0; tr < table->rows; tr++)
        {
            TableCell *cell = &table->cells[tr][table->cols - 1];

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

        for (int32_t fr = 0; fr < table->rows; fr++)
        {
            for (int32_t fc = usableCols - table->cells[0][table->cols - 1].size; fc < con->cols; fc++)
            {
                setCellData(con, fr, fc, FWHITE, BBLACK, L' ');
            }
        }
    }

    free(separators);

    table->cols--;
}