#include <windows.h>
#include <stdbool.h>

#ifndef TUI_H
#define TUI_H

typedef enum ColorForeground
{
    FBLACK = 0,
    FDARKBLUE = FOREGROUND_BLUE,
    FDARKGREEN = FOREGROUND_GREEN,
    FDARKCYAN = FOREGROUND_GREEN | FOREGROUND_BLUE,
    FDARKRED = FOREGROUND_RED,
    FDARKMAGENTA = FOREGROUND_RED | FOREGROUND_BLUE,
    FDARKYELLOW = FOREGROUND_RED | FOREGROUND_GREEN,
    FDARKGRAY = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
    FGRAY = FOREGROUND_INTENSITY,
    FBLUE = FOREGROUND_INTENSITY | FOREGROUND_BLUE,
    FGREEN = FOREGROUND_INTENSITY | FOREGROUND_GREEN,
    FCYAN = FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE,
    FRED = FOREGROUND_INTENSITY | FOREGROUND_RED,
    FMAGENTA = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE,
    FYELLOW = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN,
    FWHITE = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
} ColorForeground;

typedef enum ColorBackground
{
    BBLACK = 0,
    BDARKBLUE = BACKGROUND_BLUE,
    BDARKGREEN = BACKGROUND_GREEN,
    BDARKCYAN = BACKGROUND_GREEN | BACKGROUND_BLUE,
    BDARKRED = BACKGROUND_RED,
    BDARKMAGENTA = BACKGROUND_RED | BACKGROUND_BLUE,
    BDARKYELLOW = BACKGROUND_RED | BACKGROUND_GREEN,
    BDARKGRAY = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE,
    BGRAY = BACKGROUND_INTENSITY,
    BBLUE = BACKGROUND_INTENSITY | BACKGROUND_BLUE,
    BGREEN = BACKGROUND_INTENSITY | BACKGROUND_GREEN,
    BCYAN = BACKGROUND_INTENSITY | BACKGROUND_GREEN | BACKGROUND_BLUE,
    BRED = BACKGROUND_INTENSITY | BACKGROUND_RED,
    BMAGENTA = BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_BLUE,
    BYELLOW = BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN,
    BWHITE = BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE,
} ColorBackground;

typedef struct OriginalVals
{
    WORD originalAttributes;            // From csbi.wAttributes
    COORD originalBufferSize;           // From csbi.dwSize
    SMALL_RECT originalWindow;          // From csb.srWindow
    DWORD originalMode;                 // From GetConsoleMode()
    CONSOLE_CURSOR_INFO originalCursor; // From GetConsoleCursorInfo()
} OriginalVals;

typedef struct Cell
{
    wchar_t Char;
    WORD Foreground;
    WORD Background;
} Cell;

typedef struct Console
{
    int rows;
    int cols;
    bool cursorVisible;
    Cell **framebuffer;
    OriginalVals original;
} Console;

void toggleCursor(Console console, HANDLE hConsole, bool reset);
void disableScroll();
CHAR_INFO *framebufferToLinearBuffer(Console con);
void printScreen(HANDLE hConsole, CHAR_INFO *charInfo, Console con);
void clearScreen(HANDLE hConsole);
Console getWindowSize(Console *con, HANDLE hConsole);
Console initConsole(HANDLE hConsole);
void setCellData(Console *con, int row, int col, ColorForeground Fcolor, ColorBackground Bcolor, wchar_t Char);
void resetConsole(Console con, HANDLE hConsole);

#endif