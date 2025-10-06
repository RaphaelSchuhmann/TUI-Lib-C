#include "tui.h"
#include <stdio.h>

int main()
{
    HANDLE hStdOut;
    hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    Console console = initConsole(hStdOut);

    getWindowSize(&console, hStdOut);
    toggleCursor(console, hStdOut, false);

    for (int i = 5; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            setCellColor(&console, i, j, FGREEN, BGREEN);
        }
    }

    CHAR_INFO *linearBuffer = framebufferToLinearBuffer(console);

    printScreen(hStdOut, linearBuffer, console);

    getchar();

    // Reset
    free(linearBuffer);
    resetConsole(console, hStdOut);

    return 0;
}