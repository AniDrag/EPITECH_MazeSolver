#include "maze_parser.h"  
#include <stdlib.h> 
#include <string.h>      

#define MAX_MAZE_SIZE 10000  

/*
    Load a rectangular maze from a file into one flat, writable array.
 
    On success, returns the maze and writes its dimensions through width
    and height.
 
    On failure, prints the reason to stderr and returns NULL.
    The caller is responsible for closing the file in either case.
 */
char *load_maze(FILE *mazeFile, int *width, int *height)
{
    
    // 10,000 cells + '\r' + '\n' + the '\0' that fgets adds.
    char row[MAX_MAZE_SIZE + 3];

    *width = 0;
    *height = 0;

    // PASS 1: Find the dimensions before allocating the maze.
    // Each successful fgets reads the next row into the same buffer.
    while (fgets(row, sizeof row, mazeFile) != NULL) {

        size_t length = strcspn(row, "\r\n");

        int rowNumber = *height + 1;
        if (*height >= MAX_MAZE_SIZE) {
            fprintf(stderr, "Maze has more than %d rows.\n", MAX_MAZE_SIZE);
            return NULL;
        }
        if (length == 0) {
            fprintf(stderr, "Maze row %d is empty. Each row needs at least one cell.\n", rowNumber);
            return NULL;
        }
        if (length > MAX_MAZE_SIZE) {
            fprintf(stderr, "Maze row %d is too wide. The limit is %d cells per row.\n", rowNumber, MAX_MAZE_SIZE);
            return NULL;
        }

        // The first row establishes the width. Every later row must match
        if (*height == 0) {
            *width = (int)length;
        } else if (length != (size_t)*width) {
            fprintf(stderr, "Maze row %d has %zu cells. Expected %d to match row 1.\n", rowNumber, length, *width);
            return NULL;
        }

        // Parentheses matter: increment the row count stored in main.
        (*height)++;
    }

    // fgets returns NULL both at end-of-file and on a read error.
    // Reaching the end is expected. A read error or an empty file is not.
    if (ferror(mazeFile)) {
        fprintf(stderr, "Could not read the maze file. Successfully read %d rows.\n", *height);
        return NULL;
    }
    if (*height == 0) {
        fputs("The maze file is empty. Add at least one row of cells.\n", stderr);
        return NULL;
    }

    // The first pass left us at the end. Move back to byte 0 so the
    // second pass starts with the first row again.
    if (fseek(mazeFile, 0, SEEK_SET) != 0) {
        fputs("Could not return to the start of the maze file to load its cells.\n", stderr);
        return NULL;
    }

    // Allocate one byte per cell. This is a grid accessed by index,
    // so we don't need a newline between rows or a final '\0'.
    char *maze = malloc((size_t)*width * *height);
    if (maze == NULL) {
        fprintf(stderr, "Not enough memory to load a %d by %d maze.\n", *width, *height);
        return NULL;
    }

    // PASS 2: Copy the cells from each row into the allocated grid.
    for (int r = 0; r < *height; r++) {
        // Check that the row was read and still has the expected width.
        // From here on, we own allocated memory, so free it on failure.
        if (fgets(row, sizeof row, mazeFile) == NULL) {
            if (ferror(mazeFile)) {
                fprintf(stderr, "Read error while loading maze row %d.\n", r + 1);
            } else {
                fprintf(stderr, "Maze file ended before row %d. Expected %d rows.\n", r + 1, *height);
            }
            free(maze);
            return NULL;
        }
        size_t length = strcspn(row, "\r\n");
        if (length != (size_t)*width) {
            fprintf(stderr, "Maze row %d changed width while loading. Expected %d cells, found %zu.\n",
                    r + 1, *width, length);
            free(maze);
            return NULL;
        }

        // memcpy(destination, source, number_of_bytes)
        // Row r starts at index r * width. Copy exactly width bytes
        // from row, leaving its line ending behind.
        memcpy(maze + (size_t)r * *width, row, (size_t)*width);
    }

    return maze;
}