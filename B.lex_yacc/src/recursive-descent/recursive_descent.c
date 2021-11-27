#include <stdio.h>
#include <stdbool.h>

#include "recursive_descent.h"

TOKEN *next;

/*	__ General Boiler Plate __
bool
S_n ()
{
	return 0;
}

bool
S ()
{
	return 0;
}

*/

/*
 *	E -> T
 * */
bool
E_1 ()
{
	return T();
}

/*
 *	E -> T + E
 * */
bool
E_2 ()
{
	return T () && PLUS (*next++) && E ();
}

/*
 *	E handeling backtracking by using E_n ()
 *	and saving the pointer by trying...
 * */
bool
E ()
{
	TOKEN *save = next;
	return ( next = save, E_1 () && *next == '\0')
		|| ( next = save, E_2 () && *next == '\0');
}

/*
 *	T -> int
 * */
bool
T_1 ()
{
	return INT (*next++);
}

/*
 *	T -> int * T
 * */
bool
T_2 ()
{
	return INT (*next++) && TIMES (*next++) && T ();
}

/*
 *	T -> ( E )
 * */
bool
T_3 ()
{
	return OPEN (*next++) && E () && CLOSE (*next++);
}

bool
T ()
{
	TOKEN *save = next;
	return ( next = save, T_1 () )
		|| ( next = save, T_2 () )
		|| ( next = save, T_3 () );
}

int
main(int argc, char **argv)
{
	next = argv[1];

	if ( true == E () && *next == '\0')
		printf ("success\n");
	else
		printf ("fail\n");

	return 0;
}
