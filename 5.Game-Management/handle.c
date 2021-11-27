//	Mohammad Faiz ATHER === UQ 2020
//
//	handle.h
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>

#include "handle.h"

char*
ERRORS_STRING[ERRORS_NUM] = {PARAMETER, FILE_PROCESS_RULES};


void
handle_exit (int e)
{
    fprintf (stderr, strcat (ERRORS_STRING[e-1], "\n"));
    exit (e);
}
