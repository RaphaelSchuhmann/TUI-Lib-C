#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include <windows.h>
#include "tcon.h"

void toggleCursor(Console console, HANDLE hConsole, bool reset)
{
    if (reset)
    {
        SetConsoleCursorInfo(hConsole, &console.original.originalCursor);
        return;
    }

    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = !console.cursorVisible;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

void disableScroll()
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    GetConsoleScreenBufferInfo(hOut, &csbi);

    // Set buffer size equal to window size
    COORD newSize;
    newSize.X = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    newSize.Y = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    SetConsoleScreenBufferSize(hOut, newSize);
}

CHAR_INFO *framebufferToLinearBuffer(Console con)
{
    CHAR_INFO *charInfos = malloc(con.rows * con.cols * sizeof(CHAR_INFO));

    int k = 0;

    for (int row = 0; row < con.rows; row++)
    {
        for (int col = 0; col < con.cols; col++)
        {
            CHAR_INFO temp;
            temp.Char.UnicodeChar = con.framebuffer[row][col].Char;
            temp.Attributes = con.framebuffer[row][col].Foreground | con.framebuffer[row][col].Background;

            k = row * con.cols + col;
            charInfos[k] = temp;
        }
    }

    return charInfos;
}

void printScreen(HANDLE hConsole, CHAR_INFO *charInfo, Console con)
{
    COORD bufferSize;
    bufferSize.X = con.cols;
    bufferSize.Y = con.rows;

    COORD bufferCoord;
    bufferCoord.X = 0;
    bufferCoord.Y = 0;

    SMALL_RECT writeRegion;
    writeRegion.Top = 0;
    writeRegion.Left = 0;
    writeRegion.Right = con.cols - 1;
    writeRegion.Bottom = con.rows - 1;

    WriteConsoleOutput(hConsole, charInfo, bufferSize, bufferCoord, &writeRegion);
}

void clearScreen(HANDLE hConsole, Console *con, bool hlt)
{
    for (int r = 0; r < con->rows; r++)
    {
        for (int c = 0; c < con->cols; c++) 
        {
            con->framebuffer[r][c].Char = L' ';
            con->framebuffer[r][c].Background = BBLACK;
            con->framebuffer[r][c].Foreground = FWHITE;
        }
    }

    renderConsole(*con, hConsole, hlt);
}

void getWindowSize(Console *con, HANDLE hConsole)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    if (hConsole == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "Error: invalid handle\n");
        exit(EXIT_FAILURE);
    }

    if (!GetConsoleScreenBufferInfo(hConsole, &csbi))
    {
        fprintf(stderr, "Error: could not get console info\n");
        exit(EXIT_FAILURE);
    }

    int columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    con->rows = rows;
    con->cols = columns;
}

Console initConsole(HANDLE hConsole)
{
    Console con;
    con.rows = 0;
    con.cols = 0;
    con.cursorVisible = true;

    getWindowSize(&con, hConsole);

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);

    DWORD mode;
    GetConsoleMode(hConsole, &mode);

    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);

    con.original.originalAttributes = csbi.wAttributes;
    con.original.originalBufferSize = csbi.dwSize;
    con.original.originalWindow = csbi.srWindow;
    con.original.originalMode = mode;
    con.original.originalCursor = cursorInfo;

    con.framebuffer = malloc(con.rows * sizeof(Cell *));
    for (int i = 0; i < con.rows; i++)
    {
        con.framebuffer[i] = malloc(con.cols * sizeof(Cell));
    }

    // Populate Cells
    for (int row = 0; row < con.rows; row++)
    {
        for (int col = 0; col < con.cols; col++)
        {
            Cell cell = con.framebuffer[row][col];
            cell.Char = L' ';

            cell.Foreground = con.original.originalAttributes & 0x0F;
            cell.Background = con.original.originalAttributes & 0xF0;

            con.framebuffer[row][col] = cell;
        }
    }

    return con;
}

void setCellData(Console *con, int row, int col, ColorForeground Fcolor, ColorBackground Bcolor, wchar_t Char)
{
    con->framebuffer[row][col].Foreground = Fcolor;
    con->framebuffer[row][col].Background = Bcolor;
    con->framebuffer[row][col].Char = Char;
}

void resetConsole(Console *con, HANDLE hConsole)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    
    SetConsoleTextAttribute(hConsole, con->original.originalAttributes);
    SetConsoleScreenBufferSize(hConsole, con->original.originalBufferSize);
    SetConsoleWindowInfo(hConsole, TRUE, &con->original.originalWindow);
    SetConsoleMode(hConsole, con->original.originalMode);

    clearScreen(hConsole, con, false);
    toggleCursor(*con, hConsole, true);

    COORD topLeft = {0, 0};
    SetConsoleCursorPosition(hConsole, topLeft);
}

void renderConsole(Console con, HANDLE hConsole, bool hlt)
{
    CHAR_INFO *linearBuffer = framebufferToLinearBuffer(con);
    printScreen(hConsole, linearBuffer, con);

    if (hlt)
        getchar();

    free(linearBuffer);
}