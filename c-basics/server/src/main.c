#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <setjmp.h>
#include <stdbool.h>

#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "s.h"

extern char *optarg;

#define LISTEN_BACKLOG 50

#define KILO 1024
#define MEGA (KILO * KILO)

#define REQ_BUF_CAP (640 * KILO)

char req_buf[REQ_BUF_CAP] = {0};

jmp_buf handle_req_err;

char response[] = 
    "HTTP/1.1 200 OK\n"
    "Content-Type: text/html\n"
    "\n"
    "<html>"
        "<head>"
            "<title>PHP is the best!!!</title>"
        "</head>"
        "<body>"
            "<h1>PHP</h1>"
        "</body>"
    "</html>";

void
usage ( void )
{
    fprintf ( stderr, "usage: main [-46] url [-p port]\n" );
    exit ( 1 );
}

void
err_die ( const char *reason )
{
    perror ( reason );
    exit ( 1 );
}

void
request_error ( const char *reason )
{
    fprintf ( stderr, "%s\n", reason );
    longjmp ( handle_req_err, 1 );
}

void
handle_req ( int fd )
{
    ssize_t req_buf_siz = 0;
    string line = init_line ();
    string buf = init_line ();
    
    memset ( req_buf, 0, REQ_BUF_CAP );
    req_buf_siz = read ( fd, req_buf, REQ_BUF_CAP );
    buf = (string){
        .len = (uint64_t)req_buf_siz,
        .data = req_buf
    };
    if ( req_buf_siz == 0 )
        request_error ( "EOF" );
    else if ( req_buf_siz < 0 )
        request_error ( strerror ( errno ) );

    line = trim ( chop_line ( &buf ) );

    while ( buf.len && line.len )
    {
        printf ("|%.*s|\n", (int)line.len, line.data );
        line = trim ( chop_line ( &buf ) );
    }
}

int
Socket ( int ip_v, int type )
{
    int sfd = -1;

    if ( sfd = socket ( ip_v, type, 0 ), sfd < 0 )
    {
       err_die ( "socket" ); 
    }
    return sfd;
}

int
main ( int argc, char *argv[] )
{
    ssize_t err = 0;
    int opt = -1,
        sfd = -1, cfd = -1;
    bool ip_v6 = false;
    char *port = "80";
    struct sockaddr_in my_addr, peer_addr;
    socklen_t peer_addr_size;

    if ( argc < 2 )
    {
        usage();
    }

    while ( ( opt = getopt ( argc, argv,
                "46p:" ) ) != -1 )
    {
        switch (opt)
        {
            case '4':
                break;
            case '6':
                ip_v6 = true;
                break;
            case 'p':
                port = optarg;
                break;
            default:
                usage(  );
        }
    }

    sfd = Socket ( (ip_v6 ? AF_INET6 : AF_INET), SOCK_STREAM );

    memset ( &my_addr, 0, sizeof( struct sockaddr_in ) );
    my_addr.sin_family = (ip_v6 ? AF_INET6 : AF_INET);
    my_addr.sin_port = htons( (uint16_t)strtoul( port, NULL, 10 ) );

    if ( bind ( sfd, ( struct sockaddr * )&my_addr,
            sizeof( struct sockaddr_in ) ) )
        err_die ( "bind" );

    if ( listen ( sfd, LISTEN_BACKLOG ) == -1 )
        err_die ( "listen" );
    for ( ;; )
    {
        peer_addr_size = sizeof ( struct sockaddr_in );
        memset ( &peer_addr, 0, peer_addr_size );
        cfd = accept ( sfd, ( struct sockaddr * )&peer_addr,
                &peer_addr_size );
        if ( cfd == -1 )
            err_die ( "accept" );
        
        if ( setjmp (handle_req_err) == 0 )
            handle_req ( cfd );

        err = write ( cfd, response, sizeof( response ) );
        if ( err < 0 )
         err_die ( "write" );

        close ( cfd );
        printf ( "x---x---x---x---x---x---x---x\n" );
    }
   
    close ( sfd );
    return 0;
}
