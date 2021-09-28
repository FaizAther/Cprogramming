//	Mohammad Faiz ATHER === UQ 2020
//
//	grid.h
//	an implementation of a Grid using int**
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "grid.h"

typedef struct _grid {
    Element     **plain;
    Position     size_x;
    Position     size_y;
} _grid;

typedef Element **Plain;


static Position
position_resolve (Position p)
{
    return p-1;
}

static int
check_inside (Position x, Position max_x, Position y, Position max_y)
{
    return ( x >= 0 && x < max_x && y >= 0 && y < max_y );
}

static int
check_inside_max (Position x, Position y)
{
    return check_inside (x-1, GRID_MAX_X, y-1, GRID_MAX_Y);
}

static int
check_inside_this (Grid g, Position x, Position y)
{
    return check_inside (x, g->size_x, y, g->size_y);
}

Grid
grid_init (Position x, Position y)
{
    if (!check_inside_max (x, y))
        return NULL;

    Grid g = (Grid)malloc (sizeof (*g));
    g->size_x = x;
    g->size_y = y;
    assert (g != NULL);
    g->plain = (Plain)malloc (sizeof (Element *)*(y+1));
    assert (g->plain != NULL);
    for (int y = 0; y < g->size_y; y++)
    {
        g->plain[y] = malloc (sizeof(Element)*(x+2));
        assert (g->plain[y] != NULL);
    }
    grid_clear (g);
    return g;
}

void
grid_clear (Grid g)
{
    for (int y = 0; y < g->size_y; y++)
    {
        memset (g->plain[y], DEFAULT_ELEMENT, sizeof(Element)*(g->size_x));
        g->plain[y][g->size_x] = '\n';
        g->plain[y][g->size_x] = '\0';
    }
}

int
grid_set (Grid g, Position x, Position y, Element e)
{
    assert (g != NULL);
    assert (g->plain != NULL);
    x = position_resolve (x);
    y = position_resolve (y);
    if (!check_inside_this (g, x, y))
        return GRID_BOUND_ERROR;
    g->plain[y][x] = e;
    return 0;
}

Element
grid_get (Grid g, Position x, Position y)
{
    assert (g != NULL);
    assert (g->plain != NULL);
    x = position_resolve (x);
    y = position_resolve (y);
    if (!check_inside_this (g, x, y))
        return GRID_BOUND_ERROR;
    return g->plain[y][x];    
}

static char
discrete_check (Element e, char *show, int n)
{
    for (int i = 0; i < n; i++)
    {
        if (e == show[i]) return e;
    }
    return DEFAULT_ELEMENT;
}

static int
length_int (int i)
{
    int s = 0;
    while (i > 0)
    {
        i /= 10;
        s += 1;
    }
    return s;
}

static char *
create_white_space (int size)
{
    char *white_s = (char *)malloc (sizeof(char)*(size+1));
    memset (white_s, ' ', sizeof(char)*size);
    white_s[size] = '\0';
    return white_s;
}

static void
grid_show_general (Grid g, bool discrete, char *show, int n)
{
    int w = length_int(g->size_y);
    char *w_s = create_white_space (w);
    putchar (' ');
    for (int i = 0; i < ((w==2)?1:w); i++) putchar (' ');
    putchar (' ');
    for (char i = 'A'; i < 'A'+ g->size_x; i++)
        putchar (i);
    putchar ('\n');
    char e = '\0';
    for (int y = 0; y < g->size_y; y++)
    {
        char *tt = w==1?w_s+length_int(y+1)-1:w_s+length_int(y+1);
        printf ("%s%d ", tt, y+1);
        for (int x = 0; x < g->size_x; x++)
        {
            e = g->plain[y][x];
            if (discrete) 
                e = discrete_check (e, show, n);
            //else if (!discrete) {
            //    if (e == '/') e = DEFAULT_ELEMENT;
            //}
            putchar( e );
        }
        putchar ('\n');
    }
    free (w_s);
}

void
grid_show (Grid g)
{
    grid_show_general (g, false, NULL, 0);
}

static void
grid_show_general_err (Grid g, bool discrete, char *show, int n)
{
    int w = length_int(g->size_y);
    char *w_s = create_white_space (w);
    fputc (' ', stderr);
    for (int i = 0; i < ((w==2)?1:w); i++) fputc (' ', stderr);
    fputc (' ', stderr);
    for (char i = 'A'; i < 'A'+ g->size_x; i++)
        fputc (i, stderr);
    fputc ('\n', stderr);
    char e = '\0';
    for (int y = 0; y < g->size_y; y++)
    {
        char *tt = w==1?w_s+length_int(y+1)-1:w_s+length_int(y+1);
        fprintf (stderr, "%s%d ", tt, y+1);
        for (int x = 0; x < g->size_x; x++)
        {
            e = g->plain[y][x];
            if (discrete) 
                e = discrete_check (e, show, n);
            //else if (!discrete) {
            //    if (e == '/') e = DEFAULT_ELEMENT;
            //}
            fputc( e, stderr );
        }
        fputc ('\n', stderr);
    }
    free (w_s);
}

void
grid_show_err (Grid g)
{
    grid_show_general_err (g, false, NULL, 0);
}

void
grid_show_discrete (Grid g, char *show, int n)
{
    grid_show_general (g, true, show, n);
}

void
grid_destroy (Grid g)
{
    assert (g != NULL);
    assert (g->plain != NULL);
    for (int y = 0; y < g->size_y; y++)
    {
        free (g->plain[y]);
    }
    free (g->plain);
    free (g);
}

static int
resolve_cord_x (Position x, Direction d)
{
    switch (d)
    {
        case DIRECTION_EAST: return x + 1;
        case DIRECTION_WEST: return x - 1;
    }
    return x;
}

static int
resolve_cord_y (Position y, Direction d)
{
    switch (d)
    {
        case DIRECTION_NORTH: return y - 1;
        case DIRECTION_SOUTH: return y + 1;
    }
    return y;
}

static int
draw (Grid g, Position x, Position y, Element e, Magnitude m, Direction d)
{
    if (m < 1) return 0;

    Position r_x = resolve_cord_x(x,d);
    Position r_y = resolve_cord_y(y,d);

    Element c;

    if (!check_inside_this(g, (x), (y)))
        return DRAW_BOUND_ERROR;
    else if ( c = (g->plain[y][x]), c != '.' )
        return DRAW_OVERLAP_ERROR;

    int res = draw (g, r_x, r_y, e, m-1, d);

    if (res == 0)
        g->plain[y][x] = e;

    return res;
}

int
grid_draw (Grid g, Position x, Position y, Element e, Magnitude m, Direction d)
{
    assert (g != NULL);
    assert (g->plain != NULL);
    if (e > '9') {
        e -= '9';
        e -= 1;
        e += 'A';
    }
    if (!(d == DIRECTION_EAST || d == DIRECTION_NORTH || d == DIRECTION_SOUTH || d == DIRECTION_WEST))
        return DRAW_DIRECTION_INVALID;
    int res = draw (g, position_resolve (x), position_resolve (y), e, m, d);
    //if (res != 0 && res == DRAW_BOUND_ERROR && check_inside_this(g, position_resolve (x), position_resolve (y))) return DRAW_BOUND_EXCEED_ERROR;
    return res;
}

/*
//Just for testing it
int
main (void)
{
    Grid g = grid_init (26, 26);
    grid_set (g, 26,26,'5');
    printf("%d\n", grid_draw (g, 1, 1, '*', 20, DIRECTION_SOUTH));
    printf("%d\n", grid_draw (g, 2, 2, '\\', 20, DIRECTION_SOUTH));

    grid_show_discrete (g, "*\\", 2);
    grid_show (g);
    grid_destroy (g);
}
*/