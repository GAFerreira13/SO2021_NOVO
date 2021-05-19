/*
 * $Id: echo.c,v 1.5 2004/10/26 03:32:21 corbet Exp $
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/types.h>

//#include "echo.h"

MODULE_LICENSE("Dual BSD/GPL");

#define ECHO_MAJOR 0   /* dynamic major by default */
#define ECHO_DEVS 4 	 /* echo0 through echo3 */

int echo_major = ECHO_MAJOR;     /* echo.c */
int echo_devs = ECHO_DEVS;


static int echo_init(void)
{
	dev_t dev = MKDEV (echo_major, 0);

	if (!alloc_chrdev_region (&dev, 	echo_devs, 	0, 	"echo")) {
		printk(KERN_ALERT "Error in allocationg device!\n");
		return -1;
	}
	echo_major = MAJOR (dev);
	printk(KERN_ALERT "Echo device created! Device number:\n");
	return 0;
}

static void echo_exit(void)
{
	//kfree(dev);
	unregister_chrdev_region(MKDEV (echo_major, 0), echo_devs);
	printk(KERN_ALERT "Device has been freed!\n");
}

module_init(echo_init);
module_exit(echo_exit);
