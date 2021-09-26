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
    int ok_check = okay_args (argc);
    if ( ok_check != SUCCESS)
        handle_exit (ok_check);

    // error checking
    if ( ok_check = okay_files (argv), ok_check != SUCCESS)
        handle_exit (ok_check);

    // make the backend from files
    Data d = read_files (argv);

    Game g = game_make ();
    int check_set = game_set (g, d);
    if (check_set != 0)
    {
        read_destroy (d);
        handle_exit (check_set==-13?50:check_set);
    }

    Move m = NULL;
    int c = 0;
    while ( game_status (g) == GAME_ON ) 
    {
        game_show (g);

        display_message (DISPLAY_PLAYER_MOVE, "\0", "\0");
        
        // take input
        m = move_player ();
        // play
        c = game_play_player (g, m);
        if (c == ILLEGAL_M && move_check_eof(m)) 
        {
            //printf ("move check eof -> %d\n", move_check_eof(m));
            //if (move_check_eof(m) == false) 
            //{
            //    display_message (DISPLAY_ILLEGAL, "\n", "\0");
            //    display_message (DISPLAY_PLAYER_MOVE, "\0", "\0");
            //}
            move_destroy (m);
            naval_illegal (g, d, PLAYER_QUIT_ERR);
        }
        move_destroy (m);
        while (c ==ILLEGAL_M || c == BAD || c == REPEAT) 
        {
            display_result (c);
            display_message (DISPLAY_PLAYER_MOVE, "\0", "\0");
            m = move_player ();
            c = game_play_player (g, m);
            if (c == ILLEGAL_M && move_check_eof(m)) 
            {
                //if (move_check_eof(m) == false) 
                //{
                    //display_message (DISPLAY_ILLEGAL, "\n", "\0");
                    //display_message (DISPLAY_PLAYER_MOVE, "\0", "\0");
                //}
                move_destroy (m);
                naval_illegal (g, d, PLAYER_QUIT_ERR);
            }
            move_destroy (m);
        }
        display_result (c);
        if (c < BAD || game_status (g) == WIN_PLAYER) break;

        m = move_cpu (d[DATA_CPU_MOVES]);
        display_message (DISPLAY_CPU_MOVE, "\0", "\0");
        c = game_play_cpu (g, m);
        if (c == EXHAUST_MOVES_CPU) naval_illegal (g, d, CPU_QUIT_ERR);
        if (m != NULL) display_message (m, "\n", "\0");
        move_destroy (m);
        if (c != EXHAUST_MOVES_CPU) display_result (c);
        while (c == BAD || c == REPEAT  || c == ILLEGAL_M)
        {
            m = move_cpu (d[DATA_CPU_MOVES]);
            c = game_play_cpu (g, m);
            display_message (DISPLAY_CPU_MOVE, "\0", "\0");
            if (c == EXHAUST_MOVES_CPU) naval_illegal (g, d, CPU_QUIT_ERR);
            if (m != NULL) display_message (m, "\n", "\0");
            move_destroy (m);
            if (c != EXHAUST_MOVES_CPU ) display_result (c);
        }
        if (c < BAD) break;
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
    read_destroy (d);
}
