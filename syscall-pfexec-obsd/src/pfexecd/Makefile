SRCS=	parse.y pfexecd.c env.c pfexecd.h

PROG=	pfexecd
MAN=	pfexecd.8 pfexecd.conf.5

CFLAGS+= -g -O0 -I${.CURDIR}
COPTS+= -Wall
YFLAGS=

LDADD=	-levent

.include <bsd.prog.mk>
