#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include "handle.h"
#include "check.h"
#include "read.h"
#include "queue.h"
#include "data.h"
#include "thing.h"
#include "move.h"
#include "display.h"
#include "game.h"
#include "communication.h"

#define ARGUMENT_COUNT_HUB 2


void
naval_illegal_ (Game g, Data d, Error e);

void
naval_destroy_ (Game g, Data d);

int
main (int argc, char **argv)
{

    int ok_check = okay_args (argc, ARGUMENT_COUNT_HUB);
    if ( ok_check != SUCCESS)
        handle_exit (ok_check);

    //rules file read here
    if ( ok_check = okay_files (argv, ARGUMENT_COUNT_HUB), ok_check != SUCCESS)
        handle_exit (ok_check);

    Queue rules_q = simple_read (argv[1]);
    char *rules_s = queue_to_string (rules_q, ',');

    int max_x, max_y;
    // grid size
    Thing tt = rules_s;
    int check = sscanf (tt, "%d %d\n", &max_x, &max_y);
    check = sscanf (tt, "%d %d %d\n", &max_x, &max_y, &check);
    //thing_destroy (tt);
    if (check != 2) return GAME_DATA_QUEUE_ERROR;

    Queue config_q = simple_read(argv[2]);
    Thing t = queue_dequeue(config_q);
    Queue sep = queue_to_queue(t, ',');

    int fds_A1_from[2];
    int fds_A1_to[2];

    pipe(fds_A1_from);
    pipe(fds_A1_to);

    char *prog1 = queue_dequeue(sep);
    char *map1 = queue_dequeue(sep);
    char *prog2 = queue_dequeue(sep);
    char *map2 = queue_dequeue(sep);

    if (fork()==0)
    {//child1
        dup2(fds_A1_from[0], 0);//read a1
        dup2(fds_A1_to[1], 1);//write a1
        close(fds_A1_from[1]);//close read
        close(fds_A1_to[0]);//close write
        execl(prog1, prog1, "1", map1,"1", NULL);
    }

    int fds_A2_from[2];
    int fds_A2_to[2];

    pipe(fds_A2_from);
    pipe(fds_A2_to);

    if (fork()==0)
    {//child2
        dup2(fds_A2_from[0], 0);//read a1
        dup2(fds_A2_to[1], 1);//write a1
        close(fds_A2_from[1]);//close read
        execl(prog2, prog2, "2",map2,"2", NULL);
    }

    //parent
    close(fds_A1_from[0]);//close write
    close(fds_A1_to[1]);//close read
    close(fds_A2_from[0]);//close write
    close(fds_A2_to[1]);//close read
    char buffer[50];
    //FILE *w1 = fdopen(fds_A1_from[1], "w");
    FILE *r1 = fdopen(fds_A1_to[0], "r");

    //FILE *w2 = fdopen(fds_A2_from[1], "w");
    FILE *r2 = fdopen(fds_A2_to[0], "r");

    //while (fgets(buffer, 50, stdin)!=NULL) {

    // send rules get map
    thing_replace(rules_s, ' ', ',');
    rules_s = thing_add (thing_copy ("RULES "), rules_s, ' ');

    char map1_s[50] = {'\0'}; 
    write(fds_A1_from[1], rules_s, strlen(rules_s));
    write(fds_A1_from[1], "\n", 1);

    //read(fds_A1_to[0], map1_s, 50);
    fgets(map1_s, 50, r1);

    //printf("map1 %s\n", map1_s);

    char map2_s[50] = {'\0'}; 
    write(fds_A2_from[1], rules_s, strlen(rules_s));
    write(fds_A2_from[1], "\n", 2);

    //read(fds_A2_to[0], map2_s, 50);
    fgets(map2_s, 50, r2);


    //printf("map2 %s\n", map2_s);

    // make a game

    Data d = read_make_data (3);
    d[0] = read_string_rules (rules_s);
    d[1] = read_string_map (map1_s);
    d[2] = read_string_map (map2_s);

    Game g = game_make ();
    int check_set = game_set (g, d);
    if (check_set != 0)
    {
        read_destroy (d, 3);
        handle_exit (check_set==-13?50:check_set);
    }
    Move m1 = NULL, m2 = NULL;
    int c1 = 0, c2 = 0;
    int round = 0;
    Thing message;
    while ( game_status (g) == GAME_ON ) 
    {
        display_message (DISPLAY_STARS, "\n", "\0", stdout);
        display_message (DISPLAY_ROUND, "\0", "\0", stdout);
        printf("%d\n", round);
        fflush(stdout);
        game_show (g);
        c1=-1;
        while (c1 < 0) {
            write(fds_A1_from[1], YT_M, strlen(YT_M));
            write(fds_A1_from[1], "\n", 1);
            //fflush(f1);
            //read(fds_A1_to[0], buffer, 50);
            fgets(buffer, 50, r1);

            //printf("got from 2301A %s\n", buffer);
            m1 = thing_copy(buffer);
            m1 = m1+6;
            c1 = game_play_player (g, m1);
        }
        write(fds_A1_from[1], OK_M, strlen(OK_M));
        write(fds_A1_from[1], "\n", 2);
        display_result (c1, stdout);
        display_message (DISPLAY_PLAYER_MOVE, m1, "\0", stdout);
        //puts("");
        message = thing_add (thing_copy(display_selector(c1)), "1", ' ');
        message = thing_add (message, m1, ',');
        message = thing_add (message, "\0", '\0');

        write(fds_A1_from[1], message, strlen(message));
        write(fds_A1_from[1], "\n", 1);
        write(fds_A2_from[1], message, strlen(message));
        write(fds_A2_from[1], "\n", 1);

        if (game_status (g) != GAME_ON) {
            display_result ( game_status (g), stdout );

            naval_destroy_ (g, d);
            write(fds_A1_from[1], "DONE", 4);
            write(fds_A1_from[1], "\n", 1);
            write(fds_A2_from[1], "DONE", 4);
            write(fds_A2_from[1], "\n", 1);
            fclose(r1);
            fclose(r2);
            close(fds_A1_from[1]);
            close(fds_A1_to[0]);
            close(fds_A2_from[1]);
            close(fds_A2_to[0]);
            //printf("parent exiting...\n");
            
            //wait(NULL);
            exit(0);

    
        }

        //fflush(f2);

        //fflush(f1);
        c2=-1;
        while (c2<0) {
            write(fds_A2_from[1], YT_M, strlen(YT_M));
            write(fds_A2_from[1], "\n", 1);
            //read(fds_A2_to[0], buffer, 50);
            fgets(buffer, 50, r2);

            //printf("got from 2301B %s\n", buffer);
            fflush(stdout);
            m2 = thing_copy(buffer);
            m2 = m2+6;
            c2 = game_play_cpu (g, m2);
        }
        write(fds_A2_from[1], OK_M, strlen(OK_M));
        write(fds_A2_from[1], "\n",1);
        display_result (c2, stdout);
        display_message (DISPLAY_CPU_MOVE, m2, "\0", stdout);
        //puts("");
        message = thing_add (thing_copy(display_selector(c2)), "2", ' ');
        message = thing_add (message, m2, ',');
        message = thing_add (message, "\0", '\0');
        write(fds_A2_from[1], message, strlen(message));
        write(fds_A2_from[1], "\n", 1);
        write(fds_A1_from[1], message, strlen(message));
        write(fds_A1_from[1], "\n", 1);

        //fflush(f2);
        //display_message (DISPLAY_STARS, "\n", "\0");

        //if (m1==NULL || m2 ==NULL) break;
        //move_destroy (m1);
        //move_destroy (m2);

        //round++;
        //getc(stdin);
    }

    display_result ( game_status (g), stdout );

    naval_destroy_ (g, d);
    write(fds_A1_from[1], "DONE", 4);
    write(fds_A1_from[1], "\n", 1);
    write(fds_A2_from[1], "DONE", 4);
    write(fds_A2_from[1], "\n", 1);
    fclose(r1);
    fclose(r2);
    close(fds_A1_from[1]);
    close(fds_A1_to[0]);
    close(fds_A2_from[1]);
    close(fds_A2_to[0]);
    //printf("parent exiting...\n");
    
    //wait(NULL);
    exit(0);

    return 0;
}

void
naval_illegal_ (Game g, Data d, Error e)
{
    naval_destroy_ (g, d);
    handle_exit (e);
}

void
naval_destroy_ (Game g, Data d)
{
    // destroy backend
    game_destroy (g);

    // end
    read_destroy (d, ARGUMENT_COUNT_NAVAL);
}
