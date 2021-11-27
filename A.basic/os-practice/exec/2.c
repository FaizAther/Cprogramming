#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
main(int argc, char **argv)
{
	printf("My PID is %d.\n", getpid());

	if (argc != 2)
	{
		printf("Pass 1 argument as an integer\n");
		exit(1);
	}
	int n = atoi(argv[1]);
	printf("%d..\n", n);

	if (n != 0)
	{
		char nMinus1[10];
		sprintf(nMinus1, "%d", n-1);
		execl(argv[0], nMinus1, NULL);
	}


	/*
		This is count down replaces old process with new n from nMinus
		snprintf writes to specified stream
		
		exec never returns unless error so this area never executed...
		process has been replaced here!.

		everything gets wiped out
		1 doesnt exist after exec call
	*/

	printf("My PID is %d.\n", getpid());


	return 0;
}
