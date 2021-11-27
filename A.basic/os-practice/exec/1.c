#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
main(int argc, char **argv)
{
	printf("My PID is %d.\n", getpid());
	char *args[] = {"0", "i", "am", "a", "monster", NULL};
	execvp("./0", args);

	/*
		exec never returns unless error so this area never executed...
		process has been replaced here!.

		everything gets wiped out
		1 doesnt exist after exec call
	*/

	printf("My PID is %d.\n", getpid());


	return 0;
}
