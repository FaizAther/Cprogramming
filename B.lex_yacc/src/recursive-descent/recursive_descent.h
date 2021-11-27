#ifndef	RECURSIVE_DESCENT_H_
#define RECURSIVE_DESCENT_H_

typedef char TOKEN;

#define PLUS(T)		(T == '+')
#define MINUX(T)	(T == '-')
#define TIMES(T)	(T == '*')
#define DIVIDE(T)	(T == '/')
#define OPEN(T)		(T == '(')
#define CLOSE(T)	(T == ')')

bool INT(TOKEN T)	{return (T >= '0' && T <= '9');}

bool
T ();

bool
T_1 ();

bool
T_2 ();

bool
T_3 ();

bool
E ();

bool
E_1 ();

bool
E_2 ();

#endif //RECURSIVE_DESCENT_H_
