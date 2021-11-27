//	Mohammad Faiz ATHER === UQ 2020
//
//	game.h
//	an ADT for a Game
//
#include <stdbool.h>

#include "queue.h"
#include "move.h"
#include "data.h"
#include "handle.h"

#ifndef GAME_H_
#define GAME_H_

#define GAME_MAX_X      26
#define GAME_MAX_Y      26


#define GAME_BOUND_ERROR        -11
#define GAME_OVERLAP_ERROR      -12
#define GAME_DATA_QUEUE_ERROR   -13
#define GAME_SHIPS_MAX_ERROR    -14
#define GAME_PLAYER_CPU_ERROR   -15
#define GAME_QUEUE_ERROR        -16
#define GAME_PLAYER_QUEUE_ERROR -17
#define GAME_CPU_QUEUE_ERROR    -18
#define GAME_DIRECTION_ERROR    -19

#define GAME_RULES_PROCESS_ERROR 50


typedef struct _game    *Game;

typedef int             Ship;
typedef Ship           *Ships;

static inline int
GAME_ERRORS_OFFSET(int E)
{
    return (E-10);
}

Game
game_make (void);

void
game_show (Game);

void
game_show_agent (Game);

int
game_set (Game, Data);

int
game_play_cpu (Game, Move);

int
game_play_player (Game, Move);

int
game_play_agent (Game, Move, int);

int
game_status (Game g);

void
game_destroy (Game);

#endif
