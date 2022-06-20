#include <stdio.h>

int
main(void)
{
	fprintf(stderr, "> ");

	long pos = ftell(stderr);

	printf("(pos){%ld}\n", pos);
	int ret = 0;
	if (pos == -1) { // tty
		fprintf(stderr, "\b\b");
	} else { // file
		ret = fseek(stderr, pos - 2, SEEK_SET);
	}
	printf("ret{%d}, (pos - 2){%ld}\n", ret, pos-2);


	fprintf(stderr, "GOT: xyz\n> ");


	return 0;
}
