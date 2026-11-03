#ifndef MAZE_PARSER_H
#define MAZE_PARSER_H

#include <stdio.h>

/// <summary>Read a rectangular maze into one writable array. The caller closes the file.</summary>
/// <param name="file">Open file that can be read and rewound.</param>
/// <param name="width">Address where the row width is written.</param>
/// <param name="height">Address where the number of rows is written.</param>
/// <returns>The grid to free after use, or NULL if reading or allocating it failed.</returns>
char *load_maze(FILE *file, int *width, int *height);

#endif