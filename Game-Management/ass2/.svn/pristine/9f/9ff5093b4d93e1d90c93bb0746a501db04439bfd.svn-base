#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

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

int mode_fight = 0;
int f_move_r;
int f_move_c;

char f_move_now[4] = {'\0'};

char f_directions[4] = {'N', 'E', 'S', 'W'};

int f_d_i = 0;

int f_height;
int f_width;

void
fighter_init(int height, int width, int seed) {
    f_height = height;
    f_width = width;
    srand(seed);
}



//0 miss, 1 hit, 2 sunk
void
fighter_guess(int result, int *move_i) {
    int row, column;
    if (f_d_i >= 4) {
            f_d_i = 0;
            mode_fight = 0;
    }
    if (result == 1) mode_fight = 1;
    else if (result == 2) mode_fight = 0;
    else if (result == 0) {
        if (mode_fight == 1 && f_d_i == 4) {
            f_d_i = 0;
            mode_fight = 0;
        }
    }
    if (mode_fight == 0) {//search
        row = rand() % f_height;
        column = rand() % f_width;
        f_move_c = column;
        f_move_r = row;
    } else {//attack
        row = f_move_r;
        column = f_move_c;
        if (result == 0) {
            f_d_i++;
        }
        switch (f_directions[f_d_i]) {
            case 'N':
                row = f_move_r -1;
                if (row < 0) {
                    f_d_i++;
                    row = 0;
                    column = f_move_c +1;
                }
                //f_move_r = row;
                break;
            case 'E':
                column = f_move_c +1;
                if(column >= f_width) {
                    row = f_move_r +1;
                    column = f_width-1;
                    f_d_i++;
                }
                //f_move_c = column;
                break;
            case 'S':
                row = f_move_r +1;
                if(row >= f_height) {
                    column = f_move_c-1;
                    row = f_height-1;
                    f_d_i++;
                }
                //f_move_r = row;
                break;
            case 'W':
                column = f_move_c-1;
                if (column < 0) {
                    row = rand() % f_height;
                    column = rand() % f_width;
                    f_d_i = 0;
                    mode_fight = 0;
                }
                //f_move_c = column;
                break;
        }
    }
    move_i[0] = column;
    move_i[1] = row;
}


void
agent_move (int *row, int *coln, int *move, int max_c);

void
naval_illegal (Game g, Data d, Error e);

void
naval_destroy (Game g, Data d);

void
parser_changer(char *s, int len, int which, char what);

char *
agent_move_extract (char *s);

char *
agent_id_extract (char *s);

char *
agent_id_extract (char *s) {
    for (int i = 0; i < strlen(s); i++) {
        if (s[i] == ',') {
            s = s + i - 1;
            return s;
        }
    }
    return 0;
}

char *
agent_move_extract (char *s)
{
    for (int i = 0; i < strlen(s); i++) {
        if (s[i] == ',') {
            s = s + i + 1;
            return s;
        }
    }
    return 0;
}

void
parser_changer(char *s, int len, int which, char what)
{
    int count = 1;
    for (int i = 0; i < len; i++)
    {
        if (!(isalpha(s[i]) || isdigit(s[i]))) {
            if (count == which) {
                s[i] = what;
                return;
            }
            count++;
        }
    }
}

void
agent_move (int *row, int *coln, int *move, int max_c)
{
    if(coln[0] > coln[1]) {
        *row = *row + 1;
        coln[0] = 0;
        coln[1] = max_c-1;
    }
    if (*row % 2 == 0) {
        //coln[0] //left pos unguessed
    //coln[1] //right pos unguessed
        if(coln[0] <= coln[1]) {
            move[1] = *row;
            move[0] = coln[0];
            coln[0]++;
        }
    } else {
        if(coln[1] >= coln[0]) {
            move[1] = *row;
            move[0] = coln[1];
            coln[1]--;
        }
    }
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
    read_destroy (d, 2);
}

char *
Fgets (Game g,Data d,  char *buff, int n, FILE *stream)
{
    char *r;
    if (r = fgets (buff, n, stream), r == NULL) {
        game_show_agent (g);
        fprintf(stderr, "Communications error\n");
        naval_destroy (g, d);
        exit(5);
    }
    return r;
}


int
agent1(Game g, Data d, int height, int width, int agent, int seed)
{
//Move m1 = NULL, m2 = NULL;
    int c1 = 0, c2 = 0;

    if (agent == 2) {
        fighter_init(height, width, seed);
    }

    char buff[50] = {'\0'};
    int buff_len;
    int guessed[2] = {0,7}, move_i[2] = {0,0};
    int roww = 0;
    char move[3] = {'\0'};
    char *mm;
    char *iid;
    bool done = false;
    char *msg;
    while ( game_status (g) == GAME_ON ) 
    {
        memset (buff, 0, 50);
        Fgets (g,d, buff, 50, stdin);
        buff_len = strlen(buff);
        parser_changer(buff,buff_len, 1, '\0');
        while (strcmp(buff, YT_M) == 0) 
        {
        // do move
            if (agent == 2) {
                fighter_guess(c1, move_i);
            } else {
                agent_move(&roww, guessed, move_i, width);
            }
            move[0] = move_i[0] + 65; //0 to 'A'
            move[1] = move_i[1] + 49; //0 to '1'
            //strcpy(buff, move);
            msg = thing_add(thing_copy(GUESS_M), move, ' ');
            write(1,msg , strlen(msg));
            write(1,"\n", 1);
            //fflush(stdout);
            memset (buff, 0, 50);
            Fgets (g,d, buff, 50, stdin);
            buff_len = strlen(buff);
            parser_changer(buff,buff_len, 1, '\0');
        }
        
        if (strcmp(buff, OK_M) == 0) {
            done = false;
            Fgets (g,d, buff, 50, stdin);
            buff_len = strlen(buff);
            parser_changer(buff,buff_len, 1, '\0');
            while (!done && (strcmp(buff, MISS_M) == 0||strcmp(buff, SUNK_M) == 0||strcmp(buff, HIT_M) == 0)){//hit, miss, sunk
                parser_changer(buff,buff_len, 1, ' ');
                parser_changer(buff,buff_len, 3, '\0');
                iid = agent_id_extract(buff);
                mm = agent_move_extract(buff);
                parser_changer(mm,strlen(mm), 1, '\0');

                if (iid[0] == '1') {
                    game_show_agent (g);
                    c1 = game_play_agent (g, buff, 1);
                    display_result (c1, stderr);
                    display_message (DISPLAY_PLAYER_MOVE, move, "\0", stderr);
                    fputs("\n", stderr);
                    Fgets (g,d, buff, 50, stdin);
                    buff_len = strlen(buff);
                    parser_changer(buff,buff_len, 1, '\0');
                } else {
                    c2 = game_play_agent (g, buff, 1);
                    display_result (c2, stderr);
                    display_message (DISPLAY_CPU_MOVE, mm, "\0", stderr);
                    fputs("\n", stderr);
                    done = true;
                }
            }

        }
        if (strcmp(buff, EARLY_M) == 0)
        {
        // do something else
            game_show_agent (g);
            naval_destroy (g, d);

            return 1;
        }
        /* more else if clauses */
        if (strcmp(buff, DONE_M) == 0)
        {
            display_result ( game_status (g), stderr );
            naval_destroy (g, d);
            return 0;
        }

    }
    display_result ( game_status (g), stderr );
    naval_destroy (g, d);
    return 0;
}

int
agent2(Game g, Data d, int height, int width, int agent, int seed)
{
    //Move m1 = NULL, m2 = NULL;
    int c1 = 0, c2 = 0;
    if (agent == 2) {
        fighter_init(height, width, seed);
    }

    char buff[50] = {'\0'};
    int buff_len;
    int guessed[2] = {0,7}, move_i[2] = {0,0};
    int roww = 0;
    char move[3] = {'\0'};
    char *mm;
    char *iid;
    char *msg;
    while ( game_status (g) == GAME_ON ) 
    {
        memset (buff, 0, 50);
        Fgets (g, d,buff, 50, stdin);
        buff_len = strlen(buff);
        parser_changer(buff,buff_len, 1, '\0');
        if (strcmp(buff, MISS_M) == 0||strcmp(buff, SUNK_M) == 0||strcmp(buff, HIT_M) == 0){//hit, miss, sunk
            parser_changer(buff,buff_len, 1, ' ');
            parser_changer(buff,buff_len, 3, '\0');
            iid = agent_id_extract(buff);
            mm = agent_move_extract(buff);

            if (iid[0] == '1') {
                game_show_agent (g);
                c1 = game_play_agent (g, buff, 2);
                display_result (c1, stderr);
                display_message (DISPLAY_PLAYER_MOVE, mm, "\0", stderr);
                fputs("\n", stderr);
                Fgets (g,d, buff, 50, stdin);
                buff_len = strlen(buff);
                parser_changer(buff,buff_len, 1, '\0');
            }
        }
        while (strcmp(buff, YT_M) == 0) 
        {
        // do move
            if (agent == 2) {
                fighter_guess(c1, move_i);
            } else {
                agent_move(&roww, guessed, move_i, width);
            }
            move[0] = move_i[0] + 65; //0 to 'A'
            move[1] = move_i[1] + 49; //0 to '1'
            //strcpy(buff, move);
            msg = thing_add(thing_copy(GUESS_M), move, ' ');
            write(1,msg , strlen(msg));
            write(1,"\n", 1);
            //fflush(stdout);
            memset (buff, 0, 50);
            Fgets (g,d, buff, 50, stdin);
            buff_len = strlen(buff);
            parser_changer(buff,buff_len, 1, '\0');
        }
        
        if (strcmp(buff, OK_M) == 0) {
            memset (buff, 0, 50);
            Fgets (g,d, buff, 50, stdin);
            buff_len = strlen(buff);
            parser_changer(buff,buff_len, 1, '\0');
            if (strcmp(buff, MISS_M) == 0||strcmp(buff, SUNK_M) == 0||strcmp(buff, HIT_M) == 0){//hit, miss, sunk
                c2 = game_play_agent (g, buff, 2);
                display_result (c2, stderr);
                display_message (DISPLAY_CPU_MOVE, move, "\0", stderr);
                fputs("\n", stderr);
            }

        }
        if (strcmp(buff, EARLY_M) == 0)
        {
        // do something else
            game_show_agent (g);
            naval_destroy (g, d);

            return 1;
        }
        /* more else if clauses */
        if (strcmp(buff, DONE_M) == 0)
        {
            display_result ( game_status (g), stderr );
            naval_destroy (g, d);
            return 0;
        }

        //write(2, buff, strlen(buff));


        //display_message (DISPLAY_STARS, "\n", "\0");

        //if (m1==NULL || m2 ==NULL) break;
        //move_destroy (m1);
        //move_destroy (m2);

        //round++;
    }

    display_result ( game_status (g), stderr );

    naval_destroy (g, d);
    return 0;
}

