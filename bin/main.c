    #include "maze_parser.h"
    #include "utility.h"
    #include <stdio.h>
    #include <stdlib.h>
// The compiler defines _WIN32 for Windows targets, even 64-bit Windows.
// #ifdef picks this branch at compile time. #else picks the other one.
#ifdef _WIN32
    #include <windows.h> // Windows API declarations for console handles and modes.
#else
    #include <unistd.h> // On POSIX systems this declares isatty and STDOUT_FILENO.
#endif


    /* PSUDO CODE


    if start or finish is a wall:
        return NO_PATH

    set every direction to UNSEEN
    direction[finish] = END
    enqueue(finish)

    while the queue is not empty:
        current = dequeue()

        if current == start:
            break

        for each of the four neighbors of current:
            skip it if outside the maze, a wall, or already seen
        direction[neighbor] = the step from neighbor back to current
        enqueue(neighbor)

    if direction[start] == UNSEEN:
        return NO_PATH

    cell = start
    while cell != finish:
        maze[cell] = 'o'
        cell = take the step stored in direction[cell]
    maze[finish] = 'o'

    return SOLVED
    */

    /// <summary>Check that both ends are open. A blocked end frees the grid and stops the solver.</summary>
    /// <param name="maze">Maze with its start and finish indices already set.</param>
    void validate_maze(Maze *maze)
    {
        if (is_wall(maze->grid[maze->start])) {
            puts("no solution found! Start is a wall");
            free(maze->grid);
            exit(EXIT_FAILURE);
        }
        if (is_wall(maze->grid[maze->finish])) {
            puts("no solution found! End is a wall");
            free(maze->grid);
            exit(EXIT_FAILURE);
        }
    }

    /// <summary>Print the solved grid row by row, using color when the output supports it.</summary>
    /// <param name="maze">The grid to print and the dimensions needed to find each row.</param>
    void print_maze(Maze *maze)
    {
#ifdef _WIN32
        // DWORD is a 32-bit unsigned Windows type. GetStdHandle gets the handle for stdout.
        // &mode passes its address so GetConsoleMode can fill it with the console flags.
        // && checks the flags only if that call worked. A redirected file fails the console check.
        // The single & tests one bit. If set, the console understands ANSI color codes.
        DWORD mode;
        bool use_color = GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &mode) &&
            (mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#else
        // STDOUT_FILENO is the descriptor number for stdout. isatty checks if it leads to a terminal.
        bool use_color = isatty(STDOUT_FILENO);
#endif
        for (int row = 0; row < maze->height; row++) {
            const char *row_start = maze->grid + (size_t)row * maze->width;
            if (!use_color) {
                // fwrite takes a start address, bytes per item, item count, and stream.
                // One cell is one byte. (size_t) converts the width to the count type fwrite expects.
                // This writes exactly one row, so the grid doesn't need a string terminator.
                fwrite(row_start, 1, (size_t)maze->width, stdout);
                // fputc writes one character. '\n' starts the next row.
                fputc('\n', stdout);
                continue;
            }
            int column = 0;

            // Group adjacent cells with the same color so each run can be written at once.
            while (column < maze->width) {
                int color = color_for_cell(row_start[column]);
                int run_start = column++;

                while (column < maze->width && color_for_cell(row_start[column]) == color)
                    column++;

                // fprintf formats text for a stream. %d inserts the color number.
                // \x1b is the ESC byte in hex. With color 32 this sends ESC [ 3 2 m
                // (no spaces), which tells an ANSI terminal to switch to green.
                fprintf(stdout, "\x1b[%dm", color);
                // Start at the first cell of this color. sizeof *maze->grid is the size of one cell.
                // column is just past the run, so column - run_start is how many cells to write.
                fwrite(row_start + run_start, sizeof *maze->grid,
                    (size_t)(column - run_start), stdout);
            }

            // Reset the terminal color, then end this maze row.
            fputs("\x1b[0m", stdout);
            // Finish the printed row with a newline.
            fputc('\n', stdout);
        }
    }
    
    /// <summary>Follow the saved directions from start to finish and mark that path with o.</summary>
    /// <param name="maze">Grid whose path cells will be changed.</param>
    /// <param name="map">One direction per cell, pointing toward the finish.</param>
    void modify_maze(Maze *maze, unsigned char *map)
    {
        int cellIndex = maze->start;

        while (cellIndex != maze->finish) 
        {
            int offset = next_cell(map[cellIndex], maze->width);

            if (offset == 0) 
            {
                fprintf(stderr, "Invalid direction at cell %d\n", cellIndex);
                free(map);
                free(maze->grid);
                exit(EXIT_FAILURE);
            }

            maze->grid[cellIndex] = 'o';
            cellIndex += offset;
        }

        maze->grid[maze->finish] = 'o';
    }


    /// <summary>Load the maze, search from the finish, then print the path from the start.</summary>
    /// <param name="argc">Number of command-line arguments, including the program name.</param>
    /// <param name="argv">Array of argument strings. argv[1] is the maze filename.</param>
    /// <returns>Zero on success or EXIT_FAILURE on error.</returns>
    int main(int argc, char **argv)
    {
        // ./solver maze.txt gives two arguments: argv[0] is the program and argv[1] is the file.
        // != means "is different from", so this catches both missing and extra arguments.
        if (argc != 2) {
            // stderr is the error stream. %s prints the program name even if I rename the executable.
            fprintf(stderr, "Usage: %s <maze file>\n", argv[0]);
            // EXIT_FAILURE comes from stdlib.h. Returning it tells the shell the command failed.
            return EXIT_FAILURE;
        }

        // "rb" opens the file for reading as bytes, so the parser sees the original line endings.
        FILE *file = fopen(argv[1], "rb");
        if (file == NULL) 
        {
            perror(argv[1]);
            return EXIT_FAILURE;
        }

        Maze maze = {0};
        // & gives the parser the addresses of these fields so it can write the dimensions back.
        maze.grid = load_maze(file, &maze.width, &maze.height);
        fclose(file);

        if (maze.grid == NULL) 
        {
            return EXIT_FAILURE;
        }
    
        maze.start = 0;
        maze.finish = maze.width * maze.height - 1;

        validate_maze(&maze);

        size_t capacity = (size_t)maze.width * maze.height;

        Queue queue = {0};

        // calloc zeros out my direction map ;) One entry per cell, each sizeof *map bytes.
        // Unseen is 0, so every cell starts unseen without a separate loop.
        unsigned char *map = calloc(capacity, sizeof *map);
        // || stops if the map allocation failed, so enqueue_cell never gets a null map.
        // Otherwise, seed the queue with the finish. ! catches false if its first allocation fails.
        if (map == NULL || !enqueue_cell(&queue, map, maze.finish, End))
            goto out_of_memory;

        while (queue.count > 0){
            int currentCell = dequeue_cell(&queue);

            if (currentCell == maze.start) break;

            if(CAN_GO_UP(currentCell,maze.width)){
                int aboveCell = currentCell - maze.width;

                if(!is_wall(maze.grid[aboveCell]) && map[aboveCell] == Unseen){
                    if (!enqueue_cell(&queue, map, aboveCell, Down))
                        goto out_of_memory;
                }
            }
            if(CAN_GO_LEFT(currentCell,maze.width)){
                int leftCell = currentCell - 1;

                if(!is_wall(maze.grid[leftCell]) && map[leftCell] == Unseen){
                    if (!enqueue_cell(&queue, map, leftCell, Right))
                        goto out_of_memory;
                }
            }
            if(CAN_GO_DOWN(currentCell,maze.width,maze.height)){
                int downCell = currentCell + maze.width;

                if(!is_wall(maze.grid[downCell]) && map[downCell] == Unseen)
                    if (!enqueue_cell(&queue, map, downCell, Up))
                        goto out_of_memory;
            }
            if(CAN_GO_RIGHT(currentCell, maze.width)){
                int rightCell = currentCell + 1;

                if(!is_wall(maze.grid[rightCell]) && map[rightCell] == Unseen)
                    if (!enqueue_cell(&queue, map, rightCell, Left))
                        goto out_of_memory;
            }
            // Each saved direction points back to currentCell, one step closer to the finish.

        }

        if(!START_CELL_REACHED(map, maze.start)) {
            puts("Maze is impossible to clear");
            free(map);
            free(queue.q);
            free(maze.grid);
            return EXIT_FAILURE;
        }

        // Maze modifications
        modify_maze(&maze, map);

        // Print maze   
        print_maze(&maze);

        free(map);
        free(queue.q);
        free(maze.grid);
        return 0;

        // A name followed by a colon is a label. goto out_of_memory jumps here in this function.
        // All allocation failures share this cleanup. free(NULL) is safe if an allocation failed.
        out_of_memory:
            fputs("Not enough memory to search the maze.\n", stderr);
            free(map);
            free(queue.q);
            free(maze.grid);
            return EXIT_FAILURE;
    }
