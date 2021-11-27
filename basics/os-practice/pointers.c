#include <stdio.h>

int
foo(void *i, char j)
{
	switch(j)
	{
	case 'i': printf("%d\n", *(int *)i); break;
	case 'c': printf("%c\n", *(char *)i); break;
	case 's': printf("%s\n", (char *)i); break;
	default: printf("Not found!\n");
	}
  return 0;
}



int
main(void)
{
	int i = 7;
	char c = 65, c_t = 'c';
	char *s = "hello";
	foo(&i, 'i');
	foo(&c, c_t);
	foo(s, 's');

	return 0;
}
