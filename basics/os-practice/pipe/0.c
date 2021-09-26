#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
main (int argc, char **argv)
{
	pid_t pid;
	int mypipefd[2];
	int ret;
	char buf[25];

	ret = pipe(mypipefd);

	if (ret == -1)
	{
		perror("Pipe error!");
		exit(1);
	}

	pid = fork();

	if (pid == 0)
	{
		/*Child process*/
		printf("Child process\n");
		// 1 is for writing 0 is for reading
		write(mypipefd[1], "Hello there! i am child", 23);
	} else {
		/*Parent process*/
		printf("Parent process\n");
		read(mypipefd[0], buf, 25);
		printf("buf : %s\n", buf);
	}


	return 0;
}
