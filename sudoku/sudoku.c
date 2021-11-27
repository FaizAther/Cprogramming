#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "sudoku.h"

typedef struct _grid {
    int     row;
    int     column;
    sudoku  board;
    bool    res;
} _grid;

void
grid_set_col (grid data, int col)
{
    data->column = col;
}

void
grid_set_row (grid data, int row)
{
    data->row = row;
}

grid
make_grid (sudoku board)
{
    grid data = (grid)malloc (sizeof(struct _grid));
    data->row = GRID_SIZE;
    data->column = GRID_SIZE;
    data->board = board;
    return data;
}

sudoku
make_board (void)
{
    sudoku board = (sudoku)malloc(sizeof(line) * GRID_SIZE);
    for (int i = 0; i < GRID_SIZE; i++)
    {
        board[i] = malloc (sizeof (int) * GRID_SIZE);
    }
    return board;
}

bool
check_elem (elem val)
{
    return val >= 1 && val <= 9;
}

bool
_check (grid data, bool row_on, int row_col)
{
    int unique;
    line total = (line)malloc (sizeof (elem) * GRID_SIZE);
    memset (total, 0, sizeof (elem) * GRID_SIZE); 
    for (int i = 0; i < GRID_SIZE; i++)
    {
        elem val = row_on ? data->board[row_col][i] : data->board[i][row_col];
        
        if (check_elem (val) && total[val-1] == 0) {
            total[val-1] = val;
        } else {
            free (total);
            return false;
        }
    }
    free (total);
    return true;
}

bool
check_row (grid data)
{
    return data->row == -1 || _check (data, true, data->row);
}


bool
check_column (grid data)
{
    return data->column == -1 || _check (data, false, data->column);
}

bool
check_grid (grid data)
{
    bool b = true;
    int i;
    for ( i = 0; i < GRID_SIZE && b; i++ )
    {
        data->row = i;
        data->column = i;
        b = check_column (data) && check_row (data);
    }
    data->row = GRID_SIZE;
    data->column = GRID_SIZE;
    return b && i == data->row;
}


void
show_sudoku (sudoku board)
{
    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            printf ("%d ", board[i][j]);
        }
        printf ("\n");
    }
}

void
free_sudoku (sudoku board)
{
    free (board);
}

void *
routine (void *inp)
{
    grid data = (grid)inp;
    data->res = check_column (data) && check_row (data);
    //printf ("%d, row: %d, col: %d\n", data->res, data->row, data->column);
    pthread_exit ((void *)(&(data->res)));
}

bool
grid_get_res (grid data)
{
    return data->res;
}