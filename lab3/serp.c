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

#define serp_DEVS 1

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

struct file *serp_file;
struct resource *serp_res;
int port_busy = 0;
int RW_ERR = 0;
//struct inode *serp_inode;

static int serp_init(void)
{
	int a, b = 0;

	printk(KERN_ALERT "ou deves querer que te foda\n");

	a = alloc_chrdev_region(&serp_number, 0, 1, "serp");
	if (a != 0)
	{
		printk(KERN_ALERT "Erro a criar major device number");
	}

	printk(KERN_ALERT "major: %d\n", MAJOR(serp_number));

	serp_cdev = cdev_alloc();
	serp_cdev->ops = &fops;
	serp_cdev->owner = THIS_MODULE;

	b = cdev_add(serp_cdev, serp_number, serp_DEVS);

	serp_res = request_region(0x3f8, 8, "serp");
	setup_serial(PORT_COM1, 96, BITS_8 | PARITY_EVEN | STOP_TWO);

	return 0;
}

static void serp_exit(void)
{
	unsigned int a = MAJOR(serp_number);

	unregister_chrdev_region(serp_number, serp_DEVS);

	printk(KERN_ALERT "Goodbye, cruel world\n");
	printk(KERN_ALERT "major: %d\n", a);

	cdev_del(serp_cdev);
}

int serp_open(struct inode *inodep, struct file *filep)
{
	int a = 0;
	filep->private_data = inodep->i_cdev;
	printk(KERN_ALERT "int open\n");

	a = nonseekable_open(inodep, filep);

	return 0;
}

int serp_release(struct inode *inodep, struct file *filep)
{
	printk(KERN_ALERT "int release\n");
	return 0;
}

ssize_t serp_read(struct file *filep, char __user *buff, size_t count, loff_t *offp)
{
	unsigned char a, b, rcv;
	int tmp, n = 0;
	set_current_state(TASK_INTERRUPTIBLE);

	if (RW_ERR == 0)
	{ /*
		a = copy_to_user(buff, filep, (int)count);
		if (a != 0)
		{
			RW_ERR = 1;
			return (ssize_t)count - a;
		}
		else
		{
			RW_ERR = 0;
			return (ssize_t)count;
		} */

		printk(KERN_ALERT "teste 1 \n");

		char *temp = kmalloc(count + 1, GFP_KERNEL);
		a = copy_from_user(temp, buff, (unsigned long)count);
				printk(KERN_ALERT "teste 2 \n");

		temp[count] = '\0';
				printk(KERN_ALERT "teste 3 \n");


		while (1)
		{
			b = read_uart(port_busy, REG_LSR);
			printk(KERN_ALERT "teste 4 \n");
			if (b & (UART_LSR_FE | UART_LSR_OE | UART_LSR_PE))
			{
				printk(KERN_ALERT "erro nos dados lidos\n");
				kfree(temp);
				return -EIO;
			}
			else if (b & UART_LSR_DR)
			{
				printk(KERN_ALERT "data ready\n");
				rcv = (char) read_uart(port_busy, REG_RHR);
				if (rcv != '!')
				{
					printk(KERN_ALERT "carater recebido: %c\n", rcv);
					temp[n] = rcv;
					n++;
					

				}
				else if (rcv == '!')
				{
					tmp = copy_to_user(buff, temp, n);
					if (tmp != 0)
					{
						printk(KERN_ALERT "houve %d letras nao escritas\n", tmp);
					}
					printk(KERN_ALERT "frase recebida: %s\n", temp);
					kfree(temp);
					break;

				}
				else
				{
					printk(KERN_ALERT "erro desconhecido\n");
					kfree(temp);
					return -EIO;
				}
			}
			else
			{
				msleep_interruptible(500);
				printk(KERN_ALERT "sleep \n");
			}
		}
		return (ssize_t)count;
	}
	else
	{
		printk(KERN_ALERT "Houve um erro no ssize_cd lab3
		t read anterior\n");
		RW_ERR = 0;
		return -1;
	}
}

ssize_t serp_write(struct file *filep, const char __user *buff, size_t count, loff_t *offp)
{
	if (RW_ERR == 0)
	{
		int i;
		int a = 0;
		/*
		b = copy_to_user(buff, temp, (unsigned long)count + 1);
		printk(KERN_ALERT "%s\n", temp);
		
		}*/

		char *temp = kmalloc(count + 1, GFP_KERNEL);
		a = copy_from_user(temp, buff, (unsigned long)count);
		temp[count] = '\0';

		for (i = 0; i < count; i++)
		{
			serial_write(temp[i]);
		}

		kfree(temp);

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
	else
	{
		printk(KERN_ALERT "Houve um erro no ssize_t write anterior\n");
		RW_ERR = 0;
		return -1;
	}
}

int read_uart(int COM_port, int reg)
{
	return (inb(COM_port + reg));
}

void write_uart(int COM_port, int reg, int data)
{

	//outp((COM_port + reg), data);		//original
	outb(data, (COM_port + reg)); //como diz no guiao
	return;
}

void serial_write(unsigned char ch)
{
	while (1)
	{
		if (!((unsigned char)read_uart(port_busy, REG_LSR) & 0x20))
		{
			schedule();
		}
		else
			break;
	}

	write_uart(port_busy, REG_THR, (int)ch);
}

int setup_serial(int COM_port, int baud, unsigned char misc)
{
	word divisor;

	if (port_busy)
		return (port_busy);

	port_busy = COM_port;

	write_uart(COM_port, REG_IER, 0);
	write_uart(COM_port, REG_LCR, (int)DLR_ON);
	divisor = 0x1c200 / baud;
	//outpw(COM_port, divisor);	//original
	outw(divisor, COM_port); //como diz no guiao

	write_uart(COM_port, REG_LCR, (int)misc);
	return 1;
}

module_init(serp_init);
module_exit(serp_exit);
