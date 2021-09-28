//	Mohammad Faiz ATHER === UQ 2020
//
//	display.h
//	an ADT for a Display
//

#include <stdio.h>
#include <string.h>

#ifndef DISPLAY_H_
#define DISPLAY_H_

#define DISPLAY_MISS        "Miss"
#define DISPLAY_HIT         "Hit"
#define DISPLAY_BAD_GUESS   "Bad guess"
#define DISPLAY_REPEAT_MOVE "Repeated guess"

#define DISPLAY_CPU_MOVE    "(CPU move)>"
#define DISPLAY_PLAYER_MOVE "(Your move)>"

#define DISPLAY_SHIP_SUNK   "Ship sunk\n"

#define DISPLAY_GAME_OVER   "Game over - "

#define DISPLAY_YOU_WIN     "you win"
#define DISPLAY_YOU_LOOSE   "you lose"

#define DISPLAY_CPU_GIVESUP "CPU gives up"
#define DISPLAY_ILLEGAL     "Bad guess"

#define UNKNOWN             "Unknown\n"


static inline void
display_message (char *m, char *addon, char *newline)
{
    printf ("%s%s%s", m, addon, newline);
    fflush (stdout);
}

static void
display_result (int r)
{
    char *show = NULL;
    switch (r)
    {
        case MISS: show = DISPLAY_MISS; break;
        case WIN_PLAYER: display_message (DISPLAY_GAME_OVER, DISPLAY_YOU_WIN, "\n"); return;
        case WIN_CPU: display_message (DISPLAY_GAME_OVER, DISPLAY_YOU_LOOSE, "\n"); return;
        case HIT: show = DISPLAY_HIT; break;
        case DOWN: display_message (DISPLAY_HIT, "\n", DISPLAY_SHIP_SUNK); return;
        case BAD: show = DISPLAY_BAD_GUESS; break;
        case REPEAT: show = DISPLAY_REPEAT_MOVE; break;
        case EXHAUST_MOVES_CPU: show = DISPLAY_CPU_GIVESUP; break;
        case ILLEGAL_M: show = DISPLAY_ILLEGAL; break;
        case GAME_ON: show = "\0"; break;
        default: show = UNKNOWN;
    }
    display_message (show, "\n", "\0");
}

#endif
