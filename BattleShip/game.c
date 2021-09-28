//	Mohammad Faiz ATHER === UQ 2020
//
//	game.c
//	an implimentation of Game ADT using Grid
//
#include <stdio.h>
#include <ctype.h>
#include <assert.h>

#include "game.h"
#include "grid.h"
#include "thing.h"

typedef struct _game {
    Ship    player_total;
    Grid    player;
    Ships   player_ships;
    Ship    cpu_total;
    Grid    cpu;
    Ships   cpu_ships;
    int     cpu_turns;
} _game;

#define CHAR_OFFSET     64 //A to 1
#define INT_OFFSET      48 //1 to '1'


Game
game_make (void)
{
    Game g = (Game)malloc (sizeof(*g));
    assert (g != NULL);
    return g;
}

static int
game_init (Game g, int max_x, int max_y, Ship max_s, Queue ship_magnitudes, int cpu_turns)
{
    // create a player and cpu grid
    g->player   = grid_init (max_x, max_y);
    g->cpu      = grid_init (max_x, max_y);

    if (g->player == NULL || g->cpu == NULL)
    {
        free (g);
        return GAME_RULES_PROCESS_ERROR;
    }

    g->player_total = g->cpu_total  = max_s;
    g->cpu_turns = cpu_turns;
    g->player_ships = (Ships)malloc (sizeof(Ship) * max_s);
    assert (g->player_ships != NULL);
    g->cpu_ships    = (Ships)malloc (sizeof(Ship) * max_s);
    assert (g->cpu_ships != NULL);

    Ship ship_magnitude = 0;
    int check = 0;
    int check_ship_area = 0;
    for ( int i = 0; i < max_s; i++ )
    {   //get the mag and set it
        Thing t = queue_dequeue (ship_magnitudes);
        check = sscanf (t, "%d\n", &ship_magnitude);
        thing_destroy (t);
        if (check != 1 || ship_magnitude <= 0) { return GAME_DATA_QUEUE_ERROR; }

        g->player_ships[i]  = ship_magnitude;
        g->cpu_ships[i]     = ship_magnitude;
        check_ship_area    += ship_magnitude;
    }

    if (check_ship_area > (max_x * max_y) ) return GAME_SHIPS_MAX_ERROR;

    return 0;
}

static int
game_draw (Grid g, Ship total_ships, Ships magnitude_ships, Queue q)
{
    int check = 0;
    
    Thing t = NULL;
    
    int ship_size = 0;
    char x = 0;
    int y = 0;
    char direction = '\0';

    int r = 0;
    
    for (Ship s = 0; s < total_ships; s++)
    {
        ship_size = magnitude_ships[s];
        
        t = queue_dequeue (q);
        if (t == NULL) return GAME_QUEUE_ERROR;
        check = sscanf (t, "%c%d %c\n", &x, &y, &direction);
        if (thing_size(t) > 6) check = 0;
        thing_destroy (t);
        if (check != 3) return GAME_QUEUE_ERROR;

        if ( r = grid_draw (g, x-CHAR_OFFSET, y, s+INT_OFFSET+1, ship_size, direction), r != 0 )
            return GAME_ERRORS_OFFSET(r);
    }
    return 0;
}

// assumes q is sane
int
game_set (Game g, Data d)
{
    int max_x = 0, max_y = 0, total_ships = 0;
    
    // grid size
    Thing t = queue_dequeue (d[DATA_GAME]);
    int check = sscanf (t, "%d %d\n", &max_x, &max_y);
    check = sscanf (t, "%d %d %d\n", &max_x, &max_y, &check);
    thing_destroy (t);
    if (check != 2) return GAME_DATA_QUEUE_ERROR;
    
    t = queue_dequeue (d[DATA_GAME]);
    check = sscanf (t, "%d\n", &total_ships);
    if (total_ships != queue_size(d[DATA_GAME]) || total_ships <1 || total_ships > 15) check = -1;
    thing_destroy (t);
    if (check != 1) return GAME_DATA_QUEUE_ERROR;

    int e = game_init (g, max_x, max_y, total_ships, d[DATA_GAME], queue_size (d[DATA_CPU_MOVES]));
    if (e != 0) return e;

    int c = 0, cc = 0;
    c = game_draw ( g->player, g->player_total, g->player_ships, d[DATA_PLAYER_MAP]);
    cc = game_draw ( g->cpu, g->cpu_total, g->cpu_ships, d[DATA_CPU_MAP]);
    if ( c != 0 || cc != 0 )
    {
        game_destroy (g);
        // handle will be called here : TODO
       if (c != 0)
       {
           if (c == GAME_QUEUE_ERROR) return PROCESS_PLAYER_MAP_ERR;
           else if (c == GAME_BOUND_ERROR) return OUT_BOUND_PLAYER_MAP_ERR;
           else if (c == GAME_OVERLAP_ERROR) return OVERLAP_PLAYER_MAP_ERR;
       }
       else if (cc != 0)
       {
           if (cc == GAME_QUEUE_ERROR) return PROCESS_CPU_MAP_ERR;
           else if (cc == GAME_BOUND_ERROR) return OUT_BOUND_CPU_MAP_ERR;
           else if (cc == GAME_OVERLAP_ERROR) return OVERLAP_CPU_MAP_ERR;
       }
    }
    return 0;
}

static int
game_play_ (Grid g, Ship total, Ships s, Move m)
{
    if (m == NULL || !move_check (m)) return ILLEGAL_M;
    int hit = MISS;
    Element e = grid_get ( g, m[0]-CHAR_OFFSET, isdigit(m[2])?(m[2]-INT_OFFSET)+(m[1]-INT_OFFSET)*10:m[1]-INT_OFFSET );
    if (e == '\0') return BAD; //invalid move
    if  (e == '.' )
    {
        grid_set (g, m[0]-CHAR_OFFSET,isdigit(m[2])?(m[2]-INT_OFFSET)+(m[1]-INT_OFFSET)*10:m[1]-INT_OFFSET, '/');
    }
    else if ( e == '/' || e == '*')
    {
        hit = REPEAT;
    }
    else if (e >= '1' && e <= '9')
    {
        s[e-INT_OFFSET-1] -= 1;
        grid_set (g, m[0]-CHAR_OFFSET, isdigit(m[2])?(m[2]-INT_OFFSET)+(m[1]-INT_OFFSET)*10:m[1]-INT_OFFSET, '*');
        if (s[e-INT_OFFSET-1] == 0) { hit = DOWN; } //ship down
        else { hit = HIT; }
    }

    return hit;
}

int
game_play_cpu (Game g, Move m)
{
    if (g->player_total == 0) return WIN_CPU;
    if (g->cpu_turns == 0) return EXHAUST_MOVES_CPU;

    int res = game_play_(g->player, g->player_total, g->player_ships, m);
    if (DOWN == res)
    {
        g->player_total -= 1;
    }
    g->cpu_turns -= 1;
    return res;
}
/*
from the grid we can get the ship number and reverse it from the array subtracting from the ships magnitude
*/
int
game_play_player (Game g, Move m)
{
    if (g->cpu_total == 0) return WIN_PLAYER;
    int res = game_play_(g->cpu, g->cpu_total, g->cpu_ships, m);
    if (DOWN == res)
    {
        g->cpu_total -= 1;
    }
    return res;
}

int
game_status (Game g)
{
    if (g->cpu_total == 0) return WIN_PLAYER;
    else if (g->player_total == 0) return WIN_CPU;
    return GAME_ON;
}

void
game_show (Game g)
{
    assert (g != NULL);

    grid_show_discrete (g->cpu, "*/", 2);
    //grid_show (g->cpu);

    printf ("===\n");
    grid_show (g->player);
}

void
game_destroy (Game g)
{
    assert (g != NULL);
    grid_destroy (g->player);
    free (g->player_ships);
    grid_destroy (g->cpu);
    free (g->cpu_ships);
    free (g);
    return;
}
