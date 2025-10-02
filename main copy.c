#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include <windows.h>

// WORD → 2 bytes (16 bits)
// DWORD → 32 bits (4 bytes)
// QWORD → 64 bits (8 bytes)

typedef struct OriginalVals
{
    WORD originalAttributes;            // From csbi.wAttributes
    COORD originalBufferSize;           // From csbi.dwSize
    SMALL_RECT originalWindow;          // From csb.srWindow
    DWORD originalMode;                 // From GetConsoleMode()
    CONSOLE_CURSOR_INFO originalCursor; // From GetConsoleCursorInfo()
} OriginalVals;

typedef struct Console
{
    int rows;
    int cols;
    bool cursorVisible;
    OriginalVals original;
} Console;

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

void clearScreen(HANDLE hConsole)
{
    disableScroll();

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    SMALL_RECT scrollRect;
    COORD scrollTarget;
    CHAR_INFO fill;

    // Get the number of character cells in the current buffer.
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi))
    {
        return;
    }

    // Scroll the rectangle of the entire buffer.
    scrollRect.Left = 0;
    scrollRect.Top = 0;
    scrollRect.Right = csbi.dwSize.X;
    scrollRect.Bottom = csbi.dwSize.Y;

    // Scroll it upwards off the top of the buffer with a magnitude of the entire height.
    scrollTarget.X = 0;
    scrollTarget.Y = (SHORT)(0 - csbi.dwSize.Y);

    // Fill with empty spaces with the buffer's default text attribute.
    fill.Char.UnicodeChar = TEXT(' ');
    fill.Attributes = csbi.wAttributes;

    // Do the scroll
    ScrollConsoleScreenBuffer(hConsole, &scrollRect, NULL, scrollTarget, &fill);

    // Move the cursor to the top left corner too.
    csbi.dwCursorPosition.X = 0;
    csbi.dwCursorPosition.Y = 0;

    SetConsoleCursorPosition(hConsole, csbi.dwCursorPosition);
}

void printScreen(Console console, HANDLE hConsole)
{
    disableScroll();

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    SMALL_RECT scrollRect;
    COORD scrollTarget;
    CHAR_INFO fill;

    // Get the number of character cells in the current buffer.
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi))
    {
        return;
    }

    // Scroll the rectangle of the entire buffer.
    scrollRect.Left = 0;
    scrollRect.Top = 0;
    scrollRect.Right = csbi.dwSize.X;
    scrollRect.Bottom = csbi.dwSize.Y;

    // Scroll it upwards off the top of the buffer with a magnitude of the entire height.
    scrollTarget.X = 0;
    scrollTarget.Y = (SHORT)(0 - csbi.dwSize.Y);

    csbi.wAttributes = FOREGROUND_RED | BACKGROUND_RED | FOREGROUND_INTENSITY;

    fill.Char.UnicodeChar = TEXT(' ');
    fill.Attributes = csbi.wAttributes;

    // Do the scroll
    ScrollConsoleScreenBuffer(hConsole, &scrollRect, NULL, scrollTarget, &fill);
}

Console getWindowSize(Console *con, HANDLE hConsole)
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

    // Set original values
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

    return con;
}

void resetConsole(Console con, HANDLE hConsole)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);

    SetConsoleTextAttribute(hConsole, con.original.originalAttributes);
    SetConsoleScreenBufferSize(hConsole, con.original.originalBufferSize);
    SetConsoleWindowInfo(hConsole, TRUE, &con.original.originalWindow);

    SetConsoleMode(hConsole, con.original.originalMode);
    toggleCursor(con, hConsole, true);
    clearScreen(hConsole);
}

int main()
{
    HANDLE hStdOut;
    hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    Console console = initConsole(hStdOut);

    getWindowSize(&console, hStdOut);
    // toggleCursor(console, hStdOut, false);

    printScreen(console, hStdOut);

    getchar();

    // Reset cursor
    resetConsole(console, hStdOut);

    return 0;
}