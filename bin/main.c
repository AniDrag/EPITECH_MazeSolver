#include "maze_parser.h"
#include "utility.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>


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



void print_maze(Maze *maze)
{
    for (int row = 0; row < maze->height; row++) {
        const char *row_start = maze->grid + (size_t)row * maze->width;
        int column = 0;

        // Group adjacent cells with the same color so each run can be written at once.
        while (column < maze->width) {
            int color = color_for_cell(row_start[column]);
            int run_start = column++;

            while (column < maze->width && color_for_cell(row_start[column]) == color)
                column++;

            // ANSI escape sequence selects the color; fwrite outputs this run of cells.
            fprintf(stdout, "\x1b[%dm", color);
            fwrite(row_start + run_start, sizeof *maze->grid,
                (size_t)(column - run_start), stdout);
        }

        // Reset the terminal color, then end this maze row.
        fputs("\x1b[0m", stdout);
        // Finish the printed row with a newline.
        fputc('\n', stdout);
    }
}
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


int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <maze file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *file = fopen(argv[1], "rb");
    if (file == NULL) 
    {
        perror(argv[1]);
        return EXIT_FAILURE;
    }

    //int mazeWidth;
    //int mazeHeight;
    //char *maze = load_maze(file, &mazeWidth, &mazeHeight);
    Maze maze = {0};
    maze.grid = load_maze(file, &maze.width, &maze.height);
    fclose(file);

    if (maze.grid == NULL) 
    {
        return EXIT_FAILURE;
    }
   

    //const int mazeStart = 0;
    //const int mazeFinish = mazeWidth * mazeHeight - 1;
    maze.start = 0;
    maze.finish = maze.width * maze.height - 1;

    validate_maze(&maze);

    size_t capacity = (size_t)maze.width * maze.height;

    //int *queue = malloc(capacity * sizeof *queue);
    Queue queue = {0};    
    queue.q = malloc(capacity * sizeof *queue.q);
    if (queue.q == NULL) {
        free(maze.grid);
        return EXIT_FAILURE;
    }

    queue.head = 0;  
    queue.tail = 0; 

    // calloc zeros out my dic ;)
    unsigned char *map = calloc(capacity, sizeof *map);
    if (map == NULL) {
        free(queue.q);
        free(maze.grid);
        return EXIT_FAILURE;
    }
    
    map[maze.finish] = End;
   //queue[tail++] = finish; slightly less clear what happens so im opting for second option.
   queue.q[queue.tail] = maze.finish;
   queue.tail++;

    while (queue.head < queue.tail){
        int currentCell = queue.q[queue.head++];
        //printf("Cell [%d]\n", currentCell);

        if (currentCell == maze.start) break;
    

        if(CAN_GO_UP(currentCell,maze.width)){
            int aboveCell = currentCell - maze.width;

            if(!is_wall(maze.grid[aboveCell]) && map[aboveCell] == Unseen){
                ENQUEUE_CELL(aboveCell,queue.tail,Down);
            }
        }
        if(CAN_GO_LEFT(currentCell,maze.width)){
            int leftCell = currentCell - 1;

            if(!is_wall(maze.grid[leftCell]) && map[leftCell] == Unseen){
                ENQUEUE_CELL(leftCell,queue.tail,Right);
            }
        }
        if(CAN_GO_DOWN(currentCell,maze.width,maze.height)){
            int downCell = currentCell + maze.width;

            if(!is_wall(maze.grid[downCell]) && map[downCell] == Unseen)
                ENQUEUE_CELL(downCell,queue.tail,Up);
        }
        if(CAN_GO_RIGHT(currentCell, maze.width)){
            int rightCell = currentCell + 1;

            if(!is_wall(maze.grid[rightCell]) && map[rightCell] == Unseen)
            ENQUEUE_CELL(rightCell,queue.tail,Left);
        }
        // Ok We try firs Up left Right down
        // Macro for can go up should be if curr % height-1 != 0
        // for down should be if curr % height -1 >= height
        // left right same logic

    }

    if(!START_CELL_REACHED(map, maze->start)) {
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
}