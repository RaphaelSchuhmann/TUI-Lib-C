#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include <windows.h>

// WORD → 2 bytes (16 bits)
// DWORD → 32 bits (4 bytes)
// QWORD → 64 bits (8 bytes)

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
            // For testing disabled:
            // cell.Foreground = con.original.originalAttributes & 0x0F;
            // cell.Background = con.original.originalAttributes & 0xF0;

            // TODO: Remove after testing:
            cell.Foreground = FOREGROUND_RED & 0x0F;
            cell.Background = BACKGROUND_RED & 0xF0;

            con.framebuffer[row][col] = cell;
        }
    }

    return con;
}

void setCellColor(Console *con, int row, int col, ColorForeground Fcolor, ColorBackground Bcolor) {
    con->framebuffer[row][col].Foreground = Fcolor;
    con->framebuffer[row][col].Background = Bcolor;
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
    toggleCursor(console, hStdOut, false);

    setCellColor(&console, 0, 5, FGREEN, BGREEN);

    CHAR_INFO *linearBuffer = framebufferToLinearBuffer(console);

    printScreen(hStdOut, linearBuffer, console);

    getchar();

    // Reset
    free(linearBuffer);
    resetConsole(console, hStdOut);

    return 0;
}