#ifndef UTILITY_H
#define UTILITY_H

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

#define START_CELL_REACHED(map, mazeStart) ((map)[(mazeStart)] != Unseen)
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
static int color_for_cell(char cell)
{
    switch (cell) {
        case 'o': return 32; // Green
        case '*': return 33; // dark yellow/brown
        case 'X': return 31; // Dark red
        default:  return 35; // Purple
    }
}

typedef struct {
    int *q;
    int head;
    int tail;
} Queue;

typedef struct {
    char *grid;
    int width;
    int height;
    int start;
    int finish;
} Maze;


#endif