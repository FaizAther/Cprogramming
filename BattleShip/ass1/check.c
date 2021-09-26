//	Mohammad Faiz ATHER === UQ 2020
//
//	check.c
#include <unistd.h>

#include "handle.h"
#include "check.h"

int
okay_args (int count)
{
    return (count >= ARGUMENT_COUNT ? SUCCESS: PARAMETER_ERR); 
}

bool
okay_file (char *filename, int mode)
{
    return ( 0 == access (filename, mode));
}

int
okay_files (char **files)
{
    files=(files + 1);
    for ( int i = 0; i < FILE_COUNT; i+=1 )
    {
        if (okay_file (files[i], R_OK) == false) return what_error_file (i);
    }
    return SUCCESS;
}