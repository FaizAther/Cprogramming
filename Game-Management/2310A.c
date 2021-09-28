#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

#include "communication.h"
#include "handle.h"
#include "check.h"
#include "read.h"
#include "queue.h"
#include "thing.h"
#include "move.h"
#include "display.h"
#include "game.h"
#include "agent.h"


int
main (int argc, char **argv)
{

    int ok_check = okay_args (argc, ARGUMENT_COUNT_AGENT);
    if ( ok_check != SUCCESS)
        handle_exit (ok_check);

    //rules file read here
    if ( ok_check = okay_files (argv+1, 1), ok_check != SUCCESS)
        handle_exit (ok_check);

    Queue map_q = simple_read (argv[2]);
    char *map_s = queue_to_string (map_q, ':');
    thing_replace (map_s, ' ', ',');
    map_s = thing_add (thing_copy (MAP_M), map_s, ' ');

    char *mmap = thing_copy(map_s);
    char rules_s[50];
    fgets (rules_s, 50, stdin);
    //rules[r-1] = '\0';//stripping newline char
    int height, width;
    sscanf(rules_s, "RULES %d,%d", &height, &width);    // validate map
    
    //fflush(stdout);

    //make a game and wait for talking

    Data d = read_make_data (3);
    d[0] = read_string_rules (rules_s);
    d[1] = read_string_map (map_s);
    d[2] = NULL;

    //int row = 0, column = 0;

    Game g = game_make ();
    int check_set = game_set (g, d);
    if (check_set != 0)
    {
        read_destroy (d, 3);
        if(PROCESS_PLAYER_MAP_ERR) {
            fprintf(stderr, AGENT_MAP_ERR);
            exit(3);
        }
        handle_exit (check_set==-13?50:check_set);
    }

    
        //write(2, buff, strlen(buff));


        //display_message (DISPLAY_STARS, "\n", "\0");

        //if (m1==NULL || m2 ==NULL) break;
        //move_destroy (m1);
        //move_destroy (m2);

        //round++;
    //send map
    write(1, mmap, strlen(mmap));
    write(1,"\n",1);

    if(argv[1][0] == '1')
        agent1(g, d, height, width, 1, argv[3][0] - 48);
    else
        agent2(g, d, height, width, 1, argv[3][0] - 48);
    

    return 0;
}

