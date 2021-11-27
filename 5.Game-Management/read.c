#include <stdio.h>
#include <assert.h>

#include "read.h"
#include "handle.h"

#define MAX_LINE_SIZE   4096
#define STR_(X)         #X
#define STR(X)          STR_(X)

/*
Simply reads, cleans and ignores the #'s
    clean the stream, make a malloc'd copy and save in ADT
*/
Queue
simple_read (char *filename)
{
    FILE *f = fopen (filename, "r");
    char s[MAX_LINE_SIZE] = {'\0'};
    Queue q = queue_init();
    assert (q != NULL);
    while ( ( fscanf (f, "%" STR(MAX_LINE_SIZE) "[^\n]\n", s) ) != EOF)
    {
        if (s[0] == '#') continue;
        queue_enqueue (q, s);
    }
    fclose (f);
    return q;
}

Data
read_files (char **files, int count)
{
    Queue *data = (Queue *)malloc (sizeof (*data)*(FILE_COUNT(count)));
    assert (data != NULL);

    data[DATA_GAME] = simple_read (files[FILE_GAME]); //game
    data[DATA_PLAYER_MAP] = simple_read (files[FILE_PLAYER_MAP]); //player pos
    data[DATA_CPU_MAP] = simple_read (files[FILE_CPU_MAP]); //cpu pos
    //data[DATA_CPU_MOVES] = simple_read (files[FILE_CPU_MOVES]); //cpu moves

    return data;
}

Data
read_make_data (int count)
{
    Queue *data = (Queue *)malloc (sizeof (*data)*(count));
    assert (data != NULL);
    return data;
}

Queue
read_string_rules (char *string)
{
    //move forward "RULES " so 6 
    string += 6;
    return queue_to_queue (string, ',');
}

Queue
read_string_map (char *string)
{
    //move forward "MAP " so 4
    string += 4;
    thing_replace (string, ',', ' ');
    return queue_to_queue (string, ':');
}

void
read_destroy (Data d, int count)
{
    for (int i = 0; i < FILE_COUNT(count); i++) queue_destroy (d[i]);
    free (d);
}

