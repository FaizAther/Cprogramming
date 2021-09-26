#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>


void
show_data(int *data, size_t s)
{
	if (s <= 1 || data == NULL) return;
	int i = 0;
	while(i < s)
	{
		printf("%d ", *(data + i));
		i++;
	}
	printf("\n");
}

bool
is_sorted(int *data, size_t s)
{
	if (s <= 1 || data == NULL) return true;
	int i = 1;
	while(i < s)
	{
		if (*(data + (i-1)) > *(data + i))
			return false;
		i++;
	}

	return true;
}

void
random_shuffle(int *data, size_t s)
{
	if (s<= 1 || data == NULL) return;
	int i = 0, r = 0, p = 0, d = 0;
	time_t t;
	srand((unsigned)time(&t));
	while(i < s)
	{
		r = rand() % s;
		d = *(data + p);
		*(data + p) = *(data + r);
		*(data + r) = d;
		p = r;
		i++;
		/** Debugging... show_data(data, 5); */
	}
}

int *
args_to_int(int argc, char **argv)
{
	if (argc <= 2 || argv == NULL) return NULL;
	int i = 1;
	int *data = (int *)calloc(argc-1, sizeof(int));
	while(i < argc)
	{
		*(data + (i-1)) = atoi(argv[i]);
		i++;
	}
	return data;
}



int
main(int argc, char **argv)
{
	//printf("%d->%d\n", argc, atoi(argv[1]));
	int *data = args_to_int(argc, argv);
	while(!is_sorted(data, argc-1))
	{
		//show_data(data, argc-1);
		random_shuffle(data, argc-1);
	}
	printf("%s PID: %d\n", argv[0], getpid());
	show_data(data, argc-1);
	free(data);

	return 0;
}
