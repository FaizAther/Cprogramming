#ifndef S_H_
#define S_H_

#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>

typedef struct _string {
    uint64_t    len;
    char        *data;
} string;

typedef string *String;

static inline string
init_line ( void )
{
    return (string){
        .len = 0,
        .data = NULL
    };
}

static inline string
chop_line ( String inp )
{
    uint64_t i = 0;
    while ( i < inp->len &&\
        inp->data[i] != '\n' )
    {
        ++i;
    }
    
    string line = {
        .len = 0,
        .data = NULL
    };
    line.data = inp->data;
    line.len = i;

    if ( i == inp->len )
    {
        inp->data += inp->len;
        inp->len = 0;
    } else
    {
        inp->data += i + 1;
        inp->len -= i + 1;
    }

    return line;
}

string
trim ( string inp )
{
    while ( inp.len &&\
            isspace ( inp.data[inp.len -1] ) )
        inp.len--;
    return inp;
}


#endif // S_H_
