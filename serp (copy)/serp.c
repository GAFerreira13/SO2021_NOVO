/*
 * 	Trabalho realizado por
 *	Rui André Roxo Queirós
 *	Gonçalo Ferreira de Albuquerque
 */

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
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/delay.h>

#define PORT_COM1 0x3f8
typedef short word;

//REGS

#define REG_RHR 0
#define REG_THR 0
#define REG_IER 1
#define REG_LCR 3
#define REG_LSR 5

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
#define UART_LSR_TEMT 0x40 /* Transmitter empty */
#define UART_LSR_THRE 0x20 /* Transmit-hold-register empty */
#define UART_LSR_BI 0x10   /* Break interrupt indicator */
#define UART_LSR_FE 0x08   /* Frame error indicator, (stop bit) */
#define UART_LSR_PE 0x04   /* Parity error indicator */
#define UART_LSR_OE 0x02   /* Overrun error indicator */
#define UART_LSR_DR 0x01   /* Receiver data ready */

MODULE_LICENSE("Dual BSD/GPL");

#define SERP_DEVS 1

int serp_open(struct inode *inodep, struct file *filep);
int serp_release(struct inode *inodep, struct file *filep);
ssize_t serp_read(struct file *filep, char __user *buff, size_t count, loff_t *offp);
ssize_t serp_write(struct file *filep, const char __user *buff, size_t count, loff_t *offp);
void write_uart(int COM_port, int reg, int data);
int read_uart(int COM_port, int reg);
void serial_write(unsigned char ch);
int setup_serial(int COM_port, int baud, unsigned char misc);

dev_t serp_number;
struct cdev *serp_cdev;
struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = serp_open,
	.release = serp_release,
	.read = serp_read,
	.write = serp_write,
};
struct resource *serp_res;
int RW_ERR = 0;

static int serp_init(void)
{
	printk(KERN_ALERT "Serp initialization!\n");

	if (alloc_chrdev_region(&serp_number, 0, 1, "serp"))
	{
		printk(KERN_ALERT "Error creating major device number!");
		return -1;
	}

	printk(KERN_ALERT "Major number: %d\n", MAJOR(serp_number));

	serp_cdev = cdev_alloc();
	serp_cdev->ops = &fops;
	serp_cdev->owner = THIS_MODULE;

	if (cdev_add(serp_cdev, serp_number, SERP_DEVS) < 0)
	{
		printk(KERN_ALERT "Error adding minor device!");
	}

	serp_res = request_region(0x3f8, 8, "serp");
	setup_serial(PORT_COM1, 96, BITS_8 | PARITY_EVEN | STOP_TWO);

	return 0;
}

static void serp_exit(void)
{
	printk(KERN_ALERT "Serp exit funtion called!\n");
	printk(KERN_ALERT "Major number: %d\n", MAJOR(serp_number));

	cdev_del(serp_cdev);
	kfree(serp_cdev);

	unregister_chrdev_region(serp_number, SERP_DEVS);

	return;
}

int serp_open(struct inode *inodep, struct file *filep)
{

	printk(KERN_ALERT "Open function called!\n");

	filep->private_data = inodep->i_cdev;
	nonseekable_open(inodep, filep);

	return 0;
}

int serp_release(struct inode *inodep, struct file *filep)
{
	printk(KERN_ALERT "Release function called!\n");
	return 0;
}

ssize_t serp_read(struct file *filep, char __user *buff, size_t count, loff_t *offp)
{

	unsigned char b = '\0', rcv = '\0';
	int n = 0, timeout = 0;
	char *temp = kmalloc(count + 1, GFP_KERNEL);

	set_current_state(TASK_INTERRUPTIBLE);

	if (RW_ERR)
	{
		printk(KERN_ALERT "There was an error in the previous read! Returning now...\n");
		RW_ERR = 0;
		kfree(temp);
		return -EINTR;
	}

	while (1)
	{

		b = read_uart(PORT_COM1, REG_LSR);
		if (b & (UART_LSR_FE | UART_LSR_OE | UART_LSR_PE))
		{
			printk(KERN_ALERT "Error in the read data!\n");
			RW_ERR = 1;
			kfree(temp);
			return -EIO;
		}

		if (b & UART_LSR_DR)
		{
			timeout = 0;
			rcv = (char)read_uart(PORT_COM1, REG_RHR);
			if (rcv != '!')
			{
				temp[n++] = rcv;
				if (n == count)
				{
					break;
				}
			}
			else
			{
				break;
			}
		}
		else
		{
			msleep_interruptible(500);
			timeout++;
			if (timeout > 10)
			{
				printk(KERN_ALERT "Timeout achieved!\n");
				break;
			}
		}
	}
	temp[n] = '\0';
	if (copy_to_user(buff, temp, n))
	{
		printk(KERN_ALERT "Wrong number of characters read!\n");
		RW_ERR = 1;
	}
	kfree(temp);
	return (ssize_t)count;
}

ssize_t serp_write(struct file *filep, const char __user *buff, size_t count, loff_t *offp)
{

	int i, a = 0;
	char *temp = kmalloc(count, GFP_KERNEL);

	if (RW_ERR)
	{
		printk(KERN_ALERT "There was an error in the previous write! Returning now...\n");
		RW_ERR = 0;
		kfree(temp);
		return -EINTR;
	}

	a = copy_from_user(temp, buff, (unsigned long)count);

	for (i = 0; i < count; i++)
	{
		serial_write(temp[i]);
	}

	kfree(temp);

	if (a != 0)
	{
		printk(KERN_ALERT "Wrong number of written letters!\n");
		RW_ERR = 1;
	}

	RW_ERR = 0;
	return (ssize_t)count - a;
}

/* Funções auxiliares */

int read_uart(int COM_port, int reg)
{
	return (inb(COM_port + reg));
}

void write_uart(int COM_port, int reg, int data)
{
	outb(data, (COM_port + reg)); 
	return;
}

void serial_write(unsigned char ch)
{
	while (1)
	{
		if (!((unsigned char)read_uart(PORT_COM1, REG_LSR) & 0x20))
		{
			schedule();
		}
		else
			break;
	}

	write_uart(PORT_COM1, REG_THR, (int)ch);
	return;
}

int setup_serial(int COM_port, int baud, unsigned char misc)
{
	word divisor;

	write_uart(COM_port, REG_IER, 0);
	write_uart(COM_port, REG_LCR, (int)DLR_ON);
	divisor = 0x1c200 / baud;
	outw(divisor, COM_port); 

	write_uart(COM_port, REG_LCR, (int)misc);
	return 1;
}

module_init(serp_init);
module_exit(serp_exit);
