#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct _sum_runner *sum_runner;
typedef struct _sum_runner {
	long long 	limit;
	long long	answer;
} _sum_runner;

void *
sum_runner_func(void *arg)
{
	sum_runner arg_struct = (sum_runner)arg;
	//long long *limit_ptr = (long long *) arg;
	//long long limit = *(arg_struct).limit;

	for (long long i = 0; i <= arg_struct->limit; i++)
	{
		(*arg_struct).answer +=i;
	}

	pthread_exit(0);
}

int
main (int argc, char **argv)
{
	if (argc < 2)
	{
		fprintf(stderr, "Usage: %s <num_1> <num_2> <num_3> .. <num_n>\n", argv[0]);
		exit(-1);
	}

	int num_args = argc - 1;


	// Thread ID:
	pthread_t tids[num_args];
	_sum_runner s_r_s[num_args];
	for (int i = 0; i < num_args; i++)
	{
		s_r_s[i] = (_sum_runner) { .limit = atoll(argv[i+1]), .answer = 0 };
		//long long limit = atoll(argv[i+1]);
		
		// maybe copping address space because global variables are same as this current process
		// the addresss space is the same
		// if run on same variable instead of array of i-th struct then all printfs give same value that is the (argc-1)*sum_of(argv[1])
		// Creating attributes
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_create(&tids[i], &attr, &sum_runner_func, &(s_r_s[i]));
	}
	// Do stuff here while waiting for thread doing task
	for (int i = 0; i < num_args; i++)
	{
		// parent thread waits on i-th thread sequentially
		pthread_join(tids[i], NULL);

		printf("Sum is: %lld\n", (s_r_s[i]).answer);		
	}
	return 0;
}
