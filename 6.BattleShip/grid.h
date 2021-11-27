//	Mohammad Faiz ATHER === UQ 2020
//
//	grid.h
//	an ADT for a Grid
//
#include <stdbool.h>

#ifndef GRID_H_
#define GRID_H_

#define GRID_MAX_X   26 //cannot be increased
#define GRID_MAX_Y   26 //can be increased

#define GRID_BOUND_ERROR        '\0'
#define DRAW_BOUND_ERROR        -1
#define DRAW_BOUND_EXCEED_ERROR -6
#define DRAW_OVERLAP_ERROR      -2
#define DRAW_DIRECTION_INVALID  -6

#define DEFAULT_ELEMENT     '.'

#define DIRECTION_NORTH     'N'
#define DIRECTION_SOUTH     'S'
#define DIRECTION_EAST      'E'
#define DIRECTION_WEST      'W'

typedef char    Direction;
typedef char    Element;
typedef int     Position;
typedef int     Magnitude;

typedef struct _grid *Grid;


Grid
grid_init (Position, Position);

void
grid_clear (Grid);

int
grid_set (Grid, Position, Position, Element);

Element
grid_get (Grid, Position, Position);

void
grid_show (Grid g);

void
grid_show_discrete (Grid g, char *show, int n);

void
grid_destroy (Grid);

/*
true if drew false otherwise
*/
int
grid_draw (Grid, Position, Position, Element, Magnitude, Direction);

#endif
