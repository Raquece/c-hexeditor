// 
// hexeditor.c
// Main source file - use this file to compile project.
//
// Usage: ./hexeditor {File}
//
// A test file, test.txt, has been provided if you choose to use that. It is a copy of this file (possibly from some other version).
//
// IMPORTANT NOTE: APPLICATION CHANGES TERMINAL SETTINGS; TO RESTORE THEM, EXIT FROM THE APPLICATION NORMALLY
//                 basically
//                 DON'T USE SIGNAL INTERRUPT (CTRL + C) TO EXIT
//
// Notes: Application best used with a large terminal size.
//        In the program, navigate to different bytes using arrow keys
//
//        Edit bytes by typing in a hexadecimal value. A byte that is being editted has a red background.
//
//        Using the search key (S), search for a given pattern in the file as a hexadecimal number with no spaces. For example:
//        To search for ABCDE, type 6566676869 so that the output would show 0x6566676869
//        To save a file that has been editted, use W. This will commit the changes made to the file.
//        To abort any changes made, use X.
//
// Readability Notes: The code uses the word "line" to refer to a set of 16 bytes in the file.
//
// Design Choices:-
//
//        The contents of the file are loaded into a deque data structure (read top of deque.h for more info) as opposed
//        to saving all of the contents of the file into a string to avoid using excessive amounts of memory.
//        By doing this, a limited amount of memory can be used by pushing and pulling needed / unneeded lines.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <math.h>

#include "textutils.h"
#include "deque.h"
#include "consoleutils.h"

#define BUFFER_HEIGHT 10

enum EditorState {
    browsing,
    editing
} editorState = browsing;

enum SelectState
{
    single,
    multi
} selectState = single;

unsigned long int lineOffset; // The line offset in the file that the user has navigated to.
unsigned long int size; // The size of the file in bytes.
unsigned long int lineSize; // The number of lines in the file.
int bufferHeight = BUFFER_HEIGHT; // The true size of the buffer (different to preprocessor variable if file is small).
int written = 1; // Whether the changes have been written to the temporary file.
deque *fileBuffer; // The buffer containing the contents of the file. Explanation of data type at top.
FILE *file; // The temporary file that is being editted.

int x; // X position of cursor
int y; // Y position of cursor

int foundFlag; // The flag set if the searchBuffer in searchAlgorithm() is found.

// Loads a file to be editted.
// fileName: the location of the file
//
// Throws if fileName does not exist.
void loadFile(char *fileName)
{
    file = fopen(fileName, "r+");

    // Throw if file does not exist.
    if(!file)
    {
        fprintf(stderr, "Could not load file\nSupply file in arguments\nUsage: ./hexeditor {File}\n");
        exit(1);
    }

    // Finds the size of the file, and sets it to global variable size.
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    lineSize = ceill(size / 16);
    fseek(file, 0, SEEK_SET);
    
    // Changes the buffer height variable to ensure cursor doesn't overflow if file is small (if the line size of the file is less than BUFFER_HEIGHT).
    if (lineSize <= BUFFER_HEIGHT)
    {
    	bufferHeight = lineSize + 1;
    }
}

// Read a certain segment of a file
// offset: the start position of reading from the file
// bufferLength: the length of the fileBuffer being written to.
//
// Returns: the buffer at offset.
char *readFileContents(long int offset, int bufferLength)
{
    // Allocate enough memory for the buffer
    char *tempBuffer = (char *)malloc(bufferLength);

    // Naviage to the offset in the file and read to tempBuffer
    fseek(file, offset, SEEK_SET);
    fread(tempBuffer, bufferLength, 1, file);

    // Return to start of file and return the biffer.
    fseek(file, 0, SEEK_SET);
    return tempBuffer;
}

// Convert a hex character to an integer.
// hex: the hex character.
//
// Returns: the base 10 representation of the hexadecimal character.
// Note: returns -1 if not a hexadecimal character.
int convertHexChar(char hex) 
{
    if (hex >= '0' && hex <= '9')
    {
        return hex - '0';
    }
    if (hex >= 'A' && hex <= 'F')
    {
        return hex - 'A' + 10;
    }
    if (hex >= 'a' && hex <= 'f')
    {
        return hex - 'a' + 10;
    }

    return -1;
}

// Reads the contents of a file to the buffer
// offset: the starting position reading from the file
// lineCount: the length of the fileBuffer, or lines of the file that are read at one time.
void readFileLines(long int offset, int lineCount)
{
    // Count up to lineCount.
    for (int i = 0; i < lineCount; i++) // i refers to the line offset.
    {
        // Navigate to the offset of the file.
        fseek(file, offset + (i * 16), SEEK_SET);

        // Read the line in the file.
        char *line = (char *)malloc(16);
        fread(line, 16, 1, file);

        // Add it to the buffer
        insertDequeFront(fileBuffer, line);
    }
}

// Writes a character to a file
// offset: the offset of the character
// ch: the character to write
void writeCharToFile(long int offset, char ch)
{
    // Navigate to the position.
    fseek(file, offset, SEEK_SET);
    fputs(&ch, file);
    fseek(file, 0, SEEK_SET);
}

// Display a single line of the fileBuffer on the screen
// line: the line number of the row
void writeLine(long int line)
{
    // Prints the line offset as an 8 character long hexadecimal string
    printf("  0x%08lX   ", (line + lineOffset) * 16);

    // Display byte value
    for (int i = 0; i < 16; i++)
    {
        // Reverts background changes if the current byte is selected.
        if (x == i && y == line)
        {
            if (editorState == editing)
            {
                // Display red background
                printf(SGR_BACKGROUND_RED);
            }
            else
            {
                // Display black text with white background
                printf("\033[30;47m");
            }
        }

        // Display current byte as a 2 character long hexadecimal string.
        printf("%02X", readDequeByte(fileBuffer, line, i) &0xff);
        
        // Reverts any background changes if the byte is selected
        if (x == i && y == line)
        {
            printf(SGR_RESET);
        }
        printf(" ");
    }

    printf("    ");

    // Display ASCII section
    for (int i = 0; i < 16; i++)
    {
        char c = readDequeByte(fileBuffer, line, i); // Current character

        // Changes background if the current byte is selected
        if (x == i && y == line)
        {
            printf("\033[30;47m");
        }

        // Check if character is within range of displayable characters
        if (c >= 32 && c <= 126)
        {
            // If so, print character
            printf("%c", c);
        }
        else
        {
            // Otherwise print placeholder / dummy character.
            printf(".");
        }

        // Reverts background changes if the current byte is selected.
        if (x == i && y == line)
        {
            printf(SGR_RESET);
        }
        printf(" ");
    }

    printf("\n");
}

// Writes the fileBuffer
// bufferHeight: The height of the fileBuffer in lines
void writeBuffer(int bufferHeight)
{
    // Iterate through each line of the buffer that will be drawn.
    for (int i = 0; i < bufferHeight; i++)
    {
        // Skip if the line doesn't exist
        if ((lineOffset + i) * 16 > size)
        {
            break;
        }

        writeLine(i);
    }
}

// Draws the user interface to the terminal
void drawScreen()
{
    clear();
    
    // Header
    drawLine(SGR_BACKGROUND_WHITE, 0, ' ');
    centreText("\033[0;30;47m", 0, "Hex Editor");

    // Editor
    setCursorPos(0, 8);
    printf("               00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");
    setCursorPos(0, 10);
    writeBuffer(BUFFER_HEIGHT);

    // Bottom Toolbar
    distributeLines(" ", " \033[30;47m S \033[0;0m Pattern Search", 0, w.ws_row - 5, 2, 0);
    distributeLines(" \033[30;47m W \033[0;0m Write to File ", " \033[30;47m X \033[0;0m Quit ", 0, w.ws_row - 5, 2, 1);

    // Disable cursor blink
    printf("\e[?25l");
}

// Finds a buffer in file
// searchBuffer: the string to search for
// searchLength: the length of the search buffer
//
// Returns: the offset of the buffer (if its there)
long unsigned int searchAlgorithm(char *searchBuffer, int searchLength)
{
    // Iterate through each line of the file
    for (int i = 0; i < size / 16; i++) // i refers to the line offset in the file
    {
        char *readBuffer = readFileContents(i * 16, 16); // The contents of the line

        // Iterate through each byte in the line
        for (int j = 0; j < 16; j++) // j refers to the byte that is being inspected
        {
            // Check to see if first byte in search exists in line
            if (readBuffer[j] == searchBuffer[0])
            {
                // The goal of this segment of code is to read the same amount of bytes as the length of the search buffer, then to
                // verify that the string exists in the output.

                readBuffer = readFileContents(i * 16 + j, searchLength); // The contents of the file at the offset where the first byte was found.

                // Check if searchBuffer exists in next [searchLength] bytes.
                if (strstr(readBuffer, searchBuffer))
                {
                    free(readBuffer);
                    foundFlag = 1;
                    return i * 16 + j; // i * 16 + j is the location of the first byte in the file that was matched with searchBuffer
                }

                // If this does not match, return readBuffer to the original "contents of the line"
                char *readBuffer = readFileContents(i * 16, 16);
            }
        }

        free(readBuffer);
    }
    
    return 0;
}

// Replaces real file with temporary file (writes changes)
// argv: Arguments supplied through main
void writeTemporaryToRealFile(char **argv)
{
    char copyCommand[500];
    snprintf(copyCommand, 499, "cp %s.tmp %s", argv[1], argv[1]); // Creates command for copying the command i.e.   cp {FILE}.tmp {FILE}
    system(copyCommand); // Executes the command
}

int main(int argc, char **argv)
{
    // Initialise console utilities for screen resizing
    initialiseConsoleutils();

    // Load file which will be editted.
    loadFile(argv[1]);

    // Create temporary file for reading / writing changes without committing them.
    char copyCommand[500];
    char tempFile[250];
    snprintf(copyCommand, 499, "cp %s %s.tmp", argv[1], argv[1]); // Creates command for creating temporary file  i.e cp {FILE} {FILE}.tmp
    snprintf(tempFile, 249, "%s.tmp", argv[1]); // Creates the string for name of the temporary file    i.e {FILE}.tmp
    system(copyCommand); // Executes the command
    loadFile(tempFile);

    // Loads the first set of lines to the fileBuffer
    fileBuffer = buildDeque(BUFFER_HEIGHT, 16);
    readFileLines(0, BUFFER_HEIGHT);

    while (1)
    {
        // Checks if the editor has been set to browsing mode and the changes have not been written.
        if (editorState == browsing && !written)
        {
            // Writes the changes and marks the written flag.
            writeCharToFile(lineOffset + (y * 16) + x, readDequeByte(fileBuffer, y, x));
            written = 1;
        }

        drawScreen();

        // Handle user input.
        toggleEOFRequirement();
        int c = getchar();
        if (c == 88 || c == 120) // X (Quit)
        {
            clear();
            break;
        }
        else if (c == 87 || c == 119) // W (Write)
        {
            writeTemporaryToRealFile(argv);
            continue;
        }
        else if (c == 83 || c == 115) // S (Search)
        {
            // Create input panel
            drawLine(SGR_RESET, w.ws_row - 5, ' ');
            setCursorPos(w.ws_col / 2 - 17, w.ws_row - 5); // Sets cursor location to middle, assuming 34 characters are written during input
            restoreConsole(0);
            printf("0x");

            // Sanitise input
            char inputBufferRaw[33];
            fgets(inputBufferRaw, 33, stdin);
            char *inputBuffer;
            for (int i = 0; i < strlen(inputBufferRaw); i++) // i refers to the character input by user
            {
                // Checks that the character is a valid hexadecimal character
                if (!((inputBufferRaw[i] >= 'A' && inputBufferRaw[i] <= 'F') || (inputBufferRaw[i] >= 'a' && inputBufferRaw[i] <= 'f') || (inputBufferRaw[i] >= '0' && inputBufferRaw[i] <= '9')))
                {
                    // Appends it to the sanitised input.
                    inputBuffer = malloc(i + 1);
                    memcpy(inputBuffer, inputBufferRaw, i + 1);
                }
            }
            restoreConsole(1);
            int inputLength = strlen(inputBuffer) - 1; // The length of search input buffer.

            // Abort if the length of the sanitised user input is not divisible by 2 or is too long.
            if (inputLength % 2 != 0 || inputLength > 16)
            {
                continue;
            }

            // Create the search buffer
            char *searchBuffer = (char *)malloc(inputLength / 2); // The set of bytes that are being searched for in the file.
            // Iterate through each set of 2 characters (a byte) in the search input
            for (int i = 0; i < inputLength / 2; i++) // i refers to the location of the first character in the set of 2 that is being inspected
            {
                // Check if input contains non-hex character
                int hex1 = convertHexChar(inputBuffer[i * 2]); // The first digit of the byte 
                int hex2 = convertHexChar(inputBuffer[i * 2 + 1]); // The second digit of the byte
                if (hex1 == -1 || hex2 == -1) // Checks if either digits are invalid
                {
                    continue;
                }

                // Adds the byte to the search buffer.
                searchBuffer[i] = (16 * hex1) + hex2;
            }

            long unsigned int loc = searchAlgorithm(searchBuffer, inputLength / 2); // The location of the search buffer

            if (!foundFlag)
            {
                setCursorPos(w.ws_col / 2 - 32, w.ws_row - 5);
                restoreConsole(0);
                printf("LOCATION: NOT FOUND (press enter to continue)");
                getchar();
                restoreConsole(1);
                continue;
            }

            // Reset found flag for future use.
            foundFlag = 0;
            // Outputs location and reallocates memory / terminal settings
            setCursorPos(w.ws_col / 2 - 17, w.ws_row - 5);
            restoreConsole(0);
            printf("LOCATION: 0x%08lX (press enter to resume)", loc);
            getchar();
            restoreConsole(1);

            free(searchBuffer);
        }
        else if (c == 27) // Escape character (in context of this program, navigational keys)
        {
            // Reset editor back to browsing state.
            if (editorState == editing)
            {
                written = 0;
                editorState = browsing;
            }

            // Gets the character to see which directional key was pressed, because why would it need to be simple and it's own key instead of 3 others
            getchar();
            int d = getchar();
            switch (d)
            {
                case 65: // UP
                    // If the cursor is not already at the top, move it up one. Otherwise, check whether editor can move
                    if (y != 0)
                    {
                        y--;
                    }
                    else
                    {
                        // If the editor is at top already, break from switch statement
                        if (lineOffset == 0)
                        {
                            break;
                        }

                        // Move editor up and insert new line onto file buffer.
                        lineOffset--;
                        char *bufferInsUp = readFileContents((lineOffset - 1) * 16, 16);
                        pushDequeBack(fileBuffer, bufferInsUp);
                    }

                    break;
                case 66: // DOWN
                    // If the cursor is not already at the bottom, move it down one. Otherwise, check whether editor can move
                    if (y != bufferHeight - 1)
                    {
                        y++;
                    }
                    else
                    {
                        // If the editor is at bottom already, break from switch statement
                        if ((lineOffset + BUFFER_HEIGHT) > lineSize)
                        {
                            break;
                        }

                        // Move editor down and insert new line onto file buffer.
                        lineOffset++;
                        char *bufferInsDown = readFileContents((BUFFER_HEIGHT + lineOffset) * 16, 16);
                        pushDequeFront(fileBuffer, bufferInsDown);

                    }
                    break;

                case 67: // RIGHT
                    // If the cursor is not on the right of screen already, move right one. Otherwise keep the same.
                    x = x < 15 ? x + 1 : 15;
                    break;
                case 68: // LEFT
                    // If the cursor is not on the left of screen already, move left one. Otherwise keep the same.
                    x = x > 0 ? x - 1 : 0;
                    break;
                default:
                    break;
            }
        }
        else
        {
            // User is writing to a byte.
            int val = convertHexChar(c); // User input

            // nvm user isn't
            if (val == -1)
            {
                if (editorState = editing)
                {
                    // Flags the byte to be written since the editor is editing.
                    editorState = browsing;
                    written = 0;
                }
                continue;
            }

            switch (editorState)
            {
                case browsing:
                    // If the editor is in a browsing state, write the first digit of the byte to the fileBuffer.
                    writeDequeByte(fileBuffer, y, x, val);
                    editorState = editing;
                    written = 0;
                    break;

                case editing:
                    // If the editor is in a browsing state, write the byte to the fileBuffer, where the first digit is val and the second digit is the byte already present.
                    writeDequeByte(fileBuffer, y, x, readDequeByte(fileBuffer, y, x) * 16 + val);
                    editorState = browsing;
                    written = 0;
                    break;

                default:
                    // I don't know a case where this would ever get hit, but just to be safe.
                    fprintf(stderr, "Invalid enum value in editor state.\n");
                    exit(1);
                    break;
            }
        }
    }

    // Removes temporary file and restores console.
    char removeCommand[253];
    fclose(file);
    snprintf(removeCommand, 253, "rm %s", tempFile); // Creates command for removing temporary file i.e.  rm {tempFile}
    system(removeCommand); // Executes the command

    // Re-enable cursor blink and restores console.
    printf("\e[?25h");
    printf(SGR_RESET);
    restoreConsole(0);

    return 0;
}
