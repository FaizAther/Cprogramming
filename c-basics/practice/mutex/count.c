#include <stdio.h>
#include <pthread.h>

#define NUM_LOOPS 20000000

long long sum = 0;

// https://linux.die.net/man/3/pthread_mutex_init
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *
counting_thread(void *arg)
{
	int offset = *((int *)arg);
	for (int i = 0; i < NUM_LOOPS; i++)
	{
		// start critical section
		pthread_mutex_lock(&mutex);

		sum += offset;  // if not locked different pthreads acting on data
				// at same time and causing memory corruption
				// be very careful in shared data could clober
				

		pthread_mutex_unlock(&mutex);
		// end critical section
	}
	pthread_exit(0);
}

int
main(void)
{
//	counting_function(1);
	int offset1 = 1;
	pthread_t id1;
	pthread_attr_t attr1;
	pthread_attr_init(&attr1);
	pthread_create(&id1, NULL, &counting_thread, &offset1);


	int offset2 = -1;
	pthread_t id2;
	pthread_attr_t attr2;
	pthread_attr_init(&attr2);
	pthread_create(&id2, NULL, &counting_thread, &offset2);

	pthread_join(id1, NULL);
	pthread_join(id2, NULL);
//	counting_function(-1);
	printf("Sum = %lld\n", sum);
	return 0;
}

