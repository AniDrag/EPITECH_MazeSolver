#ifndef UTILITY_H
#define UTILITY_H

#include <stdbool.h>
#include <stdlib.h>

/// <summary>Check whether a cell blocks movement.</summary>
static bool is_wall(char cell)
{
    return cell != '*';
}

// Region MACROS

#define CAN_GO_UP(currentCell, mazeWidth) ((currentCell) >= (mazeWidth))

#define CAN_GO_LEFT(currentCell, mazeWidth) (((currentCell) % (mazeWidth)) > 0)

#define CAN_GO_RIGHT(currentCell, mazeWidth) (((currentCell) % (mazeWidth)) < (mazeWidth) - 1)

#define CAN_GO_DOWN(currentCell, mazeWidth, mazeHeight) (((currentCell) / (mazeWidth)) < (mazeHeight) - 1)

#define GO_DOWN(cellIndex, mazeWidth) (cellIndex += mazeWidth)

#define START_CELL_REACHED(map, mazeStart) ((map)[(mazeStart)] != Unseen)
// endregion

//is idx 0, 1,2...
enum Direction{ Unseen, End, Up, Down, Left,Right};

/// <summary>Turn a saved direction into a step in the flat grid.</summary>
/// <returns>Signed offset to add to the cell index, or zero for a non-movement value.</returns>
static int next_cell(enum Direction direction, int mazeWidth)
{
    switch (direction) {
        case Up:    return -mazeWidth;
        case Down:  return  mazeWidth;
        case Left:  return -1;
        case Right: return  1;
        default:    return  0; // Error
    }
}
/// <summary>Choose the ANSI foreground color for a cell.</summary>
/// <returns>Color number to insert into the terminal escape sequence.</returns>
static int color_for_cell(char cell)
{
    switch (cell) {
        case 'o': return 32; // Green
        case '*': return 33; // dark yellow/brown
        case 'X': return 31; // Dark red
        default:  return 35; // Purple
    }
}



/// <summary>A flat grid with its dimensions and the indices of its two endpoints.</summary>
typedef struct {
    char *grid;
    int width;
    int height;
    int start;
    int finish;
} Maze;

// QUEUE ------------------------------------------------------------

/// <summary>A circular queue of cell indices. head is the next slot to read, count is the number waiting, and capacity is the number of allocated slots.</summary>
typedef struct {
    int *q;
    size_t head;
    size_t count;
    size_t capacity;
} Queue;

/// <summary>Make room for more cells while keeping the oldest cell first.</summary>
/// <param name="q">Queue to resize.</param>
/// <returns>True on success. On allocation failure the old queue is still usable.</returns>
static inline bool grow_queue(Queue *q)
{
    size_t capacity = q->capacity ? q->capacity * 2 : 16;//Fale safe.
    int *cells = malloc(capacity * sizeof *cells);

    if (cells == NULL)
        return false;

    // Copy in FIFO order, including elements that wrapped to the front.
    // % wraps around. With 16 slots, index 16 becomes slot 0 again.
    for (size_t i = 0; i < q->count; i++)
        cells[i] = q->q[(q->head + i) % q->capacity];
    free(q->q);
    q->q = cells;
    q->head = 0;
    q->capacity = capacity;
    return true;
}

/// <summary>Add a cell to the back and mark it seen once it has a slot.</summary>
/// <param name="q">Queue of cells still to visit.</param>
/// <param name="map">Direction map for the search.</param>
/// <param name="cell">Index to enqueue.</param>
/// <param name="dir">Step toward the finish, or End for the finish itself.</param>
/// <returns>False if the queue needed more memory and could not get it.</returns>
static inline bool enqueue_cell(Queue *q, unsigned char *map, int cell, enum Direction dir)
{
    if (q->count == q->capacity && !grow_queue(q))
        return false;
        
    q->q[(q->head + q->count) % q->capacity] = cell;
    q->count++;
    map[cell] = (unsigned char)dir;
    return true;
}

/// <summary>Take the oldest cell from a nonempty queue.</summary>
/// <param name="q">Queue with at least one cell waiting.</param>
/// <returns>The removed cell index.</returns>
static inline int dequeue_cell(Queue *q)
{
    int cell = q->q[q->head];

    // Move the read position. The old slot can be reused without shifting the other cells.
    q->head = (q->head + 1) % q->capacity;
    q->count--;
    return cell;
}

#endif
