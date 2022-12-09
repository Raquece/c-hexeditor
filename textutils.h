//
// hexeditor.c library file
// textutils.c
//
// Provides utilities for output and text utilities.
//

// Avoid redefinition errors during compilation
#ifndef FILE_TEXTUTILS_SEEN
#define FILE_TEXTUTILS_SEEN

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "consoleutils.h"

#define clear() printf("\e[1;1H\e[2J")
#define setCursorPos(x,y) printf("\033[%d;%dH", (y), (x))
#define SGR_RESET "\033[0;0m"
#define SGR_BOLD "\033[0;1m"
#define SGR_FAINT "\033[0;2m"
#define SGR_ITALIC "\033[0;3m"
#define SGR_UNDERLINE "\033[0;4m"
#define SGR_STRIKE "\033[0;9m"
#define SGR_FOREGROUND_BLACK "\033[0;30m"
#define SGR_FOREGROUND_RED "\033[0;31m"
#define SGR_FOREGROUND_GREEN "\033[0;32m"
#define SGR_FOREGROUND_YELLOW "\033[0;33m"
#define SGR_FOREGROUND_BLUE "\033[0;34m"
#define SGR_FOREGROUND_MAGENTA "\033[0;35m"
#define SGR_FOREGROUND_CYAN "\033[0;36m"
#define SGR_FOREGROUND_WHITE "\033[0;37m"
#define SGR_BACKGROUND_WHITE "\033[0;47m"
#define SGR_BACKGROUND_RED "\033[0;41m"

// Draws a character for an entire line
// colour: the ANSI code for any specified colours
// y: the y location of the line
// character: the character that the line will be filled with
void drawLine(char *colour, int y, char character)
{
    puts(colour);
    setCursorPos(0, y);
    char fill[w.ws_col + 1]; // The string that will be written
    memset(fill, character, w.ws_col);
    fill[w.ws_col] = '\0'; // Inject terminator character
    puts(fill);
    puts(SGR_RESET);
}

// Centres text on the screen
// colour: the ANSI code for any specified colours
// y: the y location of the line
// text: the text that will be centred.
void centreText(char *colour, int y, char *text)
{
    puts(colour);
    int x = (w.ws_col / 2) - (strlen(text) / 2);
    setCursorPos(x, y);
    puts(text);
    puts(SGR_RESET);
}

// Evenly distribute multiple strings onto a line.
// strA: string A
// strB: string B
// startPos: x starting position
// y: the y location of the line
// widthSegments: the number of segments in the line, with two strings occupying a segment. e.g. for 4 strings, 2 segments; 6 strings, 3 segments, etc. etc.
// segment: the number segment this set occupies. 
void distributeLines(char* strA, char* strB, int startPos, int y, int widthSegments, int segment) {
    float p1 = 0.25; // Percentage width position of string A
    float p2 = 0.75; // Percentage width position of string B

    int width = (w.ws_col / widthSegments); // Width of the segment

    int strALen = strlen(strA);
    int strBLen = strlen(strB);

    int pos1 = startPos + ((width * p1) - (strALen / 2)); // Position of string A
    int pos2 = startPos + ((width * p2) - (strBLen / 2)); // Position of string B

    // Prints text in correct location
    setCursorPos(pos1 + (segment * width), y);
    puts(strA);
    setCursorPos(pos2 + (segment * width), y);
    puts(strB);
}

#endif
