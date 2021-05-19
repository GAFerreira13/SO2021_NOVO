/*
 * $Id: hello.c,v 1.5 2004/10/26 03:32:21 corbet Exp $
 */
//#include <linux/config.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h>	  /* kmalloc() */
#include <linux/mm.h>
#include <linux/fs.h>	 /* everything... */
#include <linux/errno.h> /* error codes */
#include <linux/types.h> /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h> /* O_ACCMODE */
#include <linux/aio.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/cdev.h>
//#include <linux/ioport.h>
//#include <linux/sched.h>


#define ECHO_DEVS 1


//#define PORT_COM1 0x3f8
typedef short word;

//REGS

// #define REG_RHR 0
// #define REG_THR 0
// #define REG_IER 1
// #define REG_LCR 3
// #define REG_LSR 5

//LCR
#define PARITY_NONE 0
#define PARITY_ODD 8
#define PARITY_EVEN 24

#define STOP_ONE 0
#define STOP_TWO 4

#define BITS_5 0
#define BITS_6 1
#define BITS_7 2
#define BITS_8 3

#define DLR_ON 128

//LSR
// #define UART_LSR_TEMT 0x40 /* Transmitter empty */
// #define UART_LSR_THRE 0x20 /* Transmit-hold-register empty */
// #define UART_LSR_BI 0x10   /* Break interrupt indicator */
// #define UART_LSR_FE 0x08   /* Frame error indicator, (stop bit) */
// #define UART_LSR_PE 0x04   /* Parity error indicator */
// #define UART_LSR_OE 0x02   /* Overrun error indicator */
// #define UART_LSR_DR 0x01   /* Receiver data ready */

MODULE_LICENSE("Dual BSD/GPL");

int echo_open(struct inode *inodep, struct file *filep);
int echo_release(struct inode *inodep, struct file *filep);
ssize_t echo_read(struct file *filep, char __user *buff, size_t count, loff_t *offp);
ssize_t echo_write(struct file *filep, const char __user *buff, size_t count, loff_t *offp);

dev_t echo_number;
struct cdev *echo_cdev;
struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = echo_open,
	.release = echo_release,
	.read = echo_read,
	.write = echo_write,
};

int RW_ERR = 0;

struct file *echo_file;

static int echo_init(void)
{
	int a, b = 0;

	printk(KERN_ALERT "Hey hey world\n");

	a = alloc_chrdev_region(&echo_number, 0, 1, "echo");
	if (a != 0)
	{
		printk(KERN_ALERT "Erro a criar major device number");
	}

	printk(KERN_ALERT "major: %d\n", MAJOR(echo_number));

	echo_cdev = cdev_alloc();
	echo_cdev->ops = &fops;
	echo_cdev->owner = THIS_MODULE;

	b = cdev_add(echo_cdev, echo_number, ECHO_DEVS);

	return 0;
}


static void echo_exit(void)
{
	unsigned int a = MAJOR(echo_number);

	unregister_chrdev_region(echo_number, ECHO_DEVS);

	printk(KERN_ALERT "Goodbye, cruel world\n");
	printk(KERN_ALERT "major: %d\n", a);

	cdev_del(echo_cdev);
	
}

int echo_open(struct inode *inodep, struct file *filep)
{
	int a = 0;
	filep->private_data = inodep->i_cdev;
	printk(KERN_ALERT "int open\n");

	a = nonseekable_open(inodep, filep);

	return 0;
}

int echo_release(struct inode *inodep, struct file *filep)
{
	printk(KERN_ALERT "int release\n");
	return 0;
}


// read will return the number of characters written by the DD on the device since it was last loaded
ssize_t echo_read(struct file *filep, char __user *buff, size_t count, loff_t *offp)
{
	unsigned long a;
	if (RW_ERR == 0)
	{
		a = copy_to_user(buff, filep, (int)count);
		printk(KERN_ALERT "%s\n", (char *)buff);
		if (a != 0)
		{
			RW_ERR = 1;
			return (ssize_t)count - a;
		}
		else
		{
			RW_ERR = 0;
			return (ssize_t)count;
		}
	}
	else {
		printk(KERN_ALERT "Houve um erro no ssize_t read previo\n");
		return -1;
} }

//a write to an echo device will make it print whatever an application writes to it on the console
ssize_t echo_write(struct file *filep, const char __user *buff, size_t count, loff_t *offp)
{
	if (RW_ERR == 0)
	{
		int a, b = 0;
		char *temp = kmalloc(count + 1, GFP_KERNEL);
		a = copy_from_user(temp, buff, (unsigned long)count);
		temp[count + 1] = '0';
		b = copy_to_user(temp, buff, (unsigned long)count + 1);
		printk(KERN_ALERT "%s\n", temp);
		kfree(temp);
		if (a != 0 || b != 0)
		{
			RW_ERR = 1;
			return (ssize_t)count - (a + b);
		}
		else
		{
			RW_ERR = 0;
			return (ssize_t)count;
		}
	}
	else {
		printk(KERN_ALERT "Houve um erro no ssize_t write previo\n");
		return -1;
} }

module_init(echo_init);
module_exit(echo_exit);
