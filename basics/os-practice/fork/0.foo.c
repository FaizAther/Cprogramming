#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>


int
main(void)
{
	int child;

	printf("preFork\n");

	child=fork();
	if((int) child == 0 )
	{
		printf("iamchild %d\n", getpid());
	}
	else
	{
		printf("ntchild %d\n", getpid());
	}
	
	return 0;
}
