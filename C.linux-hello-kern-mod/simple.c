/**
 * simple.c
 *
 * A simple kernel module. 
 * 
 * To compile, run makefile by entering "make"
 *
 * Operating System Concepts - 10th Edition
 * Copyright John Wiley & Sons - 2018
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/slab.h>

#include <linux/hash.h>
#include <linux/gcd.h>

typedef struct _birthday {
	int day;
	int month;
	int year;
	struct list_head list;
} _birthday;

typedef _birthday *Birthday;

static void birthday_show ( Birthday person );

LIST_HEAD (birthday_list);

/* This function is called when the module is loaded. */
int simple_init(void) {
	Birthday person;
	Birthday ptr;
	int i = 0;

	printk(KERN_INFO "Adding Module\n");

    printk (KERN_INFO "Golden Ratio: %lu\n", GOLDEN_RATIO_PRIME);

	while ( i < 5 ) {
		person = kmalloc(sizeof(*person), GFP_KERNEL);
		person->day = 2 + i;
		person->month= 8 + i;
		person->year = 1995 + i;
		INIT_LIST_HEAD(&person->list);
		list_add_tail(&person->list, &birthday_list);

		i += 1;
	}

	list_for_each_entry(ptr, &birthday_list, list) {
		/* on each iteration ptr points */
		/* to the next birthday struct */
		birthday_show (ptr);
	}
	return 0;
}

/* This function is called when the module is removed. */
void simple_exit(void) {
	Birthday ptr, next;
	printk(KERN_INFO "Removing Module\n");

    printk(KERN_INFO "gcd(3,300, 24) == %lu\n", gcd(3300UL, 24UL));

	list_for_each_entry_safe(ptr,next,&birthday_list,list) {
		/* on each iteration ptr points */
		/* to the next birthday struct */
		list_del(&ptr->list);
		kfree(ptr);
	}
}

static void birthday_show ( Birthday person ) {
	if (person == NULL) {
		printk ("NULL");
	} else {
		printk ("ptr: %p, day: %d, month: %d, year: %d\n", \
			person, person->day, person->month, person->year);
	}
	return;
}

/* Macros for registering module entry and exit points. */
module_init( simple_init );
module_exit( simple_exit );

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple Module");
MODULE_AUTHOR("SGG");

