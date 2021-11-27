#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int
main(void)
{
	int count0=0, count1=0;

	printf("Forking now...\n");

	pid_t pid=fork();

	if ((int) pid < 0)
	{
		perror("Fork failed");
	}
	else if((int) pid == 0 )
	{
		printf("in C pid: %d\n", getpid());
		while(count0<9)
		{
			printf("CC count: %d\n", count0);
			sleep(3);
			count0++;
		}
		printf("Child exiting...\n");
		exit(0);
	}
	else
	{
		printf("in P pid: %d\n", getpid());
		while(count1<7)
		{
			printf("PP count: %d\n", count1);
			sleep(3);
			count1++;
		}
	}

	printf("Parent waiting for child to end.\n");
	wait(NULL);
	printf("Parent Ending\n");

	return 0;
}
