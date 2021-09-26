//	Mohammad Faiz ATHER === UQ 2020
//
//	move.c
#include <stdio.h>
#include <string.h>

#include "move.h"

#define INT_OFFSET      48 //1 to '1'

#define MAX_LINE_SIZE   2048
#define STR_(X)         #X
#define STR(X)          STR_(X)

bool
move_check_eof (Move m)
{
    //if (m[0] == '\n') return true;
    if (m == NULL) return true;
    return false;
}

bool
move_check (Move m)
{
    if (m == NULL) return false;
    if ( m[0] == '\0') return false;
    char x = '\0';
    int y = 0;
    int c = 0;
    if (m[1] == ' ') {
        m[1] = '\0';
        strcat (m, m+2);
    }
    c = sscanf (m, "%c%d\n", &x, &y);
    if (strlen(m) > 4) c = 0;
    return !(c != 2 || x < 'A' || x > 'Z' || y < 1 || y > 26);
}

Move
move_player (void)
{
    char m[MAX_LINE_SIZE] = {'\0'};
    char *m_th = NULL;
    if (NULL == fgets(m, MAX_LINE_SIZE, stdin)) {
        return NULL;
    }
    m_th = thing_copy(m);
    return m_th;
}

Move
move_cpu (Queue q)
{
    return queue_dequeue (q);
}

void
move_destroy (Move m)
{
    if (m != NULL) thing_destroy (m);
}
