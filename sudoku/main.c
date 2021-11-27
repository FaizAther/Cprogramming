#include <stdio.h>
#include <pthread.h>
#include <assert.h>

#include "sudoku.h"
#include "solve.h"

int
main (void)
{
    sudoku board = make_board ();
    //printf ("sudoku %p, %p, %d\n", board, board[0], board[0][0]);
    solve (board);
    pthread_t ts[GRID_SIZE*GRID_SIZE];
    grid data[2 * GRID_SIZE];
    int k = 0;
    for (int i = 0; i < GRID_SIZE; i++) {
        data[k] = make_grid (board);
        grid_set_row (data[k], i);
        grid_set_col (data[k], -1);
        pthread_create (&ts[k], NULL, &routine, data[k]);
        k++;
        data[k] = make_grid (board);
        grid_set_row (data[k], -1);
        grid_set_col (data[k], i);
        pthread_create (&ts[k], NULL, &routine, data[k]);
        //printf ("k: %d\ti: %d, j: %d\n", k, i);
        //pthread_create (&ts[k], NULL, &routine, &(_grid){i, j});    grid data = make_grid (board);
        k++;
    }
    k = 0;
    bool fine = true;
    for (int i = 0; i < GRID_SIZE; i++) {
        pthread_join (ts[k], NULL);
        if (grid_get_res (data[k]) == false)
        {
            fine = false;
        }
        k++;
        pthread_join (ts[k], NULL);
        if (grid_get_res (data[k]) == false)
        {
            fine = false;
        }
        k++;
    }
    printf ("fine is: %d\n", fine);
    //assert (fine == check_grid (make_grid (board)));
    free_sudoku (board);
    return 0;
}
