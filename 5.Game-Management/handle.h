//	Mohammad Faiz ATHER === UQ 2020
//
//	handle.h
#include <stdio.h>
#include <stdlib.h>

#ifndef HANDLE_H_
#define HANDLE_H_

#define ARGUMENT_COUNT_NAVAL    4
#define FILE_COUNT(X)              (X-1)


#define PARAMETER               "Usage: naval rules playermap cpumap turns\n"
#define MISSING_RULES           "Missing rules file\n"
#define MISSING_PLAYER_MAP      "Missing player map file\n"
#define MISSING_CPU_MAP         "Missing CPU map file\n"
#define MISSING_CPU_TURNS       "Missing CPU turns file\n"
#define FILE_PROCESS_RULES      "Error in rules file\n"
#define OVERLAP_PLAYER_MAP      "Overlap in player map file\n"
#define OVERLAP_CPU_MAP         "Overlap in CPU map file\n"
#define OUT_BOUND_PLAYER_MAP    "Out of bounds in player map file\n"
#define OUT_BOUND_CPU_MAP       "Out of bounds in CPU map file\n"
#define PROCESS_PLAYER_MAP      "Error in player map file\n"
#define PROCESS_CPU_MAP         "Error in CPU map file\n"
#define PROCESS_TURNS           "Error in turns file\n"
#define PLAYER_QUIT             "Bad guess\n"
#define CPU_QUIT                "CPU player gives up\n"
#define UNKNOWN                 "Unknown\n"

#define AGENT_MAP_ERR           "Invalid map file\n"

#define SUCCESS                     0
#define PARAMETER_ERR               10
#define MISSING_RULES_ERR           20
#define MISSING_PLAYER_MAP_ERR      30
#define MISSING_CPU_MAP_ERR         31
#define MISSING_CPU_TURNS_ERR       40
#define FILE_PROCESS_RULES_ERR      50
#define OVERLAP_PLAYER_MAP_ERR      60
#define OVERLAP_CPU_MAP_ERR         70
#define OUT_BOUND_PLAYER_MAP_ERR    80
#define OUT_BOUND_CPU_MAP_ERR       90
#define PROCESS_PLAYER_MAP_ERR      100
#define PROCESS_CPU_MAP_ERR         110
#define PROCESS_TURNS_ERR           120
#define PLAYER_QUIT_ERR             130
#define CPU_QUIT_ERR                140
#define UNKNOWN_ERR                 150

typedef int Error;

static inline void
handle_exit (Error e)
{
    char *show = UNKNOWN;
    switch (e)
    {
        case PARAMETER_ERR: show =  PARAMETER; break;
        case MISSING_RULES_ERR: show =  MISSING_RULES; break;
        case MISSING_PLAYER_MAP_ERR: show =  MISSING_PLAYER_MAP; break;
        case MISSING_CPU_MAP_ERR: show =  MISSING_CPU_MAP; break;
        case MISSING_CPU_TURNS_ERR: show =  MISSING_CPU_TURNS; break;
        case FILE_PROCESS_RULES_ERR: show =  FILE_PROCESS_RULES; break;
        case OVERLAP_PLAYER_MAP_ERR: show =  OVERLAP_PLAYER_MAP; break;
        case OVERLAP_CPU_MAP_ERR: show =  OVERLAP_CPU_MAP; break;
        case OUT_BOUND_PLAYER_MAP_ERR: show =  OUT_BOUND_PLAYER_MAP; break;
        case OUT_BOUND_CPU_MAP_ERR: show =  OUT_BOUND_CPU_MAP; break;
        case PROCESS_PLAYER_MAP_ERR: show =  PROCESS_PLAYER_MAP; break;
        case PROCESS_CPU_MAP_ERR: show =  PROCESS_CPU_MAP; break;
        case PROCESS_TURNS_ERR: show =  PROCESS_TURNS; break;
        case PLAYER_QUIT_ERR: show =  PLAYER_QUIT; break;
        case CPU_QUIT_ERR: show =  CPU_QUIT; break;
        default: show = UNKNOWN; break;
    }

    fprintf (stderr, show);
    exit (e);
}

static inline Error
what_error_file (int i)
{
    switch (i)
    {
        case 0: return MISSING_RULES_ERR;
        case 1: return MISSING_PLAYER_MAP_ERR;
        case 2: return MISSING_CPU_MAP_ERR;
        case 3: return MISSING_CPU_TURNS_ERR;
    }
    return UNKNOWN_ERR;
}

#endif
