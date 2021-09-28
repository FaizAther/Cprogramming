//	Mohammad Faiz ATHER === UQ 2020
//
//	naval.c
#include <stdio.h>
#include <stdbool.h>

#include "handle.h"
#include "check.h"
#include "read.h"
#include "move.h"
#include "display.h"
#include "game.h"

void
naval_illegal (Game g, Data d, Error e);

void
naval_destroy (Game g, Data d);

int
main (int argc, char **argv)
{
    // count arguments
    int ok_check = okay_args (argc, ARGUMENT_COUNT_NAVAL);
    if ( ok_check != SUCCESS)
        handle_exit (ok_check);

    // error checking
    if ( ok_check = okay_files (argv, ARGUMENT_COUNT_NAVAL), ok_check != SUCCESS)
        handle_exit (ok_check);

    // make the backend from files
    Data d = read_files (argv, ARGUMENT_COUNT_NAVAL);

    Game g = game_make ();
    int check_set = game_set (g, d);
    if (check_set != 0)
    {
        read_destroy (d, ARGUMENT_COUNT_NAVAL);
        handle_exit (check_set==-13?50:check_set);
    }

    Move m1 = NULL, m2 = NULL;
    int c1 = 0, c2 = 0;
    //int round = 0;
    while ( game_status (g) == GAME_ON ) 
    {
        m1 = move_player ();
        
        m2 = move_player ();

        //display_message (DISPLAY_STARS, "\n", "\0");
        //display_message(DISPLAY_ROUND, "\0", "\0");
        //printf("%d\n", round);
        //fflush(stdout);

        game_show (g);

        c1 = game_play_player (g, m1);
        c2 = game_play_cpu (g, m2);

        display_result (c1);
        display_message (DISPLAY_PLAYER_MOVE, m1, "\0");

        display_result (c2);
        display_message (DISPLAY_CPU_MOVE, m2, "\0");

        //display_message (DISPLAY_STARS, "\n", "\0");

        if (m1==NULL || m2 ==NULL) break;
        move_destroy (m1);
        move_destroy (m2);

        //round++;
    }

    display_result ( game_status (g) );

    naval_destroy (g, d);

    return 0;
}

void
naval_illegal (Game g, Data d, Error e)
{
    naval_destroy (g, d);
    handle_exit (e);
}

void
naval_destroy (Game g, Data d)
{
    // destroy backend
    game_destroy (g);

    // end
    read_destroy (d, ARGUMENT_COUNT_NAVAL);
}
