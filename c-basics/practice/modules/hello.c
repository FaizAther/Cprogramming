#include <linux/module.h>

static
int
hello_init(void)
{
	printk(KERN_ALERT "TEST: Hello, World!\n");

	return 0;
}

static
void
hello_exit(void)
{
	printk(KERN_ALERT "TEST: Good byte!\n");

	return;
}

module_init(hello_init);
module_exit(hello_exit);
