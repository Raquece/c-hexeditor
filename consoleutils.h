//
// hexeditor.c library file
// consoleutils.c
//
// Provides utilities for console interaction.
//

// Avoid redefinition errors during compilation
#ifndef FILE_CONSOLEUTILS_SEEN
#define FILE_CONSOLEUTILS_SEEN

#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

struct winsize w; // The window size of the current terminal
struct termios oldt; // The initial terminal on execution of the program.
struct termios currentt; // The current terminal after changing settings.

// Removes the requirement for an EOL at character input.
void toggleEOFRequirement()
{
    int c;
    // Removes the need for an EOL at character input
    currentt.c_lflag &= ~(ICANON | ECHO);          

    // Apply attributes to STDIN.
    tcsetattr(STDIN_FILENO, TCSANOW, &currentt);
}

// Hotswitch console to either initial or current setting.
// terminal: 0 for initial console, 1 for current console.
void restoreConsole(int terminal)
{
    if (terminal == 0)
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    } else if (terminal == 1)
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &currentt);
    }
}

// Initialises variables for these functions to work.
void initialiseConsoleutils()
{
    tcgetattr(STDIN_FILENO, &oldt);
    currentt = oldt;
    ioctl(0, TIOCGWINSZ, &w);
}

#endif
