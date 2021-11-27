#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define GRID_SIZE 9

#define DIVIDE_GRID(X) 3

typedef int     elem;
typedef elem    *line;
typedef line    *sudoku;


typedef struct _grid *grid;

void *
routine (void *);

grid
make_grid ();

sudoku
make_board ();

void
show_sudoku (sudoku);

bool
check_grid (grid);

void
free_sudoku (sudoku);

void
grid_set_col (grid, int);

void
grid_set_row (grid, int);

bool
grid_get_res (grid);