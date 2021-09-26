// by genSvt
//

#include <stdio.h>

int
addNumber(int a, int b)
{
	return a + b;
}

int
main(int argc, char **argv)
{
	int result;

	int (*myFuncPtr)(int, int);

	myFuncPtr = &addNumber;

	result = myFuncPtr(3, 4);

	printf("result = %d\n", result);


	return 0;
}
