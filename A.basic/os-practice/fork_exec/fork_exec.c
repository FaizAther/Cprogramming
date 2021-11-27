#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define NUM_CHILDREN 2

int
main(int argc, char **argv)
{
	/** Do as little as possible between the fork and the exec because 
	linux will have to copy address space of current prog
	useless because alredy exec will replace whole process
	the copy of curr_process will be wiped out by exec	
	*/
	for (int i = 0; i < NUM_CHILDREN; i++)
		{
		pid_t pid = fork();
		if (pid < 0) {
			perror("fork: fail.");
			exit(1);
		} else if (pid == 0) {
			char *args[] = {"./bogosort",
				"1", "3", "9", "5", "0",
				"2", "8", "4", "14", "12",
				"22",
				NULL
			};
			execv(args[0], args);
		}
	}

	// now parent
	printf("parent doing amazing...\n");

	printf("parent waiting...\n");
	for (int i = 0; i < NUM_CHILDREN; i++)
		wait(NULL);

	return 0;
}
