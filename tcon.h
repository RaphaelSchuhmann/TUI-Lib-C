#include <windows.h>
#include <stdbool.h>
#include <inttypes.h>

#ifndef TCON_H
#define TCON_H

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
    int32_t rows;
    int32_t cols;
    bool cursorVisible;
    Cell **framebuffer;
    OriginalVals original;
} Console;


/*
Toggles the visibility of the cursor in the terminal.

Arguments:
   console - the current instance of the console
   hConsole - the windows console api handler
   reset - a flag which is used to reset the cursor to its original state
 
Returns:
   Void
*/
void toggleCursor(Console console, HANDLE hConsole, bool reset);

/*
Resizes the terminal buffer so there is no scrollable area.

Arguments:
   Void
 
Returns:
   Void
*/
void disableScroll();

/*
Casts the 2d framebuffer into a 1d CHAR_INFO array

Arguments:
   console - the current instance of the console
 
Returns:
   A 1d CHAR_INFO array
*/
CHAR_INFO *framebufferToLinearBuffer(Console con);

/*
Writes a CHAR_INFO array to the console buffer.

Arguments:
    hConsole - the windows console api handler
    charInfo - a 1d array of type CHAR_INFO containing the data to print
    console - the current instance of the console
 
Returns:
   Void
*/
void printScreen(HANDLE hConsole, CHAR_INFO *charInfo, Console con);

/*
Clears the console screen by filling the console buffer with empty spaces and 
resizing the console buffer to remove the scrollable area.

Arguments:
   hConsole - the windows console api handler
   console - the current instance of the console
   hlt - a flag which when flagged makes the program wait for user input on rerender
 
Returns:
   Void
*/
void clearScreen(HANDLE hConsole, Console *con, bool hlt);

/*
Gets the current width and height of the console screen buffer and
updates the fields "rows" and "cols" in the con parameter.

Arguments:
   con - the current instance of the console in form of a pointer
   hConsole - the windows console api handler
 
Returns:
   Void
*/
void getWindowSize(Console *con, HANDLE hConsole);

/*
Initializes the console by creating a new instance of Console, getting and storing
all needed default values for reseting, and creates and populates all cells in the
framebuffer.

Arguments:
   hConsole - the windows console api handler
 
Returns:
   Console
*/
Console initConsole(HANDLE hConsole);

/*
Sets foreground color, background color, and char of a given cell in the framebuffer.

Arguments:
   console - the current instance of the console
   row - the row of the cell in the framebuffer
   col - the column of the cell in the framebuffer
   Fcolor - the foreground color for the cell
   Bcolor - the background color for the cell
   Char - the char that gets set to the cell
 
Returns:
   Void
*/
void setCellData(Console *con, int32_t row, int32_t col, ColorForeground Fcolor, ColorBackground Bcolor, wchar_t Char);

/*
Resets all console values to pre execution state

Arguments:
   console - the current instance of the console
   hConsole - the windows console api handler
 
Returns:
   Void
*/
void resetConsole(Console *con, HANDLE hConsole);

/*
Turns the current framebuffer into a linear buffer and prints it to the screen.
If the hlt flag is true it also waits for user input

Arguments:
   console - the current instance of the console
   hConsole - the windows console api handler
   hlt - a flag which when flagged makes the program wait for user input
 
Returns:
   Void
*/
void renderConsole(Console con, HANDLE hConsole, bool hlt);

#endif