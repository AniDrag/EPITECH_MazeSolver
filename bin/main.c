#include "maze_parser.h"
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
static bool is_wall(char cell)
{
    return cell != '*';
}
// Region MACROS

#define CAN_GO_UP(currentCell, mazeWidth) \
    ((currentCell) >= (mazeWidth))

#define CAN_GO_LEFT(currentCell, mazeWidth) \
    (((currentCell) % (mazeWidth)) > 0)

#define CAN_GO_RIGHT(currentCell, mazeWidth) \
    (((currentCell) % (mazeWidth)) < (mazeWidth) - 1)

#define CAN_GO_DOWN(currentCell, mazeWidth, mazeHeight) \
    (((currentCell) / (mazeWidth)) < (mazeHeight) - 1)

#define GO_DOWN(cellIndex, mazeWidth)(cellIndex += mazeWidth)

#define ENQUEUE_CELL(cell, tail, direction) do { \
    int cell_index = (cell);                     \
    dictionary[cell_index] = (direction);        \
    queue[(tail)++] = cell_index;                \
} while (0)

// endregion

enum Direction{ Unseen, End, Up, Down, Left,Right};

static int next_cell(enum Direction direction, int mazeWidth)
{
    switch (direction) {
        case Up:    return -mazeWidth;
        case Down:  return  mazeWidth;
        case Left:  return -1;
        case Right: return  1;
        default:    return  0;  // End or Unseen ill handle it as error
    }
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

    int mazeWidth;
    int mazeHeight;
    char *maze = load_maze(file, &mazeWidth, &mazeHeight);
    fclose(file);

    if (maze == NULL) 
    {
        return EXIT_FAILURE;
    }
   

    const int mazeStart = 0;
    const int mazeFinish = mazeWidth * mazeHeight - 1;

    if (is_wall(maze[mazeFinish])) {
        puts("no solution found! End is a wall");
        free(maze);
        return 0;
    }
    if (is_wall(maze[mazeStart])) {
        puts("no solution found! Start is A wall");
        free(maze);
        return 0;
    }

    size_t capacity = (size_t)mazeWidth * mazeHeight;
    int *queue = malloc(capacity * sizeof *queue);
    if (queue == NULL) return EXIT_FAILURE;

    int head = 0;  // Where to read the next cell.
    int tail = 0;  // Where to write the next cell.

    // calloc zeros out my dic ;)
    unsigned char *dictionary = calloc(capacity, sizeof *dictionary);
    if (dictionary == NULL) {
        free(queue);
        free(maze);
        return EXIT_FAILURE;
    }
    
    dictionary[mazeFinish] = End;
   //queue[tail++] = finish; slightly less clear what happens so im opting for second option.
   queue[tail] = mazeFinish;
   tail++;

    while (head < tail){
        int currentCell = queue[head++];
        //printf("Cell [%d]\n", currentCell);

        if (currentCell == mazeStart) break;
    

        if(CAN_GO_UP(currentCell,mazeWidth)){
            int aboveCell = currentCell - mazeWidth;

            if(!is_wall(maze[aboveCell]) && dictionary[aboveCell] == Unseen){
                ENQUEUE_CELL(aboveCell,tail,Down);
            }
        }
        if(CAN_GO_LEFT(currentCell,mazeWidth)){
            int leftCell = currentCell - 1;

            if(!is_wall(maze[leftCell]) && dictionary[leftCell] == Unseen){
                ENQUEUE_CELL(leftCell,tail,Right);
            }
        }
        if(CAN_GO_DOWN(currentCell,mazeWidth,mazeHeight)){
            int downCell = currentCell + mazeWidth;

            if(!is_wall(maze[downCell]) && dictionary[downCell] == Unseen)
                ENQUEUE_CELL(downCell,tail,Up);
        }
        if(CAN_GO_RIGHT(currentCell, mazeWidth)){
            int rightCell = currentCell + 1;

            if(!is_wall(maze[rightCell]) && dictionary[rightCell] == Unseen)
            ENQUEUE_CELL(rightCell,tail,Left);
        }
        // Ok We try firs Up left Right down
        // Macro for can go up should be if curr % height-1 != 0
        // for down should be if curr % height -1 >= height
        // left right same logic

    }

    if(dictionary[mazeStart] == Unseen) {
        puts("Maze is impossible to clear");
        free(dictionary);
        free(queue);
        free(maze);
        return EXIT_FAILURE;
    }



    // Maze modifications

   int cellIndex = mazeStart;

    while (cellIndex != mazeFinish) 
    {
        int offset = next_cell(dictionary[cellIndex], mazeWidth);

        if (offset == 0) 
        {
            fprintf(stderr, "Invalid direction at cell %d\n", cellIndex);
            free(dictionary);
            free(queue);
            free(maze);
            return EXIT_FAILURE;
        }

        maze[cellIndex] = 'o';
        cellIndex += offset;
    }

    maze[mazeFinish] = 'o';


    // Print maze   
    for(int i = 0; i < mazeWidth*mazeHeight; i++){
        if(i%mazeWidth == 0) 
            printf("\n Line >>%c", maze[i]);
        else 
            printf("%c", maze[i]);

    }

    free(dictionary);
    free(queue);
    free(maze);
    return 0;
}