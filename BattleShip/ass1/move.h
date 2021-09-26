//	Mohammad Faiz ATHER === UQ 2020
//
//	move.h
#include "thing.h"
#include "queue.h"

#ifndef MOVE_H_
#define MOVE_H_

#define MISS    0
#define HIT     1
#define DOWN    2

#define WIN_PLAYER  3
#define WIN_CPU     4
#define GAME_ON     5

#define REPEAT              -1        
#define BAD                 -2
#define EXHAUST_MOVES_CPU   -3

#define ILLEGAL_M           -4

typedef Thing Move;

bool
move_check_eof (Move m);

/*
    @return NULL if invalid move
*/
bool
move_check (Move m);

void
move_destroy (Move);

Move
move_player (void);

Move
move_cpu (Queue);

#endif