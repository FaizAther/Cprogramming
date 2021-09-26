#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>


int
main(void)
{
	int child;
	int count0=0, count1=0;

	printf("Forking now...\n");

	child=fork();
	if((int) child == 0 )
	{
		printf("in C pid: %d\n", getpid());
		while(count0<9)
		{
			printf("CC count: %d\n", count0);
			sleep(3);
			count0++;
		}
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

	if ((int) child == 0)
	{
		printf("exiting C\n");
	}
	else
	{
		printf("exiting P\n");
	}
	
	return 0;
}
